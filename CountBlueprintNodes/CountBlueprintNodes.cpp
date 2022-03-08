#include "../AbstractInfoset/TreeTraverser.h"
#include "../Blueprint/Constants.h"

int main()
{
	opt::time_t startTime = opt::getTime();

	abc::TreeTraverser traverser(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, true);

	std::vector<std::vector<abc::TreeTraverser::seq_t>> actionSeqs = traverser.traverseTree();

	// Count total number of nodes including card abstraction.
	uint64_t nNodes =
		bp::N_BCK_PREFLOP * actionSeqs[egn::PREFLOP].size()
		+ bp::N_BCK_FLOP * actionSeqs[egn::FLOP].size()
		+ bp::N_BCK_TURN * actionSeqs[egn::TURN].size()
		+ bp::N_BCK_RIVER * actionSeqs[egn::RIVER].size();

	// Memory for the regrets.
	typedef int32_t regret_t;
	uint64_t ram = nNodes * sizeof(regret_t);

	// Memory for the snapshots and the checkpoint stored to the disk.
	typedef uint8_t strat_t;
	uint64_t stratHdd = nNodes * sizeof(strat_t);
	uint64_t hdd = (bp::nSnapshots + 1) * stratHdd + ram;

	const std::string duration = opt::prettyDuration(startTime);

	std::cout
		<< "TOTAL with card abc\n"
		<< "nodes: " << opt::prettyNum(nNodes, 2, true)
		<< " | RAM: " << opt::prettyNum(ram, 2, true) << "o"
		<< " | HDD: " << opt::prettyNum(hdd, 2, true) << "o"
		<< " | strat HDD: " << opt::prettyNum(stratHdd, 2, true) << "o"
		<< " | " << duration << "\n";
}