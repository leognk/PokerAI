#include "ActionAbstraction.h"

namespace abc {

ActionAbstraction::ActionAbstraction(
	const betSizes_t& betSizes) :
	betSizes(&betSizes)
{
}

ActionAbstraction& ActionAbstraction::operator=(const ActionAbstraction& other)
{
	if (this == &other)
		return *this;
	legalActions = other.legalActions;
	return *this;
}

void ActionAbstraction::setAction(
	uint8_t action, egn::GameState& state, uint8_t& nRaises)
{
	// Fold or call
	if (action < ALLIN)
		state.action = egn::Action(action);

	// Raise
	else {
		state.action = egn::RAISE;

		// All-in
		if (action == ALLIN) state.bet = state.allin;
		// Regular raise
		else state.bet = betSizeToBet(
			(*betSizes)[state.round][nRaises][action - RAISE], state);

		if (nRaises != (*betSizes)[state.round].size() - 1) ++nRaises;
	}
}

void ActionAbstraction::calculateLegalActions(
	const egn::GameState& state, uint8_t nRaises)
{
	if (state.legalCase == 0) {
		// Fold & call
		legalActions = { FOLD, CALL };
		return;
	}

	// Calculate beginRaiseId and endRaiseId.
	// Legal bet size ids will be between
	// beginRaiseId included and endRaiseId excluded.

	uint8_t beginRaiseId = 0;
#pragma warning (suppress: 4267)
	uint8_t endRaiseId = (*betSizes)[state.round][nRaises].size();

	// Find the minimum idx for which
	// the corresponding bet value >= minRaise.
	const float minRaiseSize = betToBetSize(state.minRaise, state);
	while ((*betSizes)[state.round][nRaises][beginRaiseId] < minRaiseSize)
		if (++beginRaiseId == endRaiseId) break;

	if (beginRaiseId != endRaiseId) {
		// Find the maximum idx for which
		// the previous corresponding bet value < allin.
		const float allinSize = betToBetSize(state.allin, state);
		while ((*betSizes)[state.round][nRaises][endRaiseId - 1] >= allinSize)
			if (--endRaiseId == beginRaiseId) break;
	}

	// Build vector legalActions.

	const uint8_t nLegalRaises = endRaiseId - beginRaiseId;

	if (state.legalCase == 1) {
#pragma warning(suppress: 26451)
		legalActions.resize(nLegalRaises + 2);
		// Call
		legalActions[0] = CALL;
		// All-in
		legalActions[1] = ALLIN;
		// Legal raises
		for (uint8_t i = 2; i < nLegalRaises + 2; ++i)
			legalActions[i] = beginRaiseId + i + 1;
	}

	else {
#pragma warning(suppress: 26451)
		legalActions.resize(nLegalRaises + 3);
		// Fold
		legalActions[0] = FOLD;
		// Call
		legalActions[1] = CALL;
		// All-in
		legalActions[2] = ALLIN;
		// Legal raises
		for (uint8_t i = 3; i < nLegalRaises + 3; ++i)
			legalActions[i] = beginRaiseId + i;
	}
}

egn::chips ActionAbstraction::betSizeToBet(
	const float betSize, const egn::GameState& state) const
{
	// A x pot size raise is a raise by a fraction x of the size
	// of the pot after that the acting player called. So the amount of
	// chips bet by the acting player is bet = call + x * (call + pot).
	// This way, the opponent faces a (1+x):1 pot odds (at best).
	return state.call + (egn::chips)std::round(betSize * (state.call + state.pot));
}

// Return the result of the inverse operation of betSizeToBet.
float ActionAbstraction::betToBetSize(
	const egn::chips bet, const egn::GameState& state) const
{
	return float(bet - state.call) / (state.pot + state.call);
}



} // abc