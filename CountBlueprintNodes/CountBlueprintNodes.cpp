#include "../Blueprint/CountNodes.cpp"

int main()
{
	std::vector<uint32_t> nInfoBuckets = { 169, 200, 200, 200 };
	egn::chips ante = 0, bigBlind = 100, initialStake = 20000;

	uint64_t nNodes = 0;
	for (uint8_t i = 0; i < egn::N_ROUNDS; ++i) {
		uint64_t nActionNodes = 0;
		bp::countRoundNodes(ante, bigBlind, initialStake, egn::Round(i), nActionNodes);
		std::cout << "\n";
		nNodes += nInfoBuckets[i] * nActionNodes;
	}

	std::cout
		<< "TOTAL (" << std::to_string(opt::MAX_PLAYERS) << " players)\n"
		<< "nodes: " << nNodes
		<< (" (" + std::to_string((uint32_t)std::round(nNodes / 1e6)) + "M)")
		<< "\n";
}