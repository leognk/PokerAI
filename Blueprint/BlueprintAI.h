#ifndef ABC_BLUEPRINTAI_H
#define ABC_BLUEPRINTAI_H

#include "Blueprint.h"
#include "../GameEngine/Player.h"

namespace bp {

#define BLUEPRINT_AI_BUILDER(bpGameName, bpBuildName, \
	ante, bigBlind, initialStake, blueprint, rngSeed) \
	bp::BlueprintAI< \
		bpGameName::bckSize_t, \
		bpGameName::N_BCK_PREFLOP,bpGameName::N_BCK_FLOP, \
		bpGameName::N_BCK_TURN, bpGameName::N_BCK_RIVER>( \
			bpGameName::MAX_PLAYERS, ante, bigBlind, initialStake, \
			bpGameName::BET_SIZES, bpGameName::BLUEPRINT_GAME_NAME, \
			blueprint, rngSeed)

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
	}

	void reset(const egn::GameState& state) override
	{
		// Set abcInfo's cards.
		uint8_t i = state.firstAlive;
		do {
			abcInfo.state.setHoleCards(i, state.hands[i].data());
		} while (state.nextAlive(i) != state.firstAlive);
		abcInfo.state.setBoardCards(state.boardCards.data());

		// Set abcInfo's stakes.
		abcInfo.state.stakes = state.initialStakes;

		abcInfo.startNewHand(state.dealer, false, false);

		// Set abcInfo to a state with nAlive players.
		if (abcInfo.maxPlayers < state.nAlive)
			throw std::runtime_error("Number of players greater than the maximum.");
		for (uint8_t i = state.nAlive; i < abcInfo.maxPlayers; ++i)
			abcInfo.nextStateWithAction(abc::FOLD, false);
	}

	void act(egn::GameState& state) override
	{
		// Choose action.
		abcInfo.updateStateIds();
		abcInfo.nextState(blueprint->chooseAction(abcInfo), false);
		
		// Set state's action.
		state.action = abcInfo.state.action;
		if (state.action == egn::RAISE) {
			// We clip the bet between minRaise and allin.
			// It might have gone out of the limits because
			// the state in abcInfo is different than the real
			// game state.
			egn::chips bet = abcInfo.state.bet;
			if (bet < state.minRaise) bet = state.minRaise;
			else if (bet > state.allin) bet = state.allin;
			state.bet = bet;
		}
	}

	void update(const egn::GameState& state) override
	{
		const uint8_t action = abcInfo.mapActionToAbcAction(state, rng);
		abcInfo.nextStateWithAction(action, false);
	}

private:

	abc::AbstractInfoset<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver> abcInfo;
	Blueprint* blueprint;
	abc::ActionAbstraction::Rng rng;

}; // BlueprintAI

} // bp

#endif // ABC_BLUEPRINTAI_H