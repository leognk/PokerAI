#include "../AbstractInfoset/TreeTraverser.h"
#include "../Blueprint/Constants.h"
#include "../LosslessAbstraction/hand_index.h"

int main()
{
	auto startTime = std::chrono::high_resolution_clock::now();

	abc::TreeTraverser traverser(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, true);

	std::vector<std::vector<abc::TreeTraverser::seq_t>> actionSeqs = traverser.traverseTree();

	// Count total number of nodes including card abstraction.
	const bp::bckSize_t infoAbcSizes[] = { abc::PREFLOP_SIZE, bp::N_BCK, bp::N_BCK, bp::N_BCK };
	uint64_t nNodes = 0;
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r)
		nNodes += infoAbcSizes[r] * actionSeqs[r].size();
	uint64_t memory = nNodes * sizeof(int32_t)
		+ infoAbcSizes[egn::PREFLOP] * actionSeqs[egn::PREFLOP].size() * sizeof(uint32_t);

	auto t = std::chrono::high_resolution_clock::now();
	double duration = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(t - startTime).count();

	std::cout
		<< "TOTAL with card abc\n"
		<< "nodes: " << nNodes << " (" << std::round(nNodes * 1e-8) * 1e-1 << "G)"
		<< " | memory: " << memory << " (" << std::round(memory * 1e-8) * 1e-1 << "Go)"
		<< " | " << std::round(duration) << "s\n";
}