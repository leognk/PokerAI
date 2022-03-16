#include "ActionAbstraction.h"

namespace abc {

opt::FastRandomChoice<8> ActionAbstraction::betSizeRandChoice;

ActionAbstraction::ActionAbstraction(
	const betSizes_t& betSizes) :
	betSizes(&betSizes),
	betSizesCumWeights{ { 0, betSizeRandChoice.RANGE } }
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
	uint8_t action, egn::GameState& state,
	uint8_t& nRaises, bool incrNRaises)
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

		if (incrNRaises && nRaises != (*betSizes)[state.round].size() - 1)
			++nRaises;
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

	const std::vector<float>& currBetSizes = (*betSizes)[state.round][nRaises];

	// Calculate beginRaiseId and endRaiseId.
	// Legal bet size ids will be between
	// beginRaiseId included and endRaiseId excluded.

	beginRaiseId = 0;
	endRaiseId = (uint8_t)currBetSizes.size();

	// Find the minimum idx for which
	// the corresponding bet value >= minRaise.
	minRaiseSize = betToBetSize(state.minRaise, state);
	while (currBetSizes[beginRaiseId] < minRaiseSize)
		if (++beginRaiseId == endRaiseId) break;

	if (beginRaiseId != endRaiseId) {
		// Find the maximum idx for which
		// the previous corresponding bet value < allin.
		allinSize = betToBetSize(state.allin, state);
		while (currBetSizes[endRaiseId - 1] >= allinSize)
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

// Map the last action done in the given game state to an abstract action.
// Call it AFTER calling calculateLegalActions and setting state's action
// and bet variables and BEFORE calling state.nextState().
uint8_t ActionAbstraction::mapActionToAbcAction(
	const egn::GameState& state, uint8_t nRaises, Rng& rng)
{
	switch (state.action) {

	case egn::FOLD: return FOLD;

	case egn::CALL: return CALL;

	case egn::RAISE:

		const std::vector<float>& currBetSizes = (*betSizes)[state.round][nRaises];

		// Find the bet sizes sizeA and sizeB which frame sizeX.

		const float sizeX = betToBetSize(state.bet, state);
		float sizeA = currBetSizes[beginRaiseId];

		if (sizeX <= sizeA) return RAISE + beginRaiseId;
		else if (sizeX >= allinSize) return ALLIN;

		float sizeB;
		uint8_t sizeBId = beginRaiseId + 1;
		while (sizeBId < endRaiseId) {
			sizeB = currBetSizes[sizeBId];
			if (sizeX == sizeB) return RAISE + sizeBId;
			else if (sizeX < sizeB) break;
			sizeA = sizeB;
			++sizeBId;
		}
		if (sizeBId == endRaiseId) sizeB = allinSize;

		// Pick either sizeA or sizeB using the action mapping function
		// which gives the probability of picking sizeA.

		betSizesCumWeights[0] = (uint16_t)std::round(
			actionMappingFunction(sizeA, sizeB, sizeX) * betSizeRandChoice.RANGE);

		if (betSizeRandChoice(betSizesCumWeights, rng) == 0)
			return RAISE + sizeBId - 1;
		else if (sizeBId == endRaiseId)
			return ALLIN;
		else
			return RAISE + sizeBId;
	}
}

uint8_t ActionAbstraction::mapActionToFoldCall(
	const egn::GameState& state, uint8_t nRaises, Rng& rng)
{
	const float sizeX = betToBetSize(state.bet, state);

	if (sizeX >= allinSize) return CALL;

	betSizesCumWeights[0] = (uint16_t)std::round(
		actionMappingFunction(0.0f, allinSize, sizeX) * betSizeRandChoice.RANGE);

	if (betSizeRandChoice(betSizesCumWeights, rng) == 0)
		return FOLD;
	else
		return CALL;
}

float ActionAbstraction::actionMappingFunction(const float a, const float b, const float x)
{
	return ((b - x) * (1 + a)) / ((b - a) * (1 + x));
}

} // abc