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

	// Memory for the regrets and the preflop final strategy.
	uint64_t memory = nNodes * sizeof(int32_t)
		+ bp::N_BCK_PREFLOP * actionSeqs[egn::PREFLOP].size() * sizeof(uint32_t);

	const std::string duration = opt::prettyDuration(startTime);

	std::cout
		<< "TOTAL with card abc\n"
		<< "nodes: " << opt::prettyNum(nNodes, 2, true)
		<< " | memory: " << opt::prettyNum(memory, 2, true) << "o"
		<< " | " << duration << "\n";
}