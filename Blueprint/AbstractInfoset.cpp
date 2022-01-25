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
	calculateLegalActions();
	calculateNextStatesIds();
	initialNActions = nActions;
	initialBeginRaiseId = beginRaiseId;
	initialEndRaiseId = endRaiseId;
	initialNextStatesIds = nextStatesIds;

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
	nextStatesIds = initialNextStatesIds;

	state.startNewHand(0);
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
	calculateNextStatesIds();
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

egn::chips AbstractInfoset::getBetValue(uint8_t raiseId)
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

		// Add the all-in bet.
		++nActions;

		if (state.allin <= state.minRaise)
			return;

		// Find the minimum idx for which
		// the corresponding bet value >= minRaise.
		beginRaiseId = 0;
		float minRaiseSize = state.minRaise / state.pot;
		while (BET_SIZES[state.round][nRaises][beginRaiseId] < minRaiseSize) {
			if (++beginRaiseId == BET_SIZES[state.round][nRaises].size())
				return;
		}

		// Find the maximum idx for which
		// the previous corresponding bet value < allin.
		endRaiseId = BET_SIZES[state.round][nRaises].size();
		float allinSize = state.allin / state.pot;
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

void AbstractInfoset::calculateNextStatesIds()
{
	nextStatesIds.clear();
	for (uint8_t a = 0; a < nActions; ++a)
		nextStatesIds.push_back(calculateNextStateIdx(a));
}

// Hash the key composed of the following integers:
// - state.round
// - nPlayers
// - handsIds[state.actingPlayer]
// - roundActions
// - nextActionId
infoIdx_t AbstractInfoset::calculateNextStateIdx(uint8_t nextActionId)
{

}

} // bp