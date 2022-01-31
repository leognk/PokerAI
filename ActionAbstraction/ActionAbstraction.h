#ifndef ABC_ACTIONABSTRACTION_H
#define ABC_ACTIONABSTRACTION_H

#include "../GameEngine/GameState.h"
#include <vector>

namespace abc {

class ActionAbstraction
{
public:
	// betSizes is the available bet sizes with action abstraction
	// expressed in terms of fraction of the pot.
	// shape: n_rounds x n_raises_in_round x n_possible_bet_sizes
	ActionAbstraction(const std::vector<std::vector<std::vector<float>>>& betSizes);

	// actionId must be chosen between 0 and nActions excluded.
	// Actions are indexed using the following order:
	// fold, call, raiseSize1, raiseSize2, ..., raiseSizeLast, all-in,
	// illegal actions being skipped.
	void setAction(uint8_t actionId, egn::GameState& state, uint8_t& nRaises);
	uint8_t calculateLegalActions(const egn::GameState& state, uint8_t nRaises);

private:
	egn::chips getBetValue(uint8_t raiseId, const egn::GameState& state, uint8_t nRaises) const;

	std::vector<std::vector<std::vector<float>>> betSizes;

	// Legal bet size ids will be between
	// beginRaiseId included and endRaiseId excluded.
	uint8_t beginRaiseId, endRaiseId;

}; // ActionAbstraction

} // abc

#endif // ABC_ACTIONABSTRACTION_H