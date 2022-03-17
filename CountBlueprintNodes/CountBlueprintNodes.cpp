#include "../AbstractInfoset/TreeTraverser.h"
#include "../Blueprint/Constants.h"

int main()
{
	const opt::time_t startTime = opt::getTime();

	abc::TreeTraverser traverser(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, true);

	std::vector<std::vector<abc::TreeTraverser::seq_t>> actionSeqs = traverser.traverseTree();

	// Count total number of nodes including card abstraction.
	const uint64_t nPreflopNodes = bp::N_BCK_PREFLOP * actionSeqs[egn::PREFLOP].size();
	const uint64_t nFlopNodes = bp::N_BCK_FLOP * actionSeqs[egn::FLOP].size();
	const uint64_t nTurnNodes = bp::N_BCK_TURN * actionSeqs[egn::TURN].size();
	const uint64_t nRiverNodes = bp::N_BCK_RIVER * actionSeqs[egn::RIVER].size();
	const uint64_t nNodes = nPreflopNodes + nFlopNodes + nTurnNodes + nRiverNodes;
	const uint64_t maxNRoundNodes = std::max(nPreflopNodes,
		std::max(nFlopNodes, std::max(nTurnNodes, nRiverNodes)));

	// Memory for the regrets.
	typedef int32_t regret_t;
	const uint64_t regretMem = nNodes * sizeof(regret_t);

	// Memory for averaging the snapshots.
	typedef uint32_t sumStrat_t;
	const uint64_t avgSnapshotsMem = maxNRoundNodes * sizeof(sumStrat_t);

	// Memory for the strategy stored to the disk.
	typedef uint8_t strat_t;
	const uint64_t stratHdd = nNodes * sizeof(strat_t);

	// RAM
	// stratHdd is used when evaluating the strategy.
	const uint64_t ram = regretMem + std::max(avgSnapshotsMem, stratHdd);

	// Memory for the snapshots and the checkpoint stored to the disk.
	const uint64_t hdd = (bp::nSnapshots + 1) * stratHdd + regretMem;

	const std::string duration = opt::prettyDuration(startTime);

	std::cout
		<< "TOTAL with card abc\n"
		<< "nodes: " << opt::prettyNum(nNodes, 2, true)
		<< " | RAM: " << opt::prettyNum(ram, 2, true) << "o"
		<< " | HDD: " << opt::prettyNum(hdd, 2, true) << "o"
		<< " | strat HDD: " << opt::prettyNum(stratHdd, 2, true) << "o"
		<< " | " << duration << "\n";
}