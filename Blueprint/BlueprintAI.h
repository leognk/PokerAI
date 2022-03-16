#ifndef ABC_BLUEPRINTAI_H
#define ABC_BLUEPRINTAI_H

#include "Blueprint.h"
#include "../GameEngine/Player.h"

namespace bp {

#define BLUEPRINT_AI_BUILDER(bpGameName, bpBuildName, bigBlind, blueprint, rngSeed) \
	bp::BlueprintAI< \
		bpGameName::bckSize_t, \
		bpGameName::N_BCK_PREFLOP, \
		bpGameName::N_BCK_FLOP, \
		bpGameName::N_BCK_TURN, \
		bpGameName::N_BCK_RIVER>( \
			bpGameName::MAX_PLAYERS, \
			bpGameName::ANTE, \
			bpGameName::BIG_BLIND, \
			bpGameName::INITIAL_STAKE, \
			bpGameName::BET_SIZES, \
			bpGameName::BLUEPRINT_GAME_NAME, \
			bigBlind, \
			blueprint, \
			rngSeed)

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
class BlueprintAI : public egn::Player
{
public:

	// Set rngSeed to 0 to set a random seed.
	BlueprintAI(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const abc::betSizes_t& betSizes,
		const std::string& blueprintGameName,
		egn::chips realBigBlind,
		Blueprint* blueprint,
		unsigned rngSeed = 0) :

		abcInfo(
			maxPlayers,
			ante,
			bigBlind,
			initialStake,
			betSizes,
			blueprintGameName,
			rngSeed),

		blueprint(blueprint),

		rng{ (!rngSeed) ? std::random_device{}() : rngSeed }
	{
		setBB(realBigBlind);
	}

	void setBB(const egn::chips bb)
	{
		realOverAbcBB = (float)bb / abcInfo.state.bb;
	}

	void reset(const egn::GameState& state) override
	{
		// Set abcInfo's stakes.
		abcInfo.resetStakes(state.stakes);

		// Set abcInfo's cards.
		uint8_t i = state.firstAlive;
		do {
			abcInfo.state.setHoleCards(i, state.hands[i].data());
		} while (state.nextAlive(i) != state.firstAlive);
		abcInfo.state.setBoardCards(state.boardCards.data());

		// Start new hand.
		abcInfo.startNewHand(state.dealer, false, false);

		// Set abcInfo to a state with nAlive players.
		if (abcInfo.maxPlayers < state.nAlive)
			throw std::runtime_error("Number of players greater than the maximum.");
		for (uint8_t i = state.nAlive; i < abcInfo.maxPlayers; ++i)
			abcInfo.addFold();

		isAllIn.fill(false);
		allinExists = false;
		abcAllinFlag = false;
	}

	void act(egn::GameState& state) override
	{
		// Choose an action.
		abcInfo.updateStateIds();
		const uint8_t action = abcInfo.actionAbc.legalActions[
			blueprint->chooseAction(abcInfo)];
		abcInfo.setStateAction(action);
		
		// Set state's action.

		state.action = abcInfo.state.action;

		switch (state.action) {

		case egn::FOLD:
			break;

		case egn::CALL:
			// If the call chosen by the blueprint in the abcInfo is a response
			// to an all-in but the actual call in the real state is not, we
			// have to ensure if becomes a real all-in.
			if (abcInfo.state.call == abcInfo.state.allin
				&& state.call != state.allin) {
				state.action = egn::RAISE;
				state.bet = state.allin;
			}
			break;

		case egn::RAISE:
			// The abcInfo's all-in is limited to the abcInfo's static const initial
			// stake of players, but if the chosen action is all-in, we bet all the
			// player's stake anyway.
			if (action == abc::ALLIN)
				state.bet = state.allin;
			else {
				// We clip the bet between minRaise and allin.
				// It might have gone out of the limits because
				// the state in abcInfo is different from the real
				// game state, plus the initial stakes of all players
				// in abcInfo are set to a static const.
				egn::chips bet = abcToRealChips(abcInfo.state.bet);
				if (bet >= state.allin) bet = state.allin;
				else if (bet < state.minRaise) bet = state.minRaise;
				state.bet = bet;
			}
			break;

		default:
			throw std::runtime_error("Unknown action.");
		}
	}

	void update(const egn::GameState& state) override
	{
		// abcInfo would have been finished before the real state
		// if the blueprint AI went all-in by calling an all-in which
		// was not one in the real state but was mapped to it anyway.
		if (abcInfo.state.finished) return;

		if (goAllIn(state)) {
			isAllIn[abcInfo.state.actingPlayer] = true;
			allinExists = true;
		}
		egn::Round oldRound = state.round;

		// No need to convert the state's chips because of different bb in abcInfo,
		// because only the bet size is used and it is independent from the bb.
		uint8_t action;
		// If a previous action was mapped to an all-in but was not one in the real
		// state, a following action can be a raise while abcInfo only authorizes
		// fold or call, so we map the raise to one of these two.
		if (abcAllinFlag)
			action = abcInfo.mapActionToFoldCall(state, rng);
		else
			action = abcInfo.mapActionToAbcAction(state, rng);
		abcInfo.nextStateWithAction(action, false);

		if (action == abc::ALLIN) abcAllinFlag = true;

		// If the next acting player has gone all-in, we put him in a fold state
		// so that the number of acting players in abcInfo is correct.
		while (isAllIn[abcInfo.state.actingPlayer]
			&& abcInfo.state.round == oldRound
			&& !abcInfo.state.finished) {
			oldRound = abcInfo.state.round;
			abcInfo.nextStateWithAction(abc::FOLD, false);
		}

		// If a new round has begun, erase all alive players in abcInfo.state who
		// went all-in so that the number of acting players in abcInfo is correct.
		if (allinExists && abcInfo.state.round != oldRound) {
			uint8_t i = abcInfo.state.firstAlive;
			do {
				if (isAllIn[i] && abcInfo.state.isAlive(i)) {
					abcInfo.state.eraseAlive(i);
					abcInfo.state.eraseActing(i);
				}
			} while (abcInfo.state.nextAlive(i) != abcInfo.state.firstAlive);
			abcInfo.nPlayers = abcInfo.state.nAlive;
		}
	}

	// Convert the number of chips x in the abcInfo's bb reference
	// to the real bb reference.
	// For example, if the abcInfo's bb is 100 chips and the
	// real bb is 2 chips, an amount of 200 chips (2 bb in abcInfo)
	// will be converted to 4 chips (2 bb in the real state).
	egn::chips abcToRealChips(const egn::chips x)
	{
		return (egn::chips)std::round(x * realOverAbcBB);
	}

	// Return if the acting player goes all-in.
	// Call it BEFORE calling state.nextState.
	bool goAllIn(const egn::GameState& state)
	{
		switch (state.action) {
		case egn::FOLD: return false;
		case egn::CALL: return state.call == state.stakes[state.actingPlayer];
		case egn::RAISE: return state.bet == state.stakes[state.actingPlayer];
		default: throw std::runtime_error("Unknown action.");
		}
	}

private:

	abc::AbstractInfoset<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver> abcInfo;
	Blueprint* blueprint;
	abc::ActionAbstraction::Rng rng;

	float realOverAbcBB;

	std::array<bool, egn::MAX_PLAYERS> isAllIn;
	bool allinExists;
	bool abcAllinFlag;

}; // BlueprintAI

} // bp

#endif // ABC_BLUEPRINTAI_H