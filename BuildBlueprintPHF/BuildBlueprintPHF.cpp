#include "../AbstractInfoset/ActionSeqIndexer.h"
#include "../Blueprint/Constants.h"
#include <chrono>

int main()
{
	const std::string indexerName = "BLUEPRINT";
	int nThreads = 4;
	double gamma = 2.0;

	abc::ActionSeqIndexer indexer(
		bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES,
		indexerName, nThreads, gamma);

	auto startTime = std::chrono::high_resolution_clock::now();
	indexer.buildPHF();
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
	std::cout << "Duration: " << std::round(duration) << "s\n";

	indexer.savePHF();
}