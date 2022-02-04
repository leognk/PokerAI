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

	// The copy assignment does not copy betSizes which is constant.
	ActionAbstraction& operator=(const ActionAbstraction& other);

	// action must be an element of the vector legalActions.
	void setAction(uint8_t action, egn::GameState& state, uint8_t& nRaises);
	void calculateLegalActions(const egn::GameState& state, uint8_t nRaises);

	// Actions are indexed using the following order:
	// 0: fold
	// 1: call
	// 2: all-in
	// 3 ~ n-1: raiseSize1 ~ raiseSizeLast
	// legalActions contains all these indexes excluding illegal ones.
	std::vector<uint8_t> legalActions;

private:
	const std::vector<std::vector<std::vector<float>>> betSizes;

}; // ActionAbstraction

} // abc

#endif // ABC_ACTIONABSTRACTION_H