#include "../AbstractInfoset/TreeTraverser.h"
#include "../Blueprint/Constants.h"
#include "../LosslessAbstraction/hand_index.h"

int main()
{
	abc::TreeTraverser traverser(
		bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES, false, true);

	uint64_t nNodes = 0;
	for (uint8_t i = 0; i < egn::N_ROUNDS; ++i) {
		bp::bckSize_t nBck = (i == egn::PREFLOP) ? abc::PREFLOP_SIZE : bp::N_BCK;
		uint64_t nActionNodes = traverser.traverseRoundTree(egn::Round(i));
		std::cout << "\n";
		nNodes += nBck * nActionNodes;
	}

	std::cout
		<< "TOTAL (" << std::to_string(opt::MAX_PLAYERS) << " players)\n"
		<< "nodes: " << nNodes
		<< (" (" + std::to_string((uint32_t)std::round(nNodes / 1e6)) + "M)")
		<< "\n";
}