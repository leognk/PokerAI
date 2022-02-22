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
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
class DKEM
{
public:

	// Set rngSeed to 0 to set a random seed.
	DKEM(unsigned kMeansNRestarts = 0, unsigned kMeansMaxIter = 0,
		uint64_t kMeansInvTolerance = 0, unsigned kMeansRngSeed = 0,
		KMeansInitMode kmeansInitMode = KMeansInitMode::PlusPlus,
		KMeansIterMode kmeansIterMode = KMeansIterMode::Elkan) :
		kmeansPreflop(true, kMeansNRestarts, kMeansMaxIter, kMeansInvTolerance,
			kMeansRngSeed, kmeansInitMode, kmeansIterMode),
		kmeansFlop(true, kMeansNRestarts, kMeansMaxIter, kMeansInvTolerance,
			kMeansRngSeed, kmeansInitMode, kmeansIterMode),
		kmeansTurn(true, kMeansNRestarts, kMeansMaxIter, kMeansInvTolerance,
			kMeansRngSeed, kmeansInitMode, kmeansIterMode)
	{
	}

	void populatePreflopBckLUT()
	{
		eqt.loadPreflopHSHists();
		std::tie(preflopInertia, preflopMinWeight) =
			kmeansPreflop.buildClusters(eqt.PREFLOP_HS_HISTS, PREFLOP_BCK_LUT, PREFLOP_CENTERS);
	}

	void populateFlopBckLUT()
	{
		eqt.loadFlopHSHists();
		std::tie(flopInertia, flopMinWeight) =
			kmeansFlop.buildClusters(eqt.FLOP_HS_HISTS, FLOP_BCK_LUT, FLOP_CENTERS);
	}

	void populateTurnBckLUT()
	{
		eqt.loadTurnHSHists();
		std::tie(turnInertia, turnMinWeight) =
			kmeansTurn.buildClusters(eqt.TURN_HS_HISTS, TURN_BCK_LUT, TURN_CENTERS);
	}

	static void savePreflopBckLUT()
	{
		opt::saveArray(PREFLOP_BCK_LUT,
			bckLutDir + std::format("PREFLOP_{}_BCK_LUT.bin", nBckPreflop));
		// Write clusters' centers.
		opt::saveArray(PREFLOP_CENTERS,
			bckLutDir + std::format("PREFLOP_{}_CENTERS.bin", nBckPreflop));
		// Write inertia and min weight.
		auto file = std::fstream(
			bckLutDir + std::format("PREFLOP_{}_BCK - inertia={}, min_weight={}",
				nBckPreflop, preflopInertia, preflopMinWeight),
			std::ios::out);
		file.close();
	}

	static void saveFlopBckLUT()
	{
		opt::saveArray(FLOP_BCK_LUT,
			bckLutDir + std::format("FLOP_{}_BCK_LUT.bin", nBckFlop));
		// Write clusters' centers.
		opt::saveArray(FLOP_CENTERS,
			bckLutDir + std::format("FLOP_{}_CENTERS.bin", nBckFlop));
		// Write inertia and min weight.
		auto file = std::fstream(
			bckLutDir + std::format("FLOP_{}_BCK - inertia={}, min_weight={}",
				nBckFlop, flopInertia, flopMinWeight),
			std::ios::out);
		file.close();
	}

	static void saveTurnBckLUT()
	{
		opt::saveArray(TURN_BCK_LUT,
			bckLutDir + std::format("TURN_{}_BCK_LUT.bin", nBckTurn));
		// Write clusters' centers.
		opt::saveArray(TURN_CENTERS,
			bckLutDir + std::format("TURN_{}_CENTERS.bin", nBckTurn));
		// Write inertia and min weight.
		auto file = std::fstream(
			bckLutDir + std::format("TURN_{}_BCK - inertia={}, min_weight={}",
				nBckTurn, turnInertia, turnMinWeight),
			std::ios::out);
		file.close();
	}

	static void loadPreflopBckLUT()
	{
		opt::loadArray(PREFLOP_BCK_LUT,
			bckLutDir + std::format("PREFLOP_{}_BCK_LUT.bin", nBckPreflop));
	}

	static void loadFlopBckLUT()
	{
		opt::loadArray(FLOP_BCK_LUT,
			bckLutDir + std::format("FLOP_{}_BCK_LUT.bin", nBckFlop));
	}

	static void loadTurnBckLUT()
	{
		opt::loadArray(TURN_BCK_LUT,
			bckLutDir + std::format("TURN_{}_BCK_LUT.bin", nBckTurn));
	}

	static std::array<bckSize_t, PREFLOP_SIZE> PREFLOP_BCK_LUT;
	static std::array<bckSize_t, FLOP_SIZE> FLOP_BCK_LUT;
	static std::array<bckSize_t, CMB_TURN_SIZE> TURN_BCK_LUT;

private:

	static std::array<std::array<uint16_t, N_BINS>, nBckPreflop> PREFLOP_CENTERS;
	static std::array<std::array<uint16_t, N_BINS>, nBckFlop> FLOP_CENTERS;
	static std::array<std::array<uint8_t, N_BINS>, nBckTurn> TURN_CENTERS;

	static uint64_t preflopInertia, flopInertia, turnInertia;
	static uint32_t preflopMinWeight, flopMinWeight, turnMinWeight;

	static EquityCalculator eqt;
	KMeans<bckSize_t, nBckPreflop> kmeansPreflop;
	KMeans<bckSize_t, nBckFlop> kmeansFlop;
	KMeans<bckSize_t, nBckTurn> kmeansTurn;

}; // DKEM

// Initialize static members.

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
std::array<bckSize_t, PREFLOP_SIZE> DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::PREFLOP_BCK_LUT;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
std::array<bckSize_t, FLOP_SIZE> DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::FLOP_BCK_LUT;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
std::array<bckSize_t, CMB_TURN_SIZE> DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::TURN_BCK_LUT;

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
std::array<std::array<uint16_t, N_BINS>, nBckPreflop> DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::PREFLOP_CENTERS;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
std::array<std::array<uint16_t, N_BINS>, nBckFlop> DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::FLOP_CENTERS;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
std::array<std::array<uint8_t, N_BINS>, nBckTurn> DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::TURN_CENTERS;

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
uint64_t DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::preflopInertia;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
uint64_t DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::flopInertia;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
uint64_t DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::turnInertia;

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
uint32_t DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::preflopMinWeight;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
uint32_t DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::flopMinWeight;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
uint32_t DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::turnMinWeight;

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn>
EquityCalculator DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn>::eqt;

} // abc

#endif // ABC_DKEM_H