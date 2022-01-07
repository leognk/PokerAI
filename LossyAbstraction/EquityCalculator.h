#ifndef ABC_EQUITYCALCULATOR_H
#define ABC_EQUITYCALCULATOR_H

#include <string>
#include <fstream>
#include "../LosslessAbstraction/hand_index.h"
#include "../OMPEval/omp/HandEvaluator.h"

namespace abc {

static const std::string dir = "../data/AbstractionSaves/";
static const std::string rivHSLUTPath = dir + "RIV_HS_LUT.bin";
static const std::string turnHSHistsPath = dir + "TURN_HS_HISTS.bin";
static const std::string flopHSHistsPath = dir + "FLOP_HS_HISTS.bin";
static const std::string preflopHSHistsPath = dir + "PREFLOP_HS_HISTS.bin";

// Number of bins of hand strength histograms.
static const uint16_t N_BINS = 50;

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

	static std::array<uint8_t, N_BINS> buildTurnHSHist(uint8_t hand[])
	{
		// Get hand's mask.
		uint64_t handMask = 0;
		for (uint8_t i = 0; i < omp::TURN_HAND; ++i)
			handMask |= 1ull << hand[i];

		// Loop over all river's cards,
		// skipping cards already used.
		std::array<uint8_t, N_BINS> hsHist{};
		for (uint8_t c = 0; c < omp::CARD_COUNT; ++c) {
			if (handMask & (1ull << c))
				continue;
			hand[omp::TURN_HAND] = c;
			hand_index_t handIdx = cmbRivIndexer.hand_index_last(hand);
#pragma warning(suppress: 4244 26451)
			uint16_t binIdx = (double)RIV_HS_LUT[handIdx] / (MAX_RIV_HS + 1) * N_BINS;
			++hsHist[binIdx];
		}
		return hsHist;
	}

	static std::array<uint16_t, N_BINS> buildFlopHSHist(uint8_t hand[])
	{
		// Get hand's mask.
		uint64_t handMask = 0;
		for (uint8_t i = 0; i < omp::FLOP_HAND; ++i)
			handMask |= 1ull << hand[i];

		// Loop over all turn's and river's cards,
		// skipping cards already used.
		std::array<uint16_t, N_BINS> hsHist{};
		for (uint8_t c1 = 1; c1 < omp::CARD_COUNT; ++c1) {
			if (handMask & (1ull << c1))
				continue;
			for (uint8_t c2 = 0; c2 < c1; ++c2) {
				if (handMask & (1ull << c2))
					continue;
				hand[omp::FLOP_HAND] = c1;
				hand[omp::TURN_HAND] = c2;
				hand_index_t handIdx = cmbRivIndexer.hand_index_last(hand);
#pragma warning(suppress: 4244 26451)
				uint16_t binIdx = (double)RIV_HS_LUT[handIdx] / (MAX_RIV_HS + 1) * N_BINS;
				++hsHist[binIdx];
			}
		}
		return hsHist;
	}

	static std::array<uint32_t, N_BINS> buildPreflopHSHist(uint8_t hand[])
	{
		// Loop over all flop's, turn's and river's cards,
		// skipping cards already used.
		std::array<uint32_t, N_BINS> hsHist{};
		for (uint8_t i = 0; i < omp::BOARD_CARDS; ++i)
			hand[omp::HOLE_CARDS + i] = i;
		uint8_t lastIdx = omp::RIVER_HAND - 1;
		while (true) {

			// Look for cards already used.
			bool skip = false;
			for (uint8_t i = omp::HOLE_CARDS; i < omp::RIVER_HAND; ++i) {
				if (hand[i] == hand[0] || hand[i] == hand[1]) {
					skip = true;
					continue;
				}
			}

			if (!skip) {
				hand_index_t handIdx = cmbRivIndexer.hand_index_last(hand);
#pragma warning(suppress: 4244 26451)
				uint16_t binIdx = (double)RIV_HS_LUT[handIdx] / (MAX_RIV_HS + 1) * N_BINS;
				++hsHist[binIdx];
			}

			// Go to the next combination of board cards.
			++hand[lastIdx];
			if (hand[lastIdx] == omp::CARD_COUNT) {
				uint8_t movingIdx = lastIdx - 1;
				while (hand[movingIdx] ==
					omp::CARD_COUNT - omp::RIVER_HAND + movingIdx) {
					if (movingIdx == omp::HOLE_CARDS) return hsHist;
					--movingIdx;
				}
				++hand[movingIdx];
				// Reset after movingIdx.
				for (uint8_t i = movingIdx + 1; i < omp::RIVER_HAND; ++i)
					hand[i] = hand[i - 1] + 1;
			}
		}
	}

	// Hand strength (equity) lookup table on river.
	static std::array<uint16_t, CMB_RIVER_SIZE> RIV_HS_LUT;
	// Hand strength histograms on each round.
	static std::array<std::array<uint8_t, N_BINS>, CMB_TURN_SIZE> TURN_HS_HISTS;
	static std::array<std::array<uint16_t, N_BINS>, FLOP_SIZE> FLOP_HS_HISTS;
	static std::array<std::array<uint32_t, N_BINS>, PREFLOP_SIZE> PREFLOP_HS_HISTS;

	static hand_indexer_t cmbRivIndexer;
	static hand_indexer_t cmbTurnIndexer;
	static hand_indexer_t flopIndexer;
	static hand_indexer_t preflopIndexer;

private:
	static uint16_t calculateRivHS(const uint8_t hand[]);

	template <typename A>
	static void saveArray(const A& arr, const std::string& path)
	{
		auto file = std::fstream(path, std::ios::out | std::ios::binary);
		file.write((char*)&arr[0], sizeof(arr));
		file.close();
	}

	template <typename A>
	static void loadArray(A& arr, const std::string& path)
	{
		auto file = std::fstream(path, std::ios::in | std::ios::binary);
		file.read((char*)&arr[0], sizeof(arr));
		file.close();
	}

	static omp::HandEvaluator eval;
	static uint64_t evalCount;

}; // EquityCalculator

} // abc

#endif ABC_EQUITYCALCULATOR_H