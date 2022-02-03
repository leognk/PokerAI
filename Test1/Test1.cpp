#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
//#include <random>
//#include <chrono>
//#include <filesystem>
//#include <fstream>
//#include <bitset>

#include "../AbstractInfoset/ActionSeqIndexer.h"
#include "../Blueprint/Constants.h"

int main()
{
	const uint8_t MAX_PLAYERS = 3;

	const egn::chips ANTE = 0;
	const egn::chips BIG_BLIND = 2;
	const egn::chips INITIAL_STAKE = 6;

	const std::vector<std::vector<std::vector<float>>> BET_SIZES = {
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		}
	};

	abc::TreeTraverser traverser(
		MAX_PLAYERS, ANTE, BIG_BLIND, INITIAL_STAKE, BET_SIZES, true, false);
	
	for (uint8_t i = 0; i < egn::N_ROUNDS; ++i) {

		std::vector<std::vector<uint8_t>> actionSeqs;
		traverser.traverseRoundTree(egn::Round(i), actionSeqs);

		std::cout << "Round: " << egn::Round(i) << "\n\n";
		for (size_t j = 0; j < actionSeqs.size(); ++j) {
			std::cout << std::setw(2) << j << ": ";
			for (size_t k = 0; k < actionSeqs[j].size(); ++k) {
				std::cout << std::to_string(actionSeqs[j][k]) << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}
}