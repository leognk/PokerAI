#ifndef BP_BLUEPRINTCALCULATOR_H
#define BP_BLUEPRINTCALCULATOR_H

#include "Constants.h"
#include "../AbstractInfoset/AbstractInfoset.h"
#include "../AbstractInfoset/GroupedActionSeqs.h"
#include "../Utils/FastVector.h"

namespace bp {

static const std::string blueprintTmpDir = "../data/Blueprint/Temporary/";
static const std::string snapshotPath = blueprintTmpDir + BLUEPRINT_NAME + "_SNAPSHOT_";
static const std::string checkpointPath = blueprintTmpDir + BLUEPRINT_NAME + "_CHECKPOINT_";

static const std::string blueprintStratDir = "../data/Blueprint/Strategy/";
static const std::string stratPath = blueprintStratDir + BLUEPRINT_NAME + "_STRATEGY_";

typedef int32_t regret_t;
typedef uint32_t sumRegret_t;
typedef uint16_t strat_t;
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
	opt::FastRandomChoice<15> cumRegretsRescaler;
	const std::array<uint8_t, 2> pruneCumWeights;
	std::vector<sumRegret_t> cumRegrets;

	std::vector<std::vector<std::vector<regret_t>>> regrets;
	// Non-normalized strategy on preflop.
	std::vector<std::vector<sumRegret_t>> preflopFinalStrat;

	uint64_t currIter;
	uint32_t nextSnapshotId;

	abcInfo_t abcInfo;

	abc::GroupedActionSeqs gpSeqs;

	// Variables used for DFS.
	std::vector<uint8_t> stack;
	opt::FastVector<abcInfo_t> hist;
	std::vector<bool> lastChild;
	std::vector<egn::dchips> expVals;
	std::vector<bool> visited;

}; // BlueprintCalculator

} // bp

#endif // BP_BLUEPRINTCALCULATOR_H