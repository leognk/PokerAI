
#include "../LossyAbstraction/DKEM.h"

template<typename cluSize_t, typename feature_t, uint32_t nSamples, uint8_t nFeatures>
uint64_t calculateInertia(
	const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
	const std::vector<std::vector<feature_t>>& centers,
	const std::vector<cluSize_t>& labels)
{
	uint64_t inertia = 0;
	for (uint32_t i = 0; i < nSamples; ++i)
		inertia += abc::emdSq(data[i], centers[labels[i]]);
#pragma warning(suppress: 4244)
	return std::round(std::sqrt(inertia));
}

int main()
{
	const std::string vizDir = opt::dataDir + "AbstractionSaves/Tests/VisualizePreflopClustering/";

	// Load data.
	const uint8_t nBck = 8;
	abc::EquityCalculator eqt;
	eqt.loadPreflopHSHists();
	std::array<uint8_t, abc::PREFLOP_SIZE> PREFLOP_BCK_LUT_REF;
	opt::loadArray(PREFLOP_BCK_LUT_REF, vizDir + "PREFLOP_8_BCK_LUT_REF.bin");
	std::vector<uint8_t> labels(abc::PREFLOP_SIZE);
	std::copy(PREFLOP_BCK_LUT_REF.begin(), PREFLOP_BCK_LUT_REF.end(), labels.begin());
	std::vector<std::vector<uint16_t>> centers(nBck, std::vector<uint16_t>(abc::N_BINS));

	// Calculate weight in clusters.
	std::array<uint32_t, nBck> weightInClusters{};
	for (uint8_t label : labels)
		++weightInClusters[label];
	uint32_t minWeight = weightInClusters[0];
	for (uint32_t weight : weightInClusters)
		if (weight < minWeight) minWeight = weight;

	// Calculate centers and inertia.
	for (uint8_t c = 0; c < nBck; ++c)
		abc::emdCenter(eqt.PREFLOP_HS_HISTS, labels, c, weightInClusters[c], centers[c]);
	std::array<std::array<uint16_t, abc::N_BINS>, nBck> refCenters;
	for (uint8_t i = 0; i < nBck; ++i)
		std::copy(centers[i].begin(), centers[i].end(), refCenters[i].begin());
	uint64_t inertia = calculateInertia(eqt.PREFLOP_HS_HISTS, centers, labels);

	// Write clusters' centers.
	opt::saveArray(refCenters, vizDir + "PREFLOP_8_CENTERS_REF.bin");
	// Write inertia and min weight.
	auto file = opt::fstream(
		vizDir + std::format("PREFLOP_8_BCK_REF - inertia={}, min_weight={}", inertia, minWeight),
		std::ios::out);
	file.close();
}