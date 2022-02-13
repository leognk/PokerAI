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

	uint8_t nActions() const;
	regret_t& getRegret(uint8_t actionId);
	const regret_t getRegret(uint8_t actionId) const;
	void calculateCumRegrets();
	sumRegret_t calculateSumRegrets() const;

	void applyDiscounting();
	void updatePreflopStrat(uint8_t traverser);
	void traverseMCCFR(uint8_t traverser);
	void traverseMCCFRP(uint8_t traverser);
	egn::dchips calculateExpectedValue() const;

	void takeSnapshot();
	void averageSnapshots();
	void updateCheckpoint();

	Rng rng;
	opt::FastRandomChoice<8> pruneRandChoice;
	opt::FastRandomChoiceRNGRescale<16> actionRandChoice;
	const std::array<uint8_t, 2> pruneCumWeights;
	std::vector<sumRegret_t> cumRegrets;

	std::vector<std::vector<std::vector<regret_t>>> regrets;
	std::vector<std::vector<std::vector<strat_t>>> finalStrat;

	uint64_t currIter;

	abcInfo_t abcInfo;

	// Variables used for DFS.
	std::vector<uint8_t> stack;
	opt::FastVector<abcInfo_t> hist;
	std::vector<bool> lastChild;
	std::vector<egn::dchips> expVals;

}; // BlueprintCalculator

} // bp

#endif // BP_BLUEPRINTCALCULATOR_H