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
	auto allRegretsFile = opt::fstream(
		histDir + std::format("AllRegretsHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	opt::buildAndSaveHist(nBins, blueprint.regrets, allRegretsFile, "symlog");
	allRegretsFile.close();

	// Histogram of regrets for each round.
	auto roundRegretsFile = opt::fstream(
		histDir + std::format("RoundRegretsHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	for (const auto& roundRegrets : blueprint.regrets)
		opt::buildAndSaveHist(nBins, roundRegrets, roundRegretsFile, "symlog");
	roundRegretsFile.close();

	// Histogram of regrets for each hand bucket.
	auto handRegretsFile = opt::fstream(
		histDir + std::format("HandRegretsHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	for (const auto& roundRegrets : blueprint.regrets) {
		for (const auto& handRegrets : roundRegrets)
			opt::buildAndSaveHist(nBins, handRegrets, handRegretsFile, "symlog");
	}
	handRegretsFile.close();

	opt::freeVectMem(blueprint.regrets);
	blueprint.loadStrat();

	// Histogram of all probas.
	auto allProbasFile = opt::fstream(
		histDir + std::format("AllProbasHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	opt::buildAndSaveHist(nBins, blueprint.strat, allProbasFile, "linear");
	allProbasFile.close();

	// Histogram of probas for each round.
	auto roundProbasFile = opt::fstream(
		histDir + std::format("RoundProbasHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	for (const auto& roundStrat : blueprint.strat)
		opt::buildAndSaveHist(nBins, roundStrat, roundProbasFile, "linear");
	roundProbasFile.close();

	// Histogram of probas for each hand bucket.
	auto handProbasFile = opt::fstream(
		histDir + std::format("HandProbasHist_{}_bins.bin", nBins),
		std::ios::out | std::ios::binary);
	for (const auto& roundStrat : blueprint.strat) {
		for (const auto& handStrat : roundStrat)
			opt::buildAndSaveHist(nBins, handStrat, handProbasFile, "linear");
	}
	handProbasFile.close();
}