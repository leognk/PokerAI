#include "../AbstractInfoset/ActionSeqIndexer.h"
#include "../Blueprint/Constants.h"
#include <chrono>

int main()
{
	int nThreads = 8;
	double gamma = 2.0;

	abc::ActionSeqIndexer indexer(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES,
		bp::ACTION_SEQ_INDEXER_NAME, nThreads, gamma);

	auto startTime = std::chrono::high_resolution_clock::now();

	indexer.buildMPHF();

	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
	std::cout << "Duration: " << std::round(duration) << " sec\n";

	indexer.saveMPHF();
}