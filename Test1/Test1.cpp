#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <random>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <bitset>

#include "../LossyAbstraction/LossyIndexer.h"

int main()
{
	static const uint8_t nBckPreflop = 50;

	static const std::string dir = "../data/AbstractionSaves/BCK_STRENGTHS/";
	static const std::string preflopFilePath = dir + std::format("PREFLOP_{}_BCK_STRENGTHS.bin", nBckPreflop);

	typedef abc::LossyIndexer<uint8_t, nBckPreflop, 50, 50, 50> indexer_t;
	indexer_t::loadLUT();

	abc::EquityCalculator eqt;
	eqt.loadPreflopHSLUT();
	//std::sort(eqt.PREFLOP_HS_LUT.begin(), eqt.PREFLOP_HS_LUT.end());
	for (uint8_t i = 0; i < abc::PREFLOP_SIZE; ++i) {
		const double equity = (double)eqt.PREFLOP_HS_LUT[i] / abc::MAX_HS;
		std::cout << std::to_string(i) << ": " << equity << " | " << std::to_string(indexer_t::dkem.PREFLOP_BCK_LUT[i]) << "\n";
	}
}