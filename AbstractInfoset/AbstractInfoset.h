#ifndef ABC_ABSTRACTINFOSET_H
#define ABC_ABSTRACTINFOSET_H

#include "../GameEngine/GameState.h"
#include "../LossyAbstraction/LossyIndexer.h"
#include "ActionAbstraction.h"
#include "ActionSeqIndexer.h"
#include "ActionSeq.h"

namespace abc {

// Class representing an abstract infoset.
// It is like a regular infoset, but using abstracted
// hands (with information abstraction) and abstracted actions
// (with action abstraction).
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
class AbstractInfoset
{
public:

	// Set rngSeed to 0 to set a random seed.
	AbstractInfoset(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const betSizes_t& betSizes,
		const std::string& actionSeqIndexerName,
		unsigned rngSeed) :

		maxPlayers(maxPlayers),
		initialStake(initialStake),
		dealer(maxPlayers - 1),
		state(ante, bigBlind, {}, rngSeed),
		actionAbc(betSizes),
		actionSeqIndexer(maxPlayers, ante, bigBlind, initialStake, betSizes, actionSeqIndexerName)
	{
		std::fill(initialStakes.begin(), initialStakes.begin() + maxPlayers, initialStake);
		// Load information abstraction lookup tables.
		handIndexer.loadLUT();
		// Load action sequences minimal perfect hash functions.
		actionSeqIndexer.loadMPHF();
	}

	AbstractInfoset& operator=(const AbstractInfoset& other)
	{
		if (this == &other) return *this;

		state = other.state;
		nRaises = other.nRaises;
		nPlayers = other.nPlayers;
		roundActions = other.roundActions;
		actionAbc = other.actionAbc;
		handsIds = other.handsIds;
		actionSeqIds = other.actionSeqIds;

		return *this;
	}

	void resetStakes()
	{
		state.stakes = initialStakes;
	}

	void resetStakes(std::array<egn::chips, egn::MAX_PLAYERS> stakes)
	{
		for (uint8_t i = 0; i < egn::MAX_PLAYERS; ++i) {
			if (stakes[i]) state.stakes[i] = initialStake;
			else state.stakes[i] = 0;
		}
	}

	void startNewHand(uint8_t dealer0, bool calculateStateId, bool dealRandomCards = true)
	{
		// Reset member variables.
		nRaises = 0;
		roundActions.clear();

		state.startNewHand(dealer0, dealRandomCards);

		actionAbc.calculateLegalActions(state, nRaises);
		if (calculateStateId) {
			calculateHandsIds();
			calculateActionSeqIds();
		}
	}

	void startNewHand(bool calculateStateId, bool dealRandomCards = true)
	{
		startNewHand(dealer, calculateStateId, dealRandomCards);
	}

	void nextStateWithAction(uint8_t action, bool calculateStateId)
	{
		setAction(action);
		goNextState(calculateStateId);
	}

	// actionId must be between 0 and nActions() excluded.
	void nextState(uint8_t actionId, bool calculateStateId)
	{
		nextStateWithAction(actionAbc.legalActions[actionId], calculateStateId);
	}

	// Same as nextState but also return the amount of the bet corresponding to actionId.
	egn::chips nextStateWithBet(uint8_t actionId, bool calculateStateId)
	{
		setAction(actionAbc.legalActions[actionId]);

		// Get the bet amount.
		egn::chips bet;
		switch (state.action) {
		case egn::FOLD:
			bet = 0;
			break;
		case egn::CALL:
			bet = state.call;
			break;
		case egn::RAISE:
			bet = state.bet;
			break;
		default:
			throw std::runtime_error("Unknown action.");
		}

		goNextState(calculateStateId);
		return bet;
	}

	void updateStateIds()
	{
		calculateHandIdx(state.actingPlayer);
		calculateActionSeqIds();
	}

	// Map the last action done in the given game state to an abstract action
	// and return it (not the idx).
	uint8_t mapActionToAbcAction(
		const egn::GameState& state, ActionAbstraction::Rng& rng)
	{
		return actionAbc.mapActionToAbcAction(state, nRaises, rng);
	}

	uint8_t mapActionToFoldCall(
		const egn::GameState& state, ActionAbstraction::Rng& rng)
	{
		return actionAbc.mapActionToFoldCall(state, rng);
	}

	// The pair composed of an abstract infoset and
	// one of its legal action is identified by:
	// - the current round.
	// - the acting player's hand's bucket.
	// - the index (given by a minimal perfect hash function) of the action
	//   sequence leading to the legal action, with the number of players
	//   included for rounds other than preflop.
	uint8_t roundIdx() const { return state.round; }
	bckSize_t handIdx() const { return handsIds[state.actingPlayer]; }
	// Indices of the action sequences leading to each legal action
	// with the number of players included for rounds other than preflop.
	// Its size is nActions().
	std::vector<uint64_t> actionSeqIds;

#pragma warning(suppress: 4267)
	uint8_t nActions() const { return actionAbc.legalActions.size(); }

