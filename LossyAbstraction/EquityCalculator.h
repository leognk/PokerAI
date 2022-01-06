#ifndef ABC_EQUITYCALCULATOR_H
#define ABC_EQUITYCALCULATOR_H

#include <string>
#include "../LosslessAbstraction/hand_index.h"
#include "../OMPEval/omp/HandEvaluator.h"

namespace abc {

static const std::string dir = "../data/AbstractionSaves/";
static const std::string rivHSLUTPath = dir + "RIV_HS_LUT.bin";

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
	static void saveRivHSLUT();
	static void loadRivHSLUT();

	template<uint16_t nBins>
	static std::array<uint8_t, nBins> buildTurnHSHist(const uint8_t hand[])
	{
		// Get hand's mask.
		uint64_t handMask = 0;
		for (uint8_t i = 0; i < omp::TURN_HAND; ++i)
			handMask |= 1ull << hand[i];

		// Loop over all river's cards,
		// skipping cards already used.
		std::array<uint8_t, nBins> hsHist{};
		uint8_t cards[omp::RIVER_HAND];
		std::copy(hand, hand + omp::TURN_HAND, cards);
		for (uint8_t c = 0; c < omp::CARD_COUNT; ++c) {
			if (handMask & (1ull << c))
				continue;
			cards[omp::TURN_HAND] = c;
			hand_index_t handIdx = cmbRivIndexer.hand_index_last(cards);
#pragma warning(suppress: 4244 26451)
			uint16_t binIdx = (double)RIV_HS_LUT[handIdx] / (MAX_RIV_HS + 1) * nBins;
			++hsHist[binIdx];
		}
		return hsHist;
	}

	template<uint16_t nBins>
	static std::array<uint16_t, nBins> buildFlopHSHist(const uint8_t hand[])
	{
		// Get hand's mask.
		uint64_t handMask = 0;
		for (uint8_t i = 0; i < omp::FLOP_HAND; ++i)
			handMask |= 1ull << hand[i];

		// Loop over all turn's and river's cards,
		// skipping cards already used.
		std::array<uint16_t, nBins> hsHist{};
		uint8_t cards[omp::RIVER_HAND];
		std::copy(hand, hand + omp::FLOP_HAND, cards);
		for (uint8_t c1 = 1; c1 < omp::CARD_COUNT; ++c1) {
			if (handMask & (1ull << c1))
				continue;
			for (uint8_t c2 = 0; c2 < c1; ++c2) {
				if (handMask & (1ull << c2))
					continue;
				cards[omp::FLOP_HAND] = c1;
				cards[omp::TURN_HAND] = c2;
				hand_index_t handIdx = cmbRivIndexer.hand_index_last(cards);
#pragma warning(suppress: 4244 26451)
				uint16_t binIdx = (double)RIV_HS_LUT[handIdx] / (MAX_RIV_HS + 1) * nBins;
				++hsHist[binIdx];
			}
		}
		return hsHist;
	}

	template<uint16_t nBins>
	static std::array<uint32_t, nBins> buildPreflopHSHist(const uint8_t hand[])
	{
		std::array<uint32_t, nBins> hsHist{};
		uint8_t cards[omp::RIVER_HAND];
		std::copy(hand, hand + omp::HOLE_CARDS, cards);

		// Loop over all flop's, turn's and river's cards,
		// skipping cards already used.

		std::array<uint8_t, omp::BOARD_CARDS> boardCards = { 0, 1, 2, 3, 4 };
		uint8_t lastIdx = omp::BOARD_CARDS - 1;

		while (true) {

			// Look for cards already used.
			bool skip = false;
			for (uint8_t c : boardCards) {
				if (c == cards[0] || c == cards[1]) {
					skip = true;
					continue;
				}
			}

			if (!skip) {
				for (uint8_t i = 0; i < omp::BOARD_CARDS; ++i)
					cards[omp::HOLE_CARDS + i] = boardCards[i];
				hand_index_t handIdx = cmbRivIndexer.hand_index_last(cards);
#pragma warning(suppress: 4244 26451)
				uint16_t binIdx = (double)RIV_HS_LUT[handIdx] / (MAX_RIV_HS + 1) * nBins;
				++hsHist[binIdx];
			}

			// Go to the next combination of board cards.
			++boardCards[lastIdx];
			if (boardCards[lastIdx] == omp::CARD_COUNT) {
				uint8_t movingIdx = lastIdx - 1;
				while (boardCards[movingIdx] ==
					omp::CARD_COUNT - omp::BOARD_CARDS + movingIdx) {
					if (!movingIdx) return hsHist;
					--movingIdx;
				}
				++boardCards[movingIdx];
				// Reset after movingIdx.
				for (uint8_t i = movingIdx + 1; i < omp::BOARD_CARDS; ++i)
					boardCards[i] = boardCards[i - 1] + 1;
			}
		}
	}

	// Hand strength (equity) lookup table on river.
	static std::array<uint16_t, CMB_RIVER_SIZE> RIV_HS_LUT;
	static hand_indexer_t cmbRivIndexer;

private:
	static uint16_t calculateRivHS(const uint8_t hand[]);

	static omp::HandEvaluator eval;
	static uint64_t evalCount;

}; // EquityCalculator

} // abc

#endif ABC_EQUITYCALCULATOR_H