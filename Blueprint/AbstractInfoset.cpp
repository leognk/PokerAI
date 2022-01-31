#include "AbstractInfoset.h"

namespace bp {

abc::LossyIndexer<bckSize_t, N_BCK_PER_ROUND> AbstractInfoset::handIndexer;

AbstractInfoset::AbstractInfoset(
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake) :

	state(ante, bigBlind, {})
{
	// Initialize member variables.

	std::fill(initialStakes.begin(), initialStakes.end(), initialStake);

	state.stakes = initialStakes;
	state.startNewHand(dealer);
	calculateLegalActions();
	calculateActionSeqIds();

	initialNActions = nActions;
	initialBeginRaiseId = beginRaiseId;
	initialEndRaiseId = endRaiseId;
	initialActionSeqIds = actionSeqIds;

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
	nActions = initialNActions;
	beginRaiseId = initialBeginRaiseId;
	endRaiseId = initialEndRaiseId;
	actionSeqIds = initialActionSeqIds;

	state.startNewHand(dealer);
	calculateHandsIds();
}

void AbstractInfoset::nextState(uint8_t actionId)
{
	setAction(actionId);

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

	calculateLegalActions();
	calculateActionSeqIds();
}

void AbstractInfoset::setAction(uint8_t actionId)
{
	switch (state.legalCase) {
	case 0:
		state.action = state.actions[actionId];
		return;
	case 1:
		if (actionId == 0) {
			state.action = egn::CALL;
			return;
		}
		else {
			state.action = egn::RAISE;
			state.bet = getBetValue(actionId - 1 + beginRaiseId);
			if (nRaises != BET_SIZES[state.round].size() - 1) ++nRaises;
			return;
		}
	case 2:
		if (actionId < 2) {
			state.action = state.actions[actionId];
			return;
		}
		else {
			state.action = egn::RAISE;
			state.bet = getBetValue(actionId - 2 + beginRaiseId);
			if (nRaises != BET_SIZES[state.round].size() - 1) ++nRaises;
			return;
		}
	default:
		throw std::runtime_error("Invalid legal case.");
	}
}

egn::chips AbstractInfoset::getBetValue(uint8_t raiseId) const
{
	if (raiseId < endRaiseId)
		return (egn::chips)std::round(BET_SIZES[state.round][nRaises][raiseId] * state.pot);
	else
		return state.allin;
}

void AbstractInfoset::calculateLegalActions()
{
	nActions = state.nActions;

	if (state.actions[state.nActions - 1] == egn::RAISE) {

		// The all-in bet is already counted
		// with the raise action in state.nActions.
		if (state.allin <= state.minRaise) {
			beginRaiseId = 0;
			endRaiseId = 0;
			return;
		}

		beginRaiseId = 0;
#pragma warning (suppress: 4267)
		endRaiseId = BET_SIZES[state.round][nRaises].size();

		// Find the minimum idx for which
		// the corresponding bet value >= minRaise.
		float minRaiseSize = (float)state.minRaise / state.pot;
		while (BET_SIZES[state.round][nRaises][beginRaiseId] < minRaiseSize) {
			if (++beginRaiseId == BET_SIZES[state.round][nRaises].size())
				return;
		}

		// Find the maximum idx for which
		// the previous corresponding bet value < allin.
		float allinSize = (float)state.allin / state.pot;
		while (BET_SIZES[state.round][nRaises][endRaiseId - 1] >= allinSize) {
			if (--endRaiseId == beginRaiseId)
				return;
		}

		nActions += endRaiseId - beginRaiseId;
	}
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