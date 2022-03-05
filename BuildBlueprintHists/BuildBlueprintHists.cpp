#include "../Blueprint/Blueprint.h"
#include "../Utils/Histogram.h"
#include "../Utils/VectorMemory.h"

int main()
{
	const size_t nBins = 100;

	const std::string histDir = "../data/Blueprint/Tests/Histograms/" + bp::blueprintName() + "/";

	// Create histograms folder.
	std::filesystem::create_directory(histDir);

	bp::Blueprint blueprint(bp::BLUEPRINT_GAME_NAME, bp::BLUEPRINT_BUILD_NAME);
	blueprint.loadRegrets();

	// Histogram of all regrets.
	auto file = std::fstream(
		histDir + std::format("AllRegretsHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	opt::buildAndSaveHist(nBins, blueprint.regrets, file, "symlog");

	// Histogram of regrets for each round.
	file = std::fstream(
		histDir + std::format("RoundRegretsHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	for (const auto& roundRegrets : blueprint.regrets)
		opt::buildAndSaveHist(nBins, roundRegrets, file, "symlog");
	file.close();

	// Histogram of regrets for each hand bucket.
	file = std::fstream(
		histDir + std::format("HandRegretsHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	for (const auto& roundRegrets : blueprint.regrets) {
		for (const auto& handRegrets : roundRegrets)
			opt::buildAndSaveHist(nBins, handRegrets, file, "symlog");
	}
	file.close();

	opt::freeVectMem(blueprint.regrets);
	blueprint.loadStrat();

	// Histogram of all probas.
	file = std::fstream(
		histDir + std::format("AllProbasHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	opt::buildAndSaveHist(nBins, blueprint.strat, file, "linear");

	// Histogram of probas for each round.
	file = std::fstream(
		histDir + std::format("RoundProbasHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	for (const auto& roundStrat : blueprint.strat)
		opt::buildAndSaveHist(nBins, roundStrat, file, "linear");
	file.close();

	// Histogram of probas for each hand bucket.
	file = std::fstream(
		histDir + std::format("HandProbasHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	for (const auto& roundStrat : blueprint.strat) {
		for (const auto& handStrat : roundStrat)
			opt::buildAndSaveHist(nBins, handStrat, file, "linear");
	}
	file.close();
}