#ifndef BP_BLUEPRINTCALCULATOR_H
#define BP_BLUEPRINTCALCULATOR_H

#include "Constants.h"
#include "EvalBlueprintAI.h"
#include "../AbstractInfoset/AbstractInfoset.h"
#include "../AbstractInfoset/AbstractInfosetDebug.h"
#include "../AbstractInfoset/GroupedActionSeqsInv.h"
#include "../Utils/FastVector.h"
#include "../Utils/Progression.h"
#include "../Utils/HardwareUsage.h"
#include "../Utils/ioVar.h"
#include "../Utils/VectorMemory.h"
#include <filesystem>

namespace bp {

typedef abc::AbstractInfoset<bckSize_t, N_BCK_PREFLOP, N_BCK_FLOP, N_BCK_TURN, N_BCK_RIVER> abcInfo_t;

class BlueprintCalculator
{
public:

	// Set rngSeed to 0 to set a random seed.
	BlueprintCalculator(unsigned rngSeed = 0, bool verbose = true);

	// Conduct MCCFR and save the final strategy to the disk.
	void buildStrategy();
	void oneIter();

	uint64_t currIter;

	abcInfo_t abcInfo;

private:

	typedef omp::XoroShiro128Plus Rng;

	std::array<uint16_t, 2> buildPruneCumWeights();

	uint8_t nActions() const;
	regret_t& getRegret(uint8_t actionId);
	const regret_t getRegret(uint8_t actionId) const;
	void calculateCumRegrets();
	int64_t calculateSumRegrets() const;

	void applyDiscounting();
	void traverseMCCFR(uint8_t traverser);
	void traverseMCCFRP(uint8_t traverser);
	egn::dchips calculateExpectedValue() const;
	egn::dchips calculateExpectedValueP() const;

	void incrNodesCount(uint8_t actionId);
	uint64_t getNUniqueNodes() const;

	void takeSnapshot();
	void averageSnapshots();
	void evaluateStrategy();

	void writeConstants() const;
	void verifyConstants() const;
	template<typename T>
	void verifyOneConstant(std::ifstream& file, const T& v) const;

	void updateCheckpoint();
	void loadCheckpoint(std::fstream& file);

	void printProgress() const;
	void printStratEval() const;
	void printFinalStats() const;

	bool verbose;

	Rng rng;
	static opt::FastRandomChoice<8> pruneRandChoice;
	static opt::FastRandomChoiceRNGRescale<16> actionRandChoice;
	static const opt::FastRandomChoice<> cumWeightsRescaler;
	const std::array<uint16_t, 2> pruneCumWeights;
	std::vector<uint64_t> cumRegrets;

	regrets_t regrets;

	double extraDuration;
	opt::time_t startTime;
	unsigned nextSnapshotId;
	uint64_t lastCheckpointIter;

	abc::GroupedActionSeqs gpSeqs;
	abc::GroupedActionSeqsInv gpSeqsInv;

	// Variables used for DFS.
	std::vector<uint8_t> stack;
	std::vector<bool> firstAction;
	opt::FastVector<abcInfo_t> hist;
	std::vector<bool> lastChild;
	std::vector<egn::dchips> expVals;
	std::vector<bool> visited;

	uint64_t nodesCount;
	uint64_t nodesUniqueCount;
	const uint64_t totUniqueNodes;

	std::vector<float> gainsAvg, gainsStd;
	std::vector<uint64_t> nSnapshotsUsedForEval;

	static const std::string printSep;

}; // BlueprintCalculator

} // bp

#endif // BP_BLUEPRINTCALCULATOR_H