#ifndef ABC_ACTIONABSTRACTION_H
#define ABC_ACTIONABSTRACTION_H

#include "../GameEngine/GameState.h"
#include <vector>

namespace abc {

typedef std::vector<std::vector<std::vector<float>>> betSizes_t;

// Legal actions ordered according to their respective indices in legalActions.
enum AbcAction { FOLD, CALL, ALLIN, RAISE };

inline std::string abcActionToStr(uint8_t a)
{
	switch (a) {
	case FOLD: return "FOLD";
	case CALL: return "CALL";
	case ALLIN: return "ALLIN";
	default: return "RAISE" + std::to_string(a - RAISE);
	}
}

class ActionAbstraction
{
public:

	typedef omp::XoroShiro128Plus Rng;

	// betSizes is the available bet sizes with action abstraction
	// expressed in terms of fraction of the pot.
	// shape: n_rounds x n_raises_in_round x n_possible_bet_sizes
	ActionAbstraction(const betSizes_t& betSizes);

	// The copy assignment does not copy betSizes which is constant.
	ActionAbstraction& operator=(const ActionAbstraction& other);

	// action must be an element of the vector legalActions.
	void setAction(
		uint8_t action, egn::GameState& state,
		uint8_t& nRaises, bool incrNRaises = true);
	void calculateLegalActions(const egn::GameState& state, uint8_t nRaises);
	// Return all the possible actions without taking
	// into account minRaise and maxRaise.
	std::vector<uint8_t> calculateAllLegalActions(
		const egn::GameState& state, uint8_t nRaises) const;

	uint8_t mapActionToAbcAction(
		const egn::GameState& state, uint8_t nRaises, Rng& rng);
	uint8_t mapActionToFoldCall(
		const egn::GameState& state, Rng& rng);

	egn::chips actionToBet(
		uint8_t action, const egn::GameState& state, uint8_t nRaises) const;

	// Actions are indexed using the following order:
	// 0: fold
	// 1: call
	// 2: all-in
	// 3 ~ n-1: raiseSize1 ~ raiseSizeLast
	// legalActions contains all these indexes excluding illegal ones.
	std::vector<uint8_t> legalActions;

private:

	static egn::chips betSizeToBet(const float betSize, const egn::GameState& state);
	static float betToBetSize(const egn::chips bet, const egn::GameState& state);

	static float actionMappingFunction(const float a, const float b, const float x);

	const betSizes_t* betSizes;

	uint8_t beginRaiseId, endRaiseId;
	float minRaiseSize, allinSize;

	static opt::FastRandomChoice<8> betSizeRandChoice;
	std::array<uint16_t, 2> betSizesCumWeights;

}; // ActionAbstraction

} // abc

#endif // ABC_ACTIONABSTRACTION_H