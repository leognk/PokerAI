#ifndef BP_BLUEPRINTCALCULATOR_H
#define BP_BLUEPRINTCALCULATOR_H

#include "Constants.h"
#include "../AbstractInfoset/AbstractInfoset.h"
#include "../AbstractInfoset/AbstractInfosetDebug.h"
#include "../AbstractInfoset/GroupedActionSeqs.h"
#include "../Utils/FastVector.h"
#include "../Utils/Progression.h"
#include "../Utils/HardwareUsage.h"
#include "../Utils/ioVar.h"
#include <filesystem>

namespace bp {

static const std::string blueprintDir = "../data/Blueprint/" + BLUEPRINT_NAME + "/";
static const std::string blueprintTmpDir = blueprintDir + "tmp/";

static const std::string snapshotPath = blueprintTmpDir + "SNAPSHOT";
static const std::string checkpointPath = blueprintTmpDir + "CHECKPOINT.bin";
static const std::string constantPath = blueprintDir + "CONSTANTS.txt";
static const std::string stratPath = blueprintDir + "STRATEGY";

typedef int32_t regret_t;
typedef uint32_t sumRegret_t;
typedef uint16_t strat_t;
typedef uint32_t sumStrat_t;
typedef abc::AbstractInfoset<bckSize_t, N_BCK_PREFLOP, N_BCK_FLOP, N_BCK_TURN, N_BCK_RIVER> abcInfo_t;

class BlueprintCalculator
{
public:

	// Set rngSeed to 0 to set a random seed.
	BlueprintCalculator(unsigned rngSeed = 0, bool verbose = true);

	// Conduct MCCFR and save the final strategy to the disk.
	void buildStrategy();
	void oneIter();

	static std::string getSnapshotPath(unsigned snapshotId, uint8_t roundId);
	static std::string getStratPath(uint8_t roundId);

	uint64_t currIter;

	abcInfo_t abcInfo;

private:

	typedef omp::XoroShiro128Plus Rng;

	std::array<uint8_t, 2> buildPruneCumWeights();

	uint8_t nActions() const;
	regret_t& getRegret(uint8_t actionId);
	const regret_t getRegret(uint8_t actionId) const;
	void calculateCumRegrets();
	sumRegret_t calculateSumRegrets() const;

	void applyDiscounting();
	void traverseMCCFR(uint8_t traverser);
	void traverseMCCFRP(uint8_t traverser);
	egn::dchips calculateExpectedValue() const;
	egn::dchips calculateExpectedValueP() const;

	void takeSnapshot();
	void averageSnapshots();

	void writeConstants() const;
	void verifyConstants() const;
	template<typename T>
	void verifyOneConstant(std::ifstream& file, const T& v) const;

	void updateCheckpoint();
	void loadCheckpoint(std::fstream& file);

	void printProgress() const;
	void printFinalStats() const;

	bool verbose;

	Rng rng;
	opt::FastRandomChoice<7> pruneRandChoice;
	opt::FastRandomChoiceRNGRescale<16> actionRandChoice;
	static const opt::FastRandomChoice<15> cumWeightsRescaler;
	const std::array<uint8_t, 2> pruneCumWeights;
	std::vector<sumRegret_t> cumRegrets;

	std::vector<std::vector<std::vector<regret_t>>> regrets;

	double extraDuration;
	opt::time_t startTime;
	unsigned nextSnapshotId;
	uint64_t lastCheckpointIter;

	abc::GroupedActionSeqs gpSeqs;

	// Variables used for DFS.
	std::vector<uint8_t> stack;
	std::vector<bool> firstAction;
	opt::FastVector<abcInfo_t> hist;
	std::vector<bool> lastChild;
	std::vector<egn::dchips> expVals;
	std::vector<bool> visited;

	static const std::string printSep;

}; // BlueprintCalculator

} // bp

#endif // BP_BLUEPRINTCALCULATOR_H