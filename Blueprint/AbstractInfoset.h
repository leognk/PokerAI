#ifndef BP_ABSTRACTINFOSET_H
#define BP_ABSTRACTINFOSET_H

#include "../LossyAbstraction/LossyIndexer.h"
#include "ActionSeqIndexer.h"
#include "../GameEngine/GameState.h"

namespace bp {

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PER_ROUND = 5;

typedef uint32_t actionSeqIdx_t;

// Available bet sizes after action abstraction
// expressed in terms of fraction of the pot.
// shape: n_rounds x n_raises_in_round x n_possible_bet_sizes
static const std::vector<std::vector<std::vector<float>>> BET_SIZES = {
	{
		{ 1, 1.25, 1.5, 1.75, 2, 3, 4, 6, 8, 15, 25, 35, 50 },
		{ 0.5, 1, 2, 4, 8, 15, 25, 50 },
		{ 0.5, 1, 2 },
		{ 1 }
	},
	{
		{ 0.25, 0.5, 1, 2, 4 },
		{ 0.5, 1, 2 },
		{ 1 }
	},
	{
		{ 0.5, 1 },
		{ 1 }
	},
	{
		{ 0.5, 1 },
		{ 1 }
	}
};

// Class representing an abstract infoset.
// It is like a regular infoset, but using abstracted
// hands (with information abstraction) and abstracted actions
// (with action abstraction).
class AbstractInfoset
{
public:
	AbstractInfoset(
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake);

	void startNewHand();
	// actionId must be chosen between 0 and nActions excluded.
	// Actions are indexed using the following order:
	// fold, call, raiseSize1, raiseSize2, ..., raiseSizeLast, all-in,
	// illegal actions being skipped.
	void nextState(uint8_t actionId);

	// The pair composed of an abstract infoset and
	// one of its legal action is identified by:
	// - the current round
	// - the acting player's hand's bucket
	// - the index (given by a perfect hash function) of the action
	//   sequence leading to the legal action.
	uint8_t roundIdx() const;
	bckSize_t handIdx() const;
	actionSeqIdx_t actionSeqIdx(uint8_t a) const;

	uint8_t nActions;

	egn::GameState state;

private:
	void setAction(uint8_t actionId);
	egn::chips getBetValue(uint8_t raiseId) const;
	void calculateLegalActions();

	void calculateHandsIds();
	void calculateActionSeqIds();

	static const uint8_t dealer = opt::MAX_PLAYERS - 1;
	std::array<egn::chips, opt::MAX_PLAYERS> initialStakes;
	uint8_t initialNActions;
	uint8_t initialBeginRaiseId, initialEndRaiseId;
	std::vector<actionSeqIdx_t> initialActionSeqIds;

	// Number of raises done in the current round.
	uint8_t nRaises;
	// Legal bet size ids will be between
	// beginRaiseId included and endRaiseId excluded.
	uint8_t beginRaiseId, endRaiseId;

	// Number of players playing at the beginning of the current round.
	uint8_t nPlayers;
	// History of actions made in the current round.
	std::vector<uint8_t> roundActions;

	static abc::LossyIndexer<bckSize_t, N_BCK_PER_ROUND> handIndexer;
	std::array<bckSize_t, omp::MAX_PLAYERS> handsIds;

	static ActionSeqIndexer<actionSeqIdx_t> actionSeqIndexer;
	// Indices of the action sequences leading to each legal actions.
	std::vector<actionSeqIdx_t> actionSeqIds;

}; // AbstractInfoset

} // bp

#endif // BP_ABSTRACTINFOSET_H