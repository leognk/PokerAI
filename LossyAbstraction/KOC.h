#ifndef ABC_KOC_H
#define ABC_KOC_H

#include "OCHSCalculator.h"

namespace abc {

// Class generating lossy information abstraction
// with k-means on Opponent Cluster Hand Strength.
// bck for bucket
template<typename bckSize_t, bckSize_t nBck>
class KOC
{
public:

	// Set rngSeed to 0 to set a random seed.
	KOC(unsigned kMeansNRestarts = 0, unsigned kMeansMaxIter = 0,
		uint64_t kMeansInvTolerance = 0, unsigned kMeansRngSeed = 0,
		KMeansInitMode kmeansInitMode = KMeansInitMode::PlusPlus,
		KMeansIterMode kmeansIterMode = KMeansIterMode::Elkan) :
		kmeans(false, kMeansNRestarts, kMeansMaxIter, kMeansInvTolerance,
			kMeansRngSeed, kmeansInitMode, kmeansIterMode)
	{
	}

	void populateRivBckLUT()
	{
		ochs.loadRivOCHSLUT();
		std::tie(rivInertia, rivMinWeight) =
			kmeans.buildClusters(ochs.RIV_OCHS_LUT, RIV_BCK_LUT, RIV_CENTERS);
	}

	static void saveRivBckLUT()
	{
		opt::saveArray(RIV_BCK_LUT,
			bckLutDir + std::format("RIV_{}_BCK_LUT.bin", nBck));
		// Write clusters' centers.
		opt::saveArray(RIV_CENTERS,
			bckLutDir + std::format("RIV_{}_CENTERS.bin", nBck));
		// Write inertia and min weight.
		auto file = std::fstream(
			bckLutDir + std::format("RIV_{}_BCK - inertia={}, min_weight={}",
				nBck, rivInertia, rivMinWeight),
			std::ios::out);
		file.close();
	}

	static void loadRivBckLUT()
	{
		opt::loadArray(RIV_BCK_LUT,
			bckLutDir + std::format("RIV_BCK_{}_LUT.bin", nBck));
	}

	static std::array<bckSize_t, CMB_RIVER_SIZE> RIV_BCK_LUT;

private:

	static std::array<std::array<uint16_t, OCHS_SIZE>, nBck> RIV_CENTERS;

	static uint64_t rivInertia;
	static uint32_t rivMinWeight;

	static OCHSCalculator ochs;
	KMeans<bckSize_t, nBck> kmeans;

}; // KOC

// Initialize static members.

template<typename bckSize_t, bckSize_t nBck>
std::array<bckSize_t, CMB_RIVER_SIZE> KOC<bckSize_t, nBck>::RIV_BCK_LUT;

template<typename bckSize_t, bckSize_t nBck>
std::array<std::array<uint16_t, OCHS_SIZE>, nBck> KOC<bckSize_t, nBck>::RIV_CENTERS;

template<typename bckSize_t, bckSize_t nBck>
uint64_t KOC<bckSize_t, nBck>::rivInertia;

template<typename bckSize_t, bckSize_t nBck>
uint32_t KOC<bckSize_t, nBck>::rivMinWeight;

template<typename bckSize_t, bckSize_t nBck>
OCHSCalculator KOC<bckSize_t, nBck>::ochs;

} // abc

#endif // ABC_KOC_H