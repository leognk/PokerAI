#include "../cpptqdm/tqdm.h"
#include "OCHSCalculator.h"

namespace abc {

std::vector<std::vector<uint16_t>> OCHSCalculator::RIV_OCHS_LUT;
DKEM<uint8_t, OCHS_SIZE> OCHSCalculator::dkem;
omp::HandEvaluator OCHSCalculator::eval;

void OCHSCalculator::populateRivOCHSLUT()
{
	RIV_OCHS_LUT.resize(CMB_RIVER_SIZE, std::vector<uint16_t>(OCHS_SIZE));
	dkem.loadPreflopBckLUT();
	uint8_t hand[omp::RIVER_HAND];
	tqdm bar;
	for (hand_index_t i = 0; i < CMB_RIVER_SIZE; ++i) {
		bar.progress(i, CMB_RIVER_SIZE);
		EquityCalculator::cmbRivIndexer.hand_unindex(1, i, hand);
		calculateRivOCHS(hand, RIV_OCHS_LUT[i]);
	}
}

void OCHSCalculator::saveRivOCHSLUT()
{
	opt::save2DVector(RIV_OCHS_LUT, rivOCHSLUTPath);
}

void OCHSCalculator::loadRivOCHSLUT()
{
	RIV_OCHS_LUT.resize(CMB_RIVER_SIZE, std::vector<uint16_t>(OCHS_SIZE));
	opt::load2DVector(RIV_OCHS_LUT, rivOCHSLUTPath);
}

void OCHSCalculator::calculateRivOCHS(const uint8_t hand[], std::vector<uint16_t>& ochs)
{
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
	// skipping cards already used,
	// and update the corresponding feature in ochs.
	std::array<uint16_t, OCHS_SIZE> weightInClusters{};
	for (uint8_t c1 = 1; c1 < omp::CARD_COUNT; ++c1) {
		if (handMask & (1ull << c1))
			continue;
		for (uint8_t c2 = 0; c2 < c1; ++c2) {
			if (handMask & (1ull << c2))
				continue;
			// Compute opponent's hand's rank.
			uint16_t oppRank = eval.evaluate(
				omp::Hand(c1) + omp::Hand(c2) + boardCards);
			uint8_t oppHand[omp::HOLE_CARDS] = { c1, c2 };
			hand_index_t oppHandBck = dkem.PREFLOP_BCK_LUT[
				EquityCalculator::preflopIndexer.hand_index_last(oppHand)];
			weightInClusters[oppHandBck] += 2;
			if (handRank > oppRank)
				ochs[oppHandBck] += 2;
			else if (handRank == oppRank)
				++ochs[oppHandBck];
		}
	}

	// Normalize OCHS.
	for (uint8_t k = 0; k < OCHS_SIZE; ++k)
#pragma warning(suppress: 4244)
		ochs[k] = std::round((double)MAX_OCHS_VALUE * ochs[k] / weightInClusters[k]);
}

} // abc