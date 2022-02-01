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
	beginRaiseId = other.beginRaiseId;
	endRaiseId = other.endRaiseId;
	return *this;
}

void ActionAbstraction::setAction(
	uint8_t actionId, egn::GameState& state, uint8_t& nRaises)
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
			state.bet = getBetValue(actionId - 1 + beginRaiseId, state, nRaises);
			if (nRaises != betSizes[state.round].size() - 1) ++nRaises;
			return;
		}
	case 2:
		if (actionId < 2) {
			state.action = state.actions[actionId];
			return;
		}
		else {
			state.action = egn::RAISE;
			state.bet = getBetValue(actionId - 2 + beginRaiseId, state, nRaises);
			if (nRaises != betSizes[state.round].size() - 1) ++nRaises;
			return;
		}
	default:
		throw std::runtime_error("Invalid legal case.");
	}
}

egn::chips ActionAbstraction::getBetValue(
	uint8_t raiseId, const egn::GameState& state, uint8_t nRaises) const
{
	if (raiseId < endRaiseId)
		return (egn::chips)std::round(betSizes[state.round][nRaises][raiseId] * state.pot);
	else
		return state.allin;
}

uint8_t ActionAbstraction::calculateLegalActions(
	const egn::GameState& state, uint8_t nRaises)
{
	uint8_t nActions = state.nActions;

	if (state.actions[state.nActions - 1] == egn::RAISE) {

		// The all-in bet is already counted
		// with the raise action in state.nActions.
		if (state.allin <= state.minRaise) {
			beginRaiseId = 0;
			endRaiseId = 0;
			return nActions;
		}

		beginRaiseId = 0;
#pragma warning (suppress: 4267)
		endRaiseId = betSizes[state.round][nRaises].size();

		// Find the minimum idx for which
		// the corresponding bet value >= minRaise.
		float minRaiseSize = (float)state.minRaise / state.pot;
		while (betSizes[state.round][nRaises][beginRaiseId] < minRaiseSize) {
			if (++beginRaiseId == betSizes[state.round][nRaises].size())
				return nActions;
		}

		// Find the maximum idx for which
		// the previous corresponding bet value < allin.
		float allinSize = (float)state.allin / state.pot;
		while (betSizes[state.round][nRaises][endRaiseId - 1] >= allinSize) {
			if (--endRaiseId == beginRaiseId)
				return nActions;
		}

		nActions += endRaiseId - beginRaiseId;
	}

	return nActions;
}

} // abc