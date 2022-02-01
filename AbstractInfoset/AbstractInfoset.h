#ifndef ABC_ABSTRACTINFOSET_H
#define ABC_ABSTRACTINFOSET_H

#include "../GameEngine/GameState.h"
#include "../LossyAbstraction/LossyIndexer.h"
#include "ActionAbstraction.h"
#include "ActionSeqIndexer.h"

namespace abc {

// Class representing an abstract infoset.
// It is like a regular infoset, but using abstracted
// hands (with information abstraction) and abstracted actions
// (with action abstraction).
template<typename bckSize_t, bckSize_t nBck>
class AbstractInfoset
{
public:

	AbstractInfoset(
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const std::vector<std::vector<std::vector<float>>>& betSizes) :
		state(ante, bigBlind, {}),
		actionAbc(betSizes)
	{
		std::fill(initialStakes.begin(), initialStakes.end(), initialStake);
		// Load information abstraction lookup tables.
		handIndexer.loadLUT();
	}

	void startNewHand()
	{
		// Reset member variables.
		nRaises = 0;
		nPlayers = opt::MAX_PLAYERS;
		roundActions.clear();

		state.stakes = initialStakes;
		state.startNewHand(dealer);

		calculateHandsIds();
		nActions = actionAbc.calculateLegalActions(state, nRaises);
		calculateActionSeqIds();
	}

	void nextState(uint8_t actionId)
	{
		actionAbc.setAction(actionId, state, nRaises);

		// If the first players on the preflop fold, remove them and
		// proceed as if they did not exist.
		if (state.round == egn::PREFLOP && roundActions.empty() && actionId == 0)
			--nPlayers;
		else
			roundActions.push_back(actionId);

		egn::Round oldRound = state.round;
		state.nextState();

		// New round.
		if (state.round != oldRound) {
			nRaises = 0;
			nPlayers = state.mNAlive;
			roundActions.clear();
			calculateHandsIds();
		}

		nActions = actionAbc.calculateLegalActions(state, nRaises);
		calculateActionSeqIds();
	}

	// The pair composed of an abstract infoset and
	// one of its legal action is identified by:
	// - the current round
	// - the acting player's hand's bucket
	// - the index (given by a perfect hash function) of the action
	//   sequence leading to the legal action.
	uint8_t roundIdx() const { return state.round; }
	bckSize_t handIdx() const { return handsIds[state.actingPlayer]; }
	uint64_t actionSeqIdx(uint8_t a) const { return actionSeqIds[a]; }

	uint8_t nActions;

	egn::GameState state;

private:

	void calculateHandsIds()
	{
		uint8_t i = state.mFirstAlive;
		do {
			handsIds[i] = handIndexer.handIndex(
				state.round, state.hands[i].data(), state.boardCards.data());
		} while (state.nextAlive(i) != state.mFirstAlive);
	}

	void calculateActionSeqIds()
	{
		actionSeqIds.clear();
		for (uint8_t a = 0; a < nActions; ++a) {
			roundActions.push_back(a);
			actionSeqIds.push_back(
				actionSeqIndexer.actionSeqIndex(roundActions));
			roundActions.pop_back();
		}
	}

	static const uint8_t dealer = opt::MAX_PLAYERS - 1;
	std::array<egn::chips, opt::MAX_PLAYERS> initialStakes;

	// Number of raises done in the current round.
	uint8_t nRaises;

	// Number of players playing at the beginning of the current round.
	uint8_t nPlayers;
	// History of actions made in the current round.
	std::vector<uint8_t> roundActions;

	static abc::LossyIndexer<bckSize_t, nBck> handIndexer;
	std::array<bckSize_t, omp::MAX_PLAYERS> handsIds;

	abc::ActionAbstraction actionAbc;
	static abc::ActionSeqIndexer actionSeqIndexer;
	// Indices of the action sequences leading to each legal actions.
	std::vector<uint64_t> actionSeqIds;

}; // AbstractInfoset

template<typename bckSize_t, bckSize_t nBck>
abc::LossyIndexer<bckSize_t, nBck> AbstractInfoset<bckSize_t, nBck>::handIndexer;

template<typename bckSize_t, bckSize_t nBck>
abc::ActionSeqIndexer AbstractInfoset<bckSize_t, nBck>::actionSeqIndexer;

} // abc

#endif // ABC_ABSTRACTINFOSET_H