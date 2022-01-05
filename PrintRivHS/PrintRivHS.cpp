#include <iostream>
#include <iomanip>
#include "../LossyAbstraction/EquityCalculator.h"
#include "../GameEngine/Hand.h"

int main()
{
	static const abc::hand_index_t offset = 0;
	static const abc::hand_index_t nHands = 10;

	abc::EquityCalculator eqt;
	eqt.loadRivHSLUT();
	uint8_t cards[omp::RIVER_HAND];
	std::cout << std::fixed << std::setprecision(2);
	for (abc::hand_index_t i = offset; i < offset + nHands; ++i) {
		eqt.cmbRivIndexer.hand_unindex(1, i, cards);
		egn::Hand holeCards(cards, omp::HOLE_CARDS);
		egn::Hand boardCards(cards + omp::HOLE_CARDS, omp::BOARD_CARDS);
		std::cout << std::setw(3) << i << " - "
			<< holeCards << " | " << boardCards << " - "
			<< std::setw(6) << (double)eqt.RIV_HS_LUT[i]
			/ abc::MAX_RIV_HS * 100 << " % - "
			<< std::setw(4) << eqt.RIV_HS_LUT[i]
			<< "/" << abc::MAX_RIV_HS << "\n";
	}
}