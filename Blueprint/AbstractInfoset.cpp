#include "AbstractInfoset.h"

namespace bp {

abc::DKEM<bckSize_t, N_BCK_PER_ROUND> AbstractInfoset::dkem;
abc::KOC<bckSize_t, N_BCK_PER_ROUND> AbstractInfoset::koc;

AbstractInfoset::AbstractInfoset(
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake) :

	state(ante, bigBlind, {})
{
	// Initialize member variables.
	std::fill(initialStakes.begin(), initialStakes.end(), initialStake);
	calculateLegalBetSizes();
	calculateIndex();
	initialNActions = nActions;
	initialBeginRaiseId = beginRaiseId;
	initialEndRaiseId = endRaiseId;
	initialIndex = index;

	// Load information abstraction lookup tables.
	dkem.loadPreflopBckLUT();
	dkem.loadFlopBckLUT();
	dkem.loadTurnBckLUT();
	koc.loadRivBckLUT();
}

void AbstractInfoset::startNewHand()
{
	// Reset member variables.
	nRaises = 0;
	state.stakes = initialStakes;
	nActions = initialNActions;
	beginRaiseId = initialBeginRaiseId;
	endRaiseId = initialEndRaiseId;
	index = initialIndex;

	state.startNewHand(0);
}

void AbstractInfoset::nextState(uint8_t actionId)
{
	setAction(actionId);
	egn::Round oldRound = state.round;
	state.nextState();
	if (state.round != oldRound) nRaises = 0;
	calculateLegalBetSizes();
	calculateIndex();
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

void AbstractInfoset::calculateLegalBetSizes()
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
		while ((egn::chips)std::round(BET_SIZES[state.round][nRaises][beginRaiseId] * state.pot)
			< state.minRaise) {

			if (++beginRaiseId == BET_SIZES[state.round][nRaises].size())
				return;
		}

		// Find the maximum idx for which
		// the previous corresponding bet value < allin.
		endRaiseId = BET_SIZES[state.round][nRaises].size();
		while ((egn::chips)std::round(BET_SIZES[state.round][nRaises][endRaiseId - 1] * state.pot)
			>= state.allin) {

			if (--endRaiseId == beginRaiseId)
				return;
		}

		nActions += endRaiseId - beginRaiseId;
	}
}

void AbstractInfoset::calculateIndex()
{

}

} // bp