#include "ActionAbstraction.h"

namespace abc {

ActionAbstraction::ActionAbstraction(
	const std::vector<std::vector<std::vector<float>>>& betSizes) :
	betSizes(betSizes)
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
	if (action < 2)
		state.action = egn::Action(action);

	// Raise
	else {
		state.action = egn::RAISE;

		// All-in
		if (action == 2) state.bet = state.allin;
		// Regular raise
		else state.bet = (egn::chips)std::round(
			betSizes[state.round][nRaises][action - 3] * state.pot);

		if (nRaises != betSizes[state.round].size() - 1) ++nRaises;
	}
}

void ActionAbstraction::calculateLegalActions(
	const egn::GameState& state, uint8_t nRaises)
{
	if (state.legalCase == 0) {
		// Fold & call
		legalActions = { 0, 1 };
		return;
	}

	// Calculate beginRaiseId and endRaiseId.
	// Legal bet size ids will be between
	// beginRaiseId included and endRaiseId excluded.

	uint8_t beginRaiseId = 0;
#pragma warning (suppress: 4267)
	uint8_t endRaiseId = betSizes[state.round][nRaises].size();

	// Find the minimum idx for which
	// the corresponding bet value >= minRaise.
	float minRaiseSize = (float)state.minRaise / state.pot;
	while (betSizes[state.round][nRaises][beginRaiseId] < minRaiseSize)
		if (++beginRaiseId == endRaiseId) break;

	if (beginRaiseId != endRaiseId) {
		// Find the maximum idx for which
		// the previous corresponding bet value < allin.
		float allinSize = (float)state.allin / state.pot;
		while (betSizes[state.round][nRaises][endRaiseId - 1] >= allinSize)
			if (--endRaiseId == beginRaiseId) break;
	}

	// Build vector legalActions.

	uint8_t nLegalRaises = endRaiseId - beginRaiseId;

	if (state.legalCase == 1) {
#pragma warning(suppress: 26451)
		legalActions.resize(nLegalRaises + 2);
		// Call
		legalActions[0] = 1;
		// All-in
		legalActions[1] = 2;
		// Legal raises
		for (uint8_t i = 2; i < nLegalRaises + 2; ++i)
			legalActions[i] = beginRaiseId + i + 1;
	}

	else {
#pragma warning(suppress: 26451)
		legalActions.resize(nLegalRaises + 3);
		// Fold
		legalActions[0] = 0;
		// Call
		legalActions[1] = 1;
		// All-in
		legalActions[2] = 2;
		// Legal raises
		for (uint8_t i = 3; i < nLegalRaises + 3; ++i)
			legalActions[i] = beginRaiseId + i;
	}
}

} // abc