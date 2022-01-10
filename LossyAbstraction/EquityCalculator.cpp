#include "../cpptqdm/tqdm.h"
#include "EquityCalculator.h"
#include "../OMPEval/omp/Constants.h"
#include "../Utils/ioArray.h"

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
	opt::saveArray(RIV_HS_LUT, rivHSLUTPath);
}

void EquityCalculator::saveTurnHSHists()
{
	opt::saveArray(TURN_HS_HISTS, turnHSHistsPath);
}

void EquityCalculator::saveFlopHSHists()
{
	opt::saveArray(FLOP_HS_HISTS, flopHSHistsPath);
}

void EquityCalculator::savePreflopHSHists()
{
	opt::saveArray(PREFLOP_HS_HISTS, preflopHSHistsPath);
}

void EquityCalculator::loadRivHSLUT()
{
	opt::loadArray(RIV_HS_LUT, rivHSLUTPath);
}

void EquityCalculator::loadTurnHSHists()
{
	opt::loadArray(TURN_HS_HISTS, turnHSHistsPath);
}

void EquityCalculator::loadFlopHSHists()
{
	opt::loadArray(FLOP_HS_HISTS, flopHSHistsPath);
}

void EquityCalculator::loadPreflopHSHists()
{
	opt::loadArray(PREFLOP_HS_HISTS, preflopHSHistsPath);
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

std::array<uint8_t, N_BINS> EquityCalculator::buildTurnHSHist(uint8_t hand[])
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

std::array<uint16_t, N_BINS> EquityCalculator::buildFlopHSHist(uint8_t hand[])
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

std::array<uint32_t, N_BINS> EquityCalculator::buildPreflopHSHist(uint8_t hand[])
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

} // abc