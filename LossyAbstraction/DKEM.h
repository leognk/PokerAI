#ifndef ABC_DKEM_H
#define ABC_DKEM_H

#include <format>
#include "EquityCalculator.h"
#include "KMeans.h"
#include "../Utils/ioArray.h"

namespace abc {
	
static const std::string bckLutDir = "../data/AbstractionSaves/BCK_LUT/";

// Class generating lossy information abstraction
// with distribution-aware k-means earth mover's distance.
// bck for bucket
template<typename bckSize_t, bckSize_t nBck>
class DKEM
{
public:

	// Set rngSeed to 0 to set a random seed.
#pragma warning(suppress: 26495)
	DKEM(unsigned kMeansNRestarts, unsigned kMeansMaxIter, unsigned kMeansRngSeed = 0) :
		kmeans(false, kMeansNRestarts, kMeansMaxIter, kMeansRngSeed)
	{
	}

	void populatePreflopBckLUT()
	{
		eqt.loadPreflopHSHists();
		preflopMinWeight = kmeans.buildClusters(eqt.PREFLOP_HS_HISTS, PREFLOP_BCK_LUT);
	}

	void populateFlopBckLUT()
	{
		eqt.loadFlopHSHists();
		flopMinWeight = kmeans.buildClusters(eqt.FLOP_HS_HISTS, FLOP_BCK_LUT);
	}

	void populateTurnBckLUT()
	{
		eqt.loadTurnHSHists();
		turnMinWeight = kmeans.buildClusters(eqt.TURN_HS_HISTS, TURN_BCK_LUT);
	}

	static void savePreflopBckLUT()
	{
		opt::saveArray(PREFLOP_BCK_LUT,
			bckLutDir + std::format("PREFLOP_BCK_{}_LUT.bin", nBck));
		// Write min weight.
		auto file = std::fstream(
			bckLutDir + std::format("PREFLOP_BCK_{} - min_weight={}, avg_weight={}",
				nBck, preflopMinWeight, PREFLOP_BCK_LUT.size() / nBck),
			std::ios::out);
		file.close();
	}

	static void saveFlopBckLUT()
	{
		opt::saveArray(FLOP_BCK_LUT,
			bckLutDir + std::format("FLOP_BCK_{}_LUT.bin", nBck));
		// Write min weight.
		auto file = std::fstream(
			bckLutDir + std::format("FLOP_BCK_{} - min_weight={}, avg_weight={}",
				nBck, flopMinWeight, FLOP_BCK_LUT.size() / nBck),
			std::ios::out);
		file.close();
	}

	static void saveTurnBckLUT()
	{
		opt::saveArray(TURN_BCK_LUT,
			bckLutDir + std::format("TURN_BCK_{}_LUT.bin", nBck));
		// Write min weight.
		auto file = std::fstream(
			bckLutDir + std::format("TURN_BCK_{} - min_weight={}, avg_weight={}",
				nBck, turnMinWeight, TURN_BCK_LUT.size() / nBck),
			std::ios::out);
		file.close();
	}

	static void loadPreflopBckLUT()
	{
		opt::loadArray(PREFLOP_BCK_LUT,
			bckLutDir + std::format("PREFLOP_BCK_{}_LUT.bin", nBck));
	}

	static void loadFlopBckLUT()
	{
		opt::loadArray(FLOP_BCK_LUT,
			bckLutDir + std::format("FLOP_BCK_{}_LUT.bin", nBck));
	}

	static void loadTurnBckLUT()
	{
		opt::loadArray(TURN_BCK_LUT,
			bckLutDir + std::format("TURN_BCK_{}_LUT.bin", nBck));
	}

	static std::array<bckSize_t, PREFLOP_SIZE> PREFLOP_BCK_LUT;
	static std::array<bckSize_t, FLOP_SIZE> FLOP_BCK_LUT;
	static std::array<bckSize_t, CMB_TURN_SIZE> TURN_BCK_LUT;

private:

	static uint64_t preflopMinWeight, flopMinWeight, turnMinWeight;

	static EquityCalculator eqt;
	KMeans<bckSize_t, nBck> kmeans;

}; // DKEM

// Initialize static members.
template<typename bckSize_t, bckSize_t nBck>
std::array<bckSize_t, PREFLOP_SIZE> DKEM<bckSize_t, nBck>::PREFLOP_BCK_LUT;
template<typename bckSize_t, bckSize_t nBck>
std::array<bckSize_t, FLOP_SIZE> DKEM<bckSize_t, nBck>::FLOP_BCK_LUT;
template<typename bckSize_t, bckSize_t nBck>
std::array<bckSize_t, CMB_TURN_SIZE> DKEM<bckSize_t, nBck>::TURN_BCK_LUT;
template<typename bckSize_t, bckSize_t nBck>
uint64_t DKEM<bckSize_t, nBck>::preflopMinWeight;
template<typename bckSize_t, bckSize_t nBck>
uint64_t DKEM<bckSize_t, nBck>::flopMinWeight;
template<typename bckSize_t, bckSize_t nBck>
uint64_t DKEM<bckSize_t, nBck>::turnMinWeight;
template<typename bckSize_t, bckSize_t nBck>
EquityCalculator DKEM<bckSize_t, nBck>::eqt;

} // abc

#endif // ABC_DKEM_H