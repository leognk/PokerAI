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
template<typename bckSize_t, bckSize_t nBck>
class AbstractInfoset
{
public:

	AbstractInfoset(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const betSizes_t& betSizes,
		const std::string& actionSeqIndexerName) :
		state(ante, bigBlind, {}),
		actionAbc(betSizes),
		actionSeqIndexer(maxPlayers, ante, bigBlind, initialStake, betSizes, actionSeqIndexerName)
	{
		std::fill(initialStakes.begin(), initialStakes.begin() + maxPlayers, initialStake);
		// Load information abstraction lookup tables.
		handIndexer.loadLUT();
		// Load action sequences perfect hash functions.
		actionSeqIndexer.loadPHF();
	}

	void startNewHand()
	{
		// Reset member variables.
		nRaises = 0;
		nPlayers = egn::MAX_PLAYERS;
		roundActions.clear();

		state.stakes = initialStakes;
		state.startNewHand(dealer);

		calculateHandsIds();
		actionAbc.calculateLegalActions(state, nRaises);
		calculateActionSeqIds();
	}

	// actionId must be between 0 and nActions() excluded.
	void nextState(uint8_t actionId)
	{
		actionAbc.setAction(actionAbc.legalActions[actionId], state, nRaises);
		roundActions.push_back(actionAbc.legalActions[actionId]);

		egn::Round oldRound = state.round;
		state.nextState();

		// New round.
		if (state.round != oldRound) {
			nRaises = 0;
			nPlayers = state.mNAlive;
			roundActions.clear();
			calculateHandsIds();
		}

		actionAbc.calculateLegalActions(state, nRaises);
		calculateActionSeqIds();
	}

	// The pair composed of an abstract infoset and
	// one of its legal action is identified by:
	// - the current round.
	// - the acting player's hand's bucket.
	// - the index (given by a perfect hash function) of the action
	//   sequence leading to the legal action, with the number of players
	//   included for rounds other than preflop.
	uint8_t roundIdx() const { return state.round; }
	bckSize_t handIdx() const { return handsIds[state.actingPlayer]; }
	// actionId must be between 0 and nActions() excluded.
	uint64_t actionSeqIdx(uint8_t actionId) const { return actionSeqIds[actionId]; }

	uint8_t nActions() const { return actionAbc.legalActions.size(); }

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

	static const uint8_t dealer = egn::MAX_PLAYERS - 1;
	std::array<egn::chips, egn::MAX_PLAYERS> initialStakes{};

	// Number of raises done in the current round.
	uint8_t nRaises;

	// Number of players playing at the beginning of the current round.
	uint8_t nPlayers;
	// History of actions made in the current round stored in a compressed format.
	StdActionSeq roundActions;

	static abc::LossyIndexer<bckSize_t, nBck> handIndexer;
	std::array<bckSize_t, omp::MAX_PLAYERS> handsIds;

	abc::ActionAbstraction actionAbc;
	abc::ActionSeqIndexer actionSeqIndexer;
	// Indices of the action sequences leading to each legal actions
	// with the number of players included for rounds other than preflop.
	std::vector<uint64_t> actionSeqIds;

}; // AbstractInfoset

template<typename bckSize_t, bckSize_t nBck>
abc::LossyIndexer<bckSize_t, nBck> AbstractInfoset<bckSize_t, nBck>::handIndexer;

} // abc

#endif // ABC_ABSTRACTINFOSET_H