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
	uint8_t a = blueprint.chooseAction(abcInfo);
	std::cout << std::to_string(a) << "\n";
}