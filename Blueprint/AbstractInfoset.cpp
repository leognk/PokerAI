#include "AbstractInfoset.h"

namespace bp {

abc::LossyIndexer<bckSize_t, N_BCK_PER_ROUND> AbstractInfoset::handIndexer;
abc::ActionSeqIndexer<actionSeqIdx_t> AbstractInfoset::actionSeqIndexer;

AbstractInfoset::AbstractInfoset(
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake) :

	state(ante, bigBlind, {}),
	actionAbc(betSizes)
{
	std::fill(initialStakes.begin(), initialStakes.end(), initialStake);
	// Load information abstraction lookup tables.
	handIndexer.loadLUT();
}

void AbstractInfoset::startNewHand()
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

void AbstractInfoset::nextState(uint8_t actionId)
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

void AbstractInfoset::calculateHandsIds()
{
	uint8_t i = state.mFirstAlive;
	do {
		handsIds[i] = handIndexer.handIndex(
			state.round, state.hands[i].data(), state.boardCards.data());
	} while (state.nextAlive(i) != state.mFirstAlive);
}

void AbstractInfoset::calculateActionSeqIds()
{
	actionSeqIds.clear();
	for (uint8_t a = 0; a < nActions; ++a) {
		roundActions.push_back(a);
		actionSeqIds.push_back(
			actionSeqIndexer.actionSeqIndex(roundActions));
		roundActions.pop_back();
	}
}

uint8_t AbstractInfoset::roundIdx() const
{
	return state.round;
}

bckSize_t AbstractInfoset::handIdx() const
{
	return handsIds[state.actingPlayer];
}

actionSeqIdx_t AbstractInfoset::actionSeqIdx(uint8_t a) const
{
	return actionSeqIds[a];
}

} // bp