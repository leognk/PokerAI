#ifndef ABC_DKEM_H
#define ABC_DKEM_H

#include <format>
#include "EquityCalculator.h"
#include "KMeans.h"
#include "../Utils/ioArray.h"

namespace abc {

// Class generating lossy information abstraction
// with distribution-aware k-means earth mover's distance.
// bck for bucket
template<typename bckSize_t, bckSize_t nBck>
class DKEM
{
public:

	// Set rngSeed to 0 to set a random seed.
	DKEM(uint16_t kMeansNRestarts, uint16_t kMeansMaxIter, unsigned kMeansRngSeed = 0) :
		kmeans(true, kMeansNRestarts, kMeansMaxIter, kMeansRngSeed)
	{}

	static void populatePreflopBckLUT()
	{
		eqt.loadPreflopHSHists();
		preflopInertia = kmeans.buildClusters(eqt.PREFLOP_HS_HISTS, PREFLOP_BCK_LUT);
	}

	static void populateFlopBckLUT()
	{
		eqt.loadFlopHSHists();
		flopInertia = kmeans.buildClusters(eqt.FLOP_HS_HISTS, FLOP_BCK_LUT);
	}

	static void populateTurnBckLUT()
	{
		eqt.loadTurnHSHists();
		turnInertia = kmeans.buildClusters(eqt.TURN_HS_HISTS, TURN_BCK_LUT);
	}

	static void savePreflopBckLUT()
	{
		saveArray(PREFLOP_BCK_LUT,
			dir + std::format("PREFLOP_BCK_LUT_{}.bin", nBck));
		// Write inertia.
		auto file = std::fstream(
			dir + std::format("PREFLOP_BCK_{} inertia = {}", nBck, preflopInertia),
			std::ios::out);
		file.close();
	}

	static void saveFlopBckLUT()
	{
		saveArray(FLOP_BCK_LUT,
			dir + std::format("FLOP_BCK_LUT_{}.bin", nBck));
		// Write inertia.
		auto file = std::fstream(
			dir + std::format("FLOP_BCK_{} inertia = {}", nBck, flopInertia),
			std::ios::out);
		file.close();
	}

	static void saveTurnBckLUT()
	{
		saveArray(TURN_BCK_LUT,
			dir + std::format("TURN_BCK_LUT_{}.bin", nBck));
		// Write inertia.
		auto file = std::fstream(
			dir + std::format("TURN_BCK_{} inertia = {}", nBck, turnInertia),
			std::ios::out);
		file.close();
	}

	static void loadPreflopBckLUT()
	{
		loadArray(PREFLOP_BCK_LUT,
			dir + std::format("PREFLOP_BCK_LUT_{}.bin", nBck));
	}

	static void loadFlopBckLUT()
	{
		loadArray(FLOP_BCK_LUT,
			dir + std::format("FLOP_BCK_LUT_{}.bin", nBck));
	}

	static void loadTurnBckLUT()
	{
		loadArray(TURN_BCK_LUT,
			dir + std::format("TURN_BCK_LUT_{}.bin", nBck));
	}

	static std::array<bckSize_t, PREFLOP_SIZE> PREFLOP_BCK_LUT;
	static std::array<bckSize_t, FLOP_SIZE> FLOP_BCK_LUT;
	static std::array<bckSize_t, CMB_TURN_SIZE> TURN_BCK_LUT;

private:

	static EquityCalculator eqt;
	KMeans<bckSize_t, nBck> kmeans;
	static uint64_t preflopInertia, flopInertia, turnInertia;

}; // DKEM

} // abc

#endif // ABC_DKEM_H