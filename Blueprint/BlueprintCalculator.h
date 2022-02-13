#ifndef BP_BLUEPRINTCALCULATOR_H
#define BP_BLUEPRINTCALCULATOR_H

#include "Constants.h"
#include "../AbstractInfoset/AbstractInfoset.h"
#include "../Utils/FastVector.h"

namespace bp {

typedef int32_t regret_t;
typedef uint32_t sumRegret_t;
typedef uint32_t strat_t;
typedef abc::AbstractInfoset<bckSize_t, N_BCK> abcInfo_t;

class BlueprintCalculator
{
public:

	// Set rngSeed to 0 to set a random seed.
	BlueprintCalculator(unsigned rngSeed = 0);

	// Conduct MCCFR.
	void buildStrategy();
	void saveStrategy();
	void loadStrategy();

private:

	typedef omp::XoroShiro128Plus Rng;

	std::array<uint8_t, 2> buildPruneCumWeights();

	void updateActionRegrets();
	void calculateCumRegrets();
	sumRegret_t calculateSumRegrets();
	void updatePreflopStrat(uint8_t traverser);
	void applyDiscounting();
	void takeSnapshot();
	void averageSnapshots();
	void updateCheckpoint();

	void traverseMCCFR();
	void traverseMCCFRP();

	Rng rng;
	opt::FastRandomChoice<8> pruneRandChoice;
	opt::FastRandomChoiceRNGRescale<16> actionRandChoice;
	const std::array<uint8_t, 2> pruneCumWeights;
	std::vector<sumRegret_t> cumRegrets;
	std::vector<regret_t> actionRegrets;

	std::vector<std::vector<std::vector<regret_t>>> regrets;
	std::vector<std::vector<std::vector<strat_t>>> finalStrat;

	abcInfo_t abcInfo;

	uint64_t currIter;

	// Variables used for DFS.
	std::vector<uint8_t> stack;
	opt::FastVector<abcInfo_t> hist;
	std::vector<bool> lastChild;

}; // BlueprintCalculator

} // bp

#endif // BP_BLUEPRINTCALCULATOR_H