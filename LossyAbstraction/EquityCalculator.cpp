#include "../cpptqdm/tqdm.h"
#include "EquityCalculator.h"
#include "../OMPEval/omp/Constants.h"

namespace abc {

std::array<uint16_t, CMB_RIVER_SIZE> EquityCalculator::RIV_HS_LUT;
std::array<std::array<uint8_t, N_BINS>, CMB_TURN_SIZE> EquityCalculator::TURN_HS_HISTS;
std::array<std::array<uint16_t, N_BINS>, FLOP_SIZE> EquityCalculator::FLOP_HS_HISTS;
std::array<std::array<uint32_t, N_BINS>, PREFLOP_SIZE> EquityCalculator::PREFLOP_HS_HISTS;

hand_indexer_t EquityCalculator::cmbRivIndexer(
	2, { omp::HOLE_CARDS, omp::BOARD_CARDS });
hand_indexer_t EquityCalculator::cmbTurnIndexer(
	2, { omp::HOLE_CARDS, omp::FLOP_CARDS + omp::TURN_CARDS });
hand_indexer_t EquityCalculator::flopIndexer(
	2, { omp::HOLE_CARDS, omp::FLOP_CARDS });
hand_indexer_t EquityCalculator::preflopIndexer(
	1, { omp::HOLE_CARDS });

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

void EquityCalculator::populateTurnHSHists()
{
	uint8_t hand[omp::RIVER_HAND];
	tqdm bar;
	for (hand_index_t i = 0; i < CMB_TURN_SIZE; ++i) {
		bar.progress(i, CMB_TURN_SIZE);
		cmbTurnIndexer.hand_unindex(1, i, hand);
		TURN_HS_HISTS[i] = buildTurnHSHist(hand);
	}
}

void EquityCalculator::populateFlopHSHists()
{
	uint8_t hand[omp::RIVER_HAND];
	tqdm bar;
	for (hand_index_t i = 0; i < FLOP_SIZE; ++i) {
		bar.progress(i, FLOP_SIZE);
		flopIndexer.hand_unindex(1, i, hand);
		FLOP_HS_HISTS[i] = buildFlopHSHist(hand);
	}
}

void EquityCalculator::populatePreflopHSHists()
{
	uint8_t hand[omp::RIVER_HAND];
	tqdm bar;
	for (hand_index_t i = 0; i < PREFLOP_SIZE; ++i) {
		bar.progress(i, PREFLOP_SIZE);
		preflopIndexer.hand_unindex(0, i, hand);
		PREFLOP_HS_HISTS[i] = buildPreflopHSHist(hand);
	}
}

void EquityCalculator::saveRivHSLUT()
{
	saveArray(RIV_HS_LUT, rivHSLUTPath);
}

void EquityCalculator::saveTurnHSHists()
{
	saveArray(TURN_HS_HISTS, turnHSHistsPath);
}

void EquityCalculator::saveFlopHSHists()
{
	saveArray(FLOP_HS_HISTS, flopHSHistsPath);
}

void EquityCalculator::savePreflopHSHists()
{
	saveArray(PREFLOP_HS_HISTS, preflopHSHistsPath);
}

void EquityCalculator::loadRivHSLUT()
{
	loadArray(RIV_HS_LUT, rivHSLUTPath);
}

void EquityCalculator::loadTurnHSHists()
{
	loadArray(TURN_HS_HISTS, turnHSHistsPath);
}

void EquityCalculator::loadFlopHSHists()
{
	loadArray(FLOP_HS_HISTS, flopHSHistsPath);
}

void EquityCalculator::loadPreflopHSHists()
{
	loadArray(PREFLOP_HS_HISTS, preflopHSHistsPath);
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