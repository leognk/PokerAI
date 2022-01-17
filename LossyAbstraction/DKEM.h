#ifndef ABC_DKEM_H
#define ABC_DKEM_H

#include <format>
#include "EquityCalculator.h"
#include "KMeans.h"
#include "../Utils/ioContainer.h"

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
	DKEM(unsigned kMeansNRestarts = 0, unsigned kMeansMaxIter = 0, unsigned kMeansRngSeed = 0) :
		kmeans(true, kMeansNRestarts, kMeansMaxIter, kMeansRngSeed)
	{
	}

	void populatePreflopBckLUT()
	{
		eqt.loadPreflopHSHists();
		std::tie(preflopInertia, preflopMinWeight) =
			kmeans.buildClusters(eqt.PREFLOP_HS_HISTS, PREFLOP_BCK_LUT, PREFLOP_CENTERS);
	}

	void populateFlopBckLUT()
	{
		eqt.loadFlopHSHists();
		std::tie(flopInertia, flopMinWeight) =
			kmeans.buildClusters(eqt.FLOP_HS_HISTS, FLOP_BCK_LUT, FLOP_CENTERS);
	}

	void populateTurnBckLUT()
	{
		eqt.loadTurnHSHists();
		std::tie(turnInertia, turnMinWeight) =
			kmeans.buildClusters(eqt.TURN_HS_HISTS, TURN_BCK_LUT, TURN_CENTERS);
	}

	static void savePreflopBckLUT()
	{
		opt::saveArray(PREFLOP_BCK_LUT,
			bckLutDir + std::format("PREFLOP_{}_BCK_LUT.bin", nBck));
		// Write clusters' centers.
		opt::saveArray(PREFLOP_CENTERS,
			bckLutDir + std::format("PREFLOP_{}_CENTERS.bin", nBck));
		// Write inertia and min weight.
		auto file = std::fstream(
			bckLutDir + std::format("PREFLOP_{}_BCK - inertia={}, min_weight={}",
				nBck, preflopInertia, preflopMinWeight),
			std::ios::out);
		file.close();
	}

	static void saveFlopBckLUT()
	{
		opt::saveArray(FLOP_BCK_LUT,
			bckLutDir + std::format("FLOP_{}_BCK_LUT.bin", nBck));
		// Write clusters' centers.
		opt::saveArray(FLOP_CENTERS,
			bckLutDir + std::format("FLOP_{}_CENTERS.bin", nBck));
		// Write inertia and min weight.
		auto file = std::fstream(
			bckLutDir + std::format("FLOP_{}_BCK - inertia={}, min_weight={}",
				nBck, flopInertia, flopMinWeight),
			std::ios::out);
		file.close();
	}

	static void saveTurnBckLUT()
	{
		opt::saveArray(TURN_BCK_LUT,
			bckLutDir + std::format("TURN_{}_BCK_LUT.bin", nBck));
		// Write clusters' centers.
		opt::saveArray(TURN_CENTERS,
			bckLutDir + std::format("TURN_{}_CENTERS.bin", nBck));
		// Write inertia and min weight.
		auto file = std::fstream(
			bckLutDir + std::format("TURN_{}_BCK - inertia={}, min_weight={}",
				nBck, turnInertia, turnMinWeight),
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

	static std::array<std::array<uint16_t, N_BINS>, nBck> PREFLOP_CENTERS;
	static std::array<std::array<uint16_t, N_BINS>, nBck> FLOP_CENTERS;
	static std::array<std::array<uint8_t, N_BINS>, nBck> TURN_CENTERS;

	static uint64_t preflopInertia, flopInertia, turnInertia;
	static uint32_t preflopMinWeight, flopMinWeight, turnMinWeight;

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
std::array<std::array<uint16_t, N_BINS>, nBck> DKEM<bckSize_t, nBck>::PREFLOP_CENTERS;
template<typename bckSize_t, bckSize_t nBck>
std::array<std::array<uint16_t, N_BINS>, nBck> DKEM<bckSize_t, nBck>::FLOP_CENTERS;
template<typename bckSize_t, bckSize_t nBck>
std::array<std::array<uint8_t, N_BINS>, nBck> DKEM<bckSize_t, nBck>::TURN_CENTERS;

template<typename bckSize_t, bckSize_t nBck>
uint64_t DKEM<bckSize_t, nBck>::preflopInertia;
template<typename bckSize_t, bckSize_t nBck>
uint64_t DKEM<bckSize_t, nBck>::flopInertia;
template<typename bckSize_t, bckSize_t nBck>
uint64_t DKEM<bckSize_t, nBck>::turnInertia;

template<typename bckSize_t, bckSize_t nBck>
uint32_t DKEM<bckSize_t, nBck>::preflopMinWeight;
template<typename bckSize_t, bckSize_t nBck>
uint32_t DKEM<bckSize_t, nBck>::flopMinWeight;
template<typename bckSize_t, bckSize_t nBck>
uint32_t DKEM<bckSize_t, nBck>::turnMinWeight;

template<typename bckSize_t, bckSize_t nBck>
EquityCalculator DKEM<bckSize_t, nBck>::eqt;

} // abc

#endif // ABC_DKEM_H