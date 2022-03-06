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
static const std::string turnHSLUTPath = hsLutDir + "TURN_HS_LUT.bin";
static const std::string flopHSLUTPath = hsLutDir + "FLOP_HS_LUT.bin";
static const std::string preflopHSLUTPath = hsLutDir + "PREFLOP_HS_LUT.bin";

static const std::string turnHSHistsPath = hsLutDir + "TURN_HS_HISTS.bin";
static const std::string flopHSHistsPath = hsLutDir + "FLOP_HS_HISTS.bin";
static const std::string preflopHSHistsPath = hsLutDir + "PREFLOP_HS_HISTS.bin";

// Number of bins of hand strength histograms.
static const uint16_t N_BINS = 50;

// Maximum sum of the heights of a histogram allowed (reached on the preflop's histograms).
static const uint16_t MAX_TOTAL_WEIGHT = 1100;

// Number of opponent's hand combinations on the preflop.
static const uint64_t PREFLOP_N_COMB = 2118760; // binom(52 - 2, 5)
// Number of opponent's hand combinations on the flop.
static const uint16_t FLOP_N_COMB = 1081; // binom(52 - 5, 2) = 47 * 23
// Number of opponent's hand combinations on the turn.
static const uint16_t TURN_N_COMB = 46; // 52 - 6

// Maximum equity score, which is 2 times the number of
// possibilities of opponent's hole cards: 2 * binom(52 - 7, 2) = 1980
// win: +2 ; draw: +1 ; defeat: +0
static const uint16_t MAX_HS = 1980;

// Class for computing a hand's equity
// or equity histogram when facing one opponent.
class EquityCalculator
{
public:
	static void populateRivHSLUT();
	static void populateTurnHSLUT();
	static void populateFlopHSLUT();
	static void populatePreflopHSLUT();

	static void populateTurnHSHists();
	static void populateFlopHSHists();
	static void populatePreflopHSHists();

	static void saveRivHSLUT();
	static void saveTurnHSLUT();
	static void saveFlopHSLUT();
	static void savePreflopHSLUT();

	static void saveTurnHSHists();
	static void saveFlopHSHists();
	static void savePreflopHSHists();

	static void loadRivHSLUT();
	static void loadTurnHSLUT();
	static void loadFlopHSLUT();
	static void loadPreflopHSLUT();

	static void loadTurnHSHists();
	static void loadFlopHSHists();
	static void loadPreflopHSHists();

	static uint16_t calculateRivHS(const uint8_t hand[]);
	static uint16_t calculateTurnHS(uint8_t hand[]);
	static uint16_t calculateFlopHS(uint8_t hand[]);
	static uint16_t calculatePreflopHS(uint8_t hand[]);

	static std::array<uint8_t, N_BINS> buildTurnHSHist(uint8_t hand[]);
	static std::array<uint16_t, N_BINS> buildFlopHSHist(uint8_t hand[]);
	static std::array<uint16_t, N_BINS> buildPreflopHSHist(uint8_t hand[]);

	// Hand strength (equity) lookup table on each round.
	static std::array<uint16_t, CMB_RIVER_SIZE> RIV_HS_LUT;
	static std::array<uint16_t, CMB_TURN_SIZE> TURN_HS_LUT;
	static std::array<uint16_t, FLOP_SIZE> FLOP_HS_LUT;
	static std::array<uint16_t, PREFLOP_SIZE> PREFLOP_HS_LUT;

	// Hand strength histograms on each round.
	static std::array<std::array<uint8_t, N_BINS>, CMB_TURN_SIZE> TURN_HS_HISTS;
	static std::array<std::array<uint16_t, N_BINS>, FLOP_SIZE> FLOP_HS_HISTS;
	static std::array<std::array<uint16_t, N_BINS>, PREFLOP_SIZE> PREFLOP_HS_HISTS;

	static hand_indexer_t cmbRivIndexer;
	static hand_indexer_t cmbTurnIndexer;
	static hand_indexer_t flopIndexer;
	static hand_indexer_t preflopIndexer;

private:
	static std::array<uint16_t, N_BINS> normalizeHist(
		std::array<uint32_t, N_BINS>& accHsHist);

	static omp::HandEvaluator eval;
	static uint64_t evalCount;

}; // EquityCalculator

} // abc

#endif ABC_EQUITYCALCULATOR_H