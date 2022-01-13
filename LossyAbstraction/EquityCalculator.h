#ifndef ABC_EQUITYCALCULATOR_H
#define ABC_EQUITYCALCULATOR_H

#include <string>
#include <cmath>
#include <numeric>
#include "../LosslessAbstraction/hand_index.h"
#include "../OMPEval/omp/HandEvaluator.h"

namespace abc {

static const std::string hsLutDir = "../data/AbstractionSaves/HS_LUT/";
static const std::string rivHSLUTPath = hsLutDir + "RIV_HS_LUT.bin";
static const std::string turnHSHistsPath = hsLutDir + "TURN_HS_HISTS.bin";
static const std::string flopHSHistsPath = hsLutDir + "FLOP_HS_HISTS.bin";
static const std::string preflopHSHistsPath = hsLutDir + "PREFLOP_HS_HISTS.bin";

// Number of bins of hand strength histograms.
static const uint16_t N_BINS = 50;
// Maximum sum of the heights of a histogram allowed.
static const uint8_t MAX_TOTAL_WEIGHT = 100;

// Maximum equity score, which is 2 times the number of
// possibilities of opponent's hole cards: 2 * binom(52-7, 2) = 1980
// win: +2 ; draw: +1 ; defeat: +0
static const uint16_t MAX_RIV_HS = 1980;

// Class for computing a hand's equity
// or equity histogram when facing one opponent.
class EquityCalculator
{
public:
	static void populateRivHSLUT();
	static void populateTurnHSHists();
	static void populateFlopHSHists();
	static void populatePreflopHSHists();

	static void saveRivHSLUT();
	static void saveTurnHSHists();
	static void saveFlopHSHists();
	static void savePreflopHSHists();

	static void loadRivHSLUT();
	static void loadTurnHSHists();
	static void loadFlopHSHists();
	static void loadPreflopHSHists();

	static uint16_t calculateRivHS(const uint8_t hand[]);
	static std::array<uint8_t, N_BINS> buildTurnHSHist(uint8_t hand[]);
	static std::array<uint8_t, N_BINS> buildFlopHSHist(uint8_t hand[]);
	static std::array<uint8_t, N_BINS> buildPreflopHSHist(uint8_t hand[]);

	// Hand strength (equity) lookup table on river.
	static std::array<uint16_t, CMB_RIVER_SIZE> RIV_HS_LUT;
	// Hand strength histograms on each round.
	static std::array<std::array<uint8_t, N_BINS>, CMB_TURN_SIZE> TURN_HS_HISTS;
	static std::array<std::array<uint8_t, N_BINS>, FLOP_SIZE> FLOP_HS_HISTS;
	static std::array<std::array<uint8_t, N_BINS>, PREFLOP_SIZE> PREFLOP_HS_HISTS;

	static hand_indexer_t cmbRivIndexer;
	static hand_indexer_t cmbTurnIndexer;
	static hand_indexer_t flopIndexer;
	static hand_indexer_t preflopIndexer;

private:
	template<typename C>
	static std::array<uint8_t, N_BINS> normalizeHist(C& accHsHist)
	{
		// We apply the following weird procedure so that the total weight of
		// the histogram will be exactly equal to MAX_TOTAL_WEIGHT, getting around
		// rounding issues.
		for (uint16_t i = 1; i < N_BINS; ++i)
			accHsHist[i] += accHsHist[i - 1];
		for (uint16_t i = 0; i < N_BINS; ++i)
#pragma warning(suppress: 4244)
			accHsHist[i] = std::round((double)MAX_TOTAL_WEIGHT * accHsHist[i] / accHsHist.back());
		std::array<uint8_t, N_BINS> hsHist;
		std::adjacent_difference(accHsHist.begin(), accHsHist.end(), hsHist.begin());
		return hsHist;
	}

	static omp::HandEvaluator eval;
	static uint64_t evalCount;

}; // EquityCalculator

} // abc

#endif ABC_EQUITYCALCULATOR_H