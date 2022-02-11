#ifndef BP_BLUEPRINTCALCULATOR_H
#define BP_BLUEPRINTCALCULATOR_H

#include "Constants.h"
#include "../AbstractInfoset/AbstractInfoset.h"

namespace bp {

typedef uint32_t regret_t;
typedef uint32_t strat_t;

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

	void updatePreflopStrat();
	void applyDiscounting();
	void takeSnapshot();
	void averageSnapshots();
	void updateCheckpoint();

	void traverseMCCFR();
	void traverseMCCFRP();

	Rng rng;
	opt::FastRandomChoice<16> randChoice;
	const std::array<uint8_t, 2> pruneCumWeights;

	std::vector<std::vector<std::vector<regret_t>>> regrets;
	std::vector<std::vector<strat_t>> finalStrategy;

	abc::AbstractInfoset<bckSize_t, N_BCK> abcInfo;

	uint64_t currIter;

}; // BlueprintCalculator

} // bp

#endif // BP_BLUEPRINTCALCULATOR_H