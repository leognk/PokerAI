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

#include "../AbstractInfoset/TreeTraverser.h"
#include "../Blueprint/Constants.h"
#include "../Utils/StringManip.h"

int main()
{
	const uint8_t MAX_PLAYERS = 3;

	const egn::chips ANTE = 0;
	const egn::chips BIG_BLIND = 2;
	const egn::chips INITIAL_STAKE = 6;

	const abc::betSizes_t BET_SIZES = {
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

	abc::TreeTraverser traverser(MAX_PLAYERS, ANTE, BIG_BLIND, INITIAL_STAKE, BET_SIZES, false);
	std::vector<std::vector<abc::StdActionSeq>> actionSeqs = traverser.traverseTree();

	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {

		std::cout << opt::toUpper((std::ostringstream() << egn::Round(r)).str()) << "\n\n";

		for (size_t i = 0; i < actionSeqs[r].size(); ++i) {

			std::cout << std::setw(2) << i + 1 << "/" << actionSeqs[r].size() << ": ";
			abc::StdActionSeqIterator iter(actionSeqs[r][i]);
			while (!iter.end())
				std::cout << std::setw(2) << std::to_string(iter.next()) << " ";
			std::cout << "\n\n";

		}
		std::cout << "\n";
	}
}