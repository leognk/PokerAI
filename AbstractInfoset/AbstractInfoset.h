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
		unsigned rngSeed = 0) :

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
			nPlayers = state.nAlive;
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

	egn::GameState state;
	abc::ActionAbstraction actionAbc;

protected:

	void calculateHandsIds()
	{
		uint8_t i = state.firstAlive;
		do {
			handsIds[i] = handIndexer.handIndex(
				state.round, state.hands[i].data(), state.boardCards.data());
		} while (state.nextAlive(i) != state.firstAlive);
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
	std::array<egn::chips, egn::MAX_PLAYERS> initialStakes{};

	// Number of raises done in the current round.
	uint8_t nRaises;

	// Number of players playing at the beginning of the current round.
	uint8_t nPlayers;
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