	static size_t nBcks(egn::Round round)
	{
		switch (round) {
		case egn::PREFLOP: return nBckPreflop;
		case egn::FLOP: return nBckFlop;
		case egn::TURN: return nBckTurn;
		case egn::RIVER: return nBckRiver;
		default: throw std::runtime_error("Unknown round.");
		}
	}

	size_t nActionSeqs(egn::Round round) const
	{
		switch (round) {
		case egn::PREFLOP: return actionSeqIndexer.preflopMPHF.nbKeys();
		case egn::FLOP: return actionSeqIndexer.flopMPHF.nbKeys();
		case egn::TURN: return actionSeqIndexer.turnMPHF.nbKeys();
		case egn::RIVER: return actionSeqIndexer.riverMPHF.nbKeys();
		default: throw std::runtime_error("Unknown round.");
		}
	}

	uint8_t getActionId(uint8_t action) const
	{
		for (uint8_t i = 0; i < nActions(); ++i) {
			if (actionAbc.legalActions[i] == action)
				return i;
		}
		throw std::runtime_error("Action was not found in legalActions.");
	}

	void setStateAction(uint8_t action)
	{
		actionAbc.setAction(action, state, nRaises, false);
	}

	void addFold()
	{
		roundActions.push_back(FOLD);
	}

	// Return the action seq ids of all the possible actions without
	// taking into account minRaise and maxRaise.
	void calculateAllActionSeqIds(
		std::vector<uint8_t>& allLegalActions,
		std::vector<uint64_t>& allActionSeqIds)
	{
		allLegalActions = actionAbc.calculateAllLegalActions(state, nRaises);

		if (state.round == egn::PREFLOP) {
			for (uint8_t a : allLegalActions) {
				roundActions.push_back(a);
				allActionSeqIds.push_back(
					actionSeqIndexer.index(state.round, roundActions));
				roundActions.pop_back();
			}
		}

		else {
			for (uint8_t a : allLegalActions) {
				roundActions.push_back(a);
				roundActions.push_back(nPlayers);
				allActionSeqIds.push_back(
					actionSeqIndexer.index(state.round, roundActions));
				roundActions.pop_back();
				roundActions.pop_back();
			}
		}
	}

	egn::chips actionToBet(uint8_t action) const
	{
		return actionAbc.actionToBet(action, state, nRaises);
	}

	const uint8_t maxPlayers;
	egn::GameState state;
	abc::ActionAbstraction actionAbc;

	// Number of players playing at the beginning of the current round.
	uint8_t nPlayers;

protected:

	void setAction(uint8_t action)
	{
		actionAbc.setAction(action, state, nRaises);
		roundActions.push_back(action);
	}

	void goNextState(bool calculateStateId)
	{
		egn::Round oldRound = state.round;
		state.nextState();

		// New round.
		if (state.round != oldRound) {
			nRaises = 0;
			nPlayers = state.nAlive;
			roundActions.clear();
			if (calculateStateId) calculateHandsIds();
		}

		actionAbc.calculateLegalActions(state, nRaises);
		if (calculateStateId) calculateActionSeqIds();
	}

	void calculateHandsIds()
	{
		uint8_t i = state.firstAlive;
		do {
			calculateHandIdx(i);
		} while (state.nextAlive(i) != state.firstAlive);
	}

	void calculateHandIdx(const uint8_t player)
	{
		handsIds[player] = handIndexer.handIndex(
			state.round, state.hands[player].data(), state.boardCards.data());
	}

	void calculateActionSeqIds()
	{
		actionSeqIds.clear();

		if (state.round == egn::PREFLOP) {
			for (uint8_t a : actionAbc.legalActions) {
				roundActions.push_back(a);
				actionSeqIds.push_back(
					actionSeqIndexer.index(state.round, roundActions));
				roundActions.pop_back();
			}
		}

		// For rounds other than preflop, include the number of players.
		else {
			for (uint8_t a : actionAbc.legalActions) {
				roundActions.push_back(a);
				roundActions.push_back(nPlayers);
				actionSeqIds.push_back(
					actionSeqIndexer.index(state.round, roundActions));
				roundActions.pop_back();
				roundActions.pop_back();
			}
		}
	}

	const uint8_t dealer;
	egn::chips initialStake;
	std::array<egn::chips, egn::MAX_PLAYERS> initialStakes{};

	// Number of raises done in the current round.
	uint8_t nRaises;

	// History of actions made in the current round stored in a compressed format.
	StdActionSeq roundActions;

	static abc::LossyIndexer<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver> handIndexer;
	std::array<bckSize_t, omp::MAX_PLAYERS> handsIds;

	abc::ActionSeqIndexer actionSeqIndexer;

}; // AbstractInfoset

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
abc::LossyIndexer<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver> AbstractInfoset<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver>::handIndexer;

} // abc

#endif // ABC_ABSTRACTINFOSET_H