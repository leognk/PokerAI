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
	uint64_t nNodesPreflop = bp::N_BCK_PREFLOP * actionSeqs[egn::PREFLOP].size();
	uint64_t nNodesNoPreflop =
		bp::N_BCK_FLOP * actionSeqs[egn::FLOP].size()
		+ bp::N_BCK_TURN * actionSeqs[egn::TURN].size()
		+ bp::N_BCK_RIVER * actionSeqs[egn::RIVER].size();
	uint64_t nNodes = nNodesPreflop + nNodesNoPreflop;

	// Memory for the regrets and the preflop final strategy.
	uint64_t ramMem = nNodes * sizeof(int32_t) + nNodesPreflop * sizeof(uint32_t);

	// Memory for the snapshots and the final strategy stored to the disk.
	uint64_t stratHddMem = nNodes * sizeof(uint16_t);
	uint64_t hddMem = bp::nSnapshots * nNodesNoPreflop * sizeof(uint16_t) + stratHddMem;

	const std::string duration = opt::prettyDuration(startTime);

	std::cout
		<< "TOTAL with card abc\n"
		<< "nodes: " << opt::prettyNum(nNodes, 2, true)
		<< " | RAM: " << opt::prettyNum(ramMem, 2, true) << "o"
		<< " | HDD: " << opt::prettyNum(hddMem, 2, true) << "o"
		<< " | strat HDD: " << opt::prettyNum(stratHddMem, 2, true) << "o"
		<< " | " << duration << "\n";
}