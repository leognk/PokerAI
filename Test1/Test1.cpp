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

#include "../Blueprint/Blueprint.h"

int main()
{
	const unsigned rngSeed = 1;

	bp::Blueprint blueprint(bp::BLUEPRINT_GAME_NAME, bp::BLUEPRINT_BUILD_NAME, rngSeed);
	blueprint.loadStrat();

	bp::abcInfo_t abcInfo(
		bp::MAX_PLAYERS,
		bp::ANTE,
		bp::BIG_BLIND,
		bp::INITIAL_STAKE,
		bp::BET_SIZES,
		bp::BLUEPRINT_GAME_NAME,
		rngSeed);

	abcInfo.startNewHand();

	blueprint.loadRegrets();
	for (size_t i = 0; i < blueprint.regrets[0][0].size(); ++i) {
		const int r = blueprint.regrets[0][0][i];
		if (r > 0) {
			const int diff = (std::numeric_limits<bp::regret_t>::max)() - r;
			if (diff < 10000)
				std::cout << i << ": " << diff << " " << r << "\n";
		}
	}
}