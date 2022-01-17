#ifndef ABC_OCHSCALCULATOR_H
#define ABC_OCHSCALCULATOR_H

#include "DKEM.h"
#include "EquityCalculator.h"

namespace abc {

static const std::string rivOCHSLUTPath = hsLutDir + "RIV_OCHS_LUT.bin";

// Size of the vector of OCHS.
// It is the number of buckets used for clustering preflop's hands.
static const uint8_t OCHS_SIZE = 8;

// Maximum value for each entry of a normalized OCHS vector.
static const uint16_t MAX_OCHS_VALUE = 1000;

// Class for computing Opponent Cluster Hand Strength (OCHS).
class OCHSCalculator
{
public:
	static void populateRivOCHSLUT();
	static void saveRivOCHSLUT();
	static void loadRivOCHSLUT();

	static std::array<std::array<uint16_t, OCHS_SIZE>, CMB_RIVER_SIZE> RIV_OCHS_LUT;

private:
	static std::array<uint16_t, OCHS_SIZE> calculateRivOCHS(const uint8_t hand[]);

	static omp::HandEvaluator eval;

}; // OCHSCalculator

} // abc

#endif ABC_OCHSCALCULATOR_H