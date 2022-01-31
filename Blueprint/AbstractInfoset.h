#ifndef BP_ABSTRACTINFOSET_H
#define BP_ABSTRACTINFOSET_H

#include "../GameEngine/GameState.h"
#include "../LossyAbstraction/LossyIndexer.h"
#include "../ActionAbstraction/ActionAbstraction.h"
#include "../ActionAbstraction/ActionSeqIndexer.h"

namespace bp {

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PER_ROUND = 5;

typedef uint32_t actionSeqIdx_t;

static const std::vector<std::vector<std::vector<float>>> betSizes = {
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
	void calculateHandsIds();
	void calculateActionSeqIds();

	static const uint8_t dealer = opt::MAX_PLAYERS - 1;
	std::array<egn::chips, opt::MAX_PLAYERS> initialStakes;

	// Number of raises done in the current round.
	uint8_t nRaises;

	// Number of players playing at the beginning of the current round.
	uint8_t nPlayers;
	// History of actions made in the current round.
	std::vector<uint8_t> roundActions;

	static abc::LossyIndexer<bckSize_t, N_BCK_PER_ROUND> handIndexer;
	std::array<bckSize_t, omp::MAX_PLAYERS> handsIds;

	abc::ActionAbstraction actionAbc;

	static abc::ActionSeqIndexer<actionSeqIdx_t> actionSeqIndexer;
	// Indices of the action sequences leading to each legal actions.
	std::vector<actionSeqIdx_t> actionSeqIds;

}; // AbstractInfoset

} // bp

#endif // BP_ABSTRACTINFOSET_H