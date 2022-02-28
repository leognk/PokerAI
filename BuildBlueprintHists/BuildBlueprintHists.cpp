#include "../Blueprint/Blueprint.h"
#include "../Utils/Histogram.h"
#include "../Utils/VectorMemory.h"

int main()
{
	const size_t nBins = 1000;

	const std::string histDir = "../data/Blueprint/Tests/Histograms/" + bp::blueprintName() + "/";

	// Create histograms folder.
	std::filesystem::create_directory(histDir);

	bp::Blueprint blueprint(bp::BLUEPRINT_GAME_NAME, bp::BLUEPRINT_BUILD_NAME);
	blueprint.loadRegrets();

	// Histogram of all regrets.
	opt::buildAndSaveHist(blueprint.regrets, nBins, histDir + "AllRegretsHist.bin");

	// Histogram of regrets for each round.
	auto file = std::fstream(histDir + "RoundRegretsHist.bin", std::ios::in | std::ios::binary);
	for (const auto& roundRegrets : blueprint.regrets)
		opt::buildAndSaveHist(roundRegrets, nBins, file);
	file.close();

	// Histogram of regrets for each hand bucket.
	file = std::fstream(histDir + "HandRegretsHist.bin", std::ios::in | std::ios::binary);
	for (const auto& roundRegrets : blueprint.regrets) {
		for (const auto& handRegrets : roundRegrets)
			opt::buildAndSaveHist(handRegrets, nBins, file);
	}
	file.close();

	opt::freeVectMem(blueprint.regrets);
}