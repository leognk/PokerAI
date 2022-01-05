#include <fstream>
#include "../cpptqdm/tqdm.h"
#include "EquityCalculator.h"
#include "../OMPEval/omp/Constants.h"

namespace abc {

std::array<uint16_t, CMB_RIVER_SIZE> EquityCalculator::RIV_HS_LUT;
hand_indexer_t EquityCalculator::cmbRivIndexer(
	2, { omp::HOLE_CARDS, omp::BOARD_CARDS });

omp::HandEvaluator EquityCalculator::eval;
uint64_t EquityCalculator::evalCount;

void EquityCalculator::populateRivHSLUT()
{
	evalCount = 0;
	uint8_t hand[omp::RIVER_HAND];
	tqdm bar;
	for (hand_index_t i = 0; i < CMB_RIVER_SIZE; ++i) {
		bar.progress(i, CMB_RIVER_SIZE);
		cmbRivIndexer.hand_unindex(1, i, hand);
		RIV_HS_LUT[i] = calculateRivHS(hand);
	}
	if (evalCount != (uint64_t)MAX_RIV_HS / 2 * CMB_RIVER_SIZE)
		throw std::runtime_error("Incorrect number of evaluations done.");
}

void EquityCalculator::saveRivHSLUT()
{
	auto file = std::fstream(rivHSLUTPath, std::ios::out | std::ios::binary);
	file.write((char*)&RIV_HS_LUT[0], RIV_HS_LUT.size() * sizeof(uint16_t));
	file.close();
}

void EquityCalculator::loadRivHSLUT()
{
	auto file = std::fstream(rivHSLUTPath, std::ios::in | std::ios::binary);
	file.read((char*)&RIV_HS_LUT[0], RIV_HS_LUT.size() * sizeof(uint16_t));
	file.close();
}

uint16_t EquityCalculator::calculateRivHS(const uint8_t hand[])
{
	uint16_t equity = 0;

	// Get hand's mask.
	uint64_t handMask = 0;
	for (uint8_t i = 0; i < omp::RIVER_HAND; ++i)
		handMask |= 1ull << hand[i];

	// Compute hand's rank.
	omp::Hand boardCards = omp::Hand::empty();
	for (uint8_t i = omp::HOLE_CARDS; i < omp::RIVER_HAND; ++i)
		boardCards += omp::Hand(hand[i]);
	uint16_t handRank = eval.evaluate(
		omp::Hand(hand[0]) + omp::Hand(hand[1]) + boardCards);

	// Loop over all opponent's hole cards,
	// skipping cards already used.
	for (uint8_t c1 = 1; c1 < omp::CARD_COUNT; ++c1) {
		if (handMask & (1ull << c1))
			continue;
		for (uint8_t c2 = 0; c2 < c1; ++c2) {
			if (handMask & (1ull << c2))
				continue;
			// Compute opponent's hand's rank.
			uint16_t oppRank = eval.evaluate(
				omp::Hand(c1) + omp::Hand(c2) + boardCards);
			if (handRank > oppRank)
				equity += 2;
			else if (handRank == oppRank)
				++equity;
			++evalCount;
		}
	}
	return equity;
}

} // abc