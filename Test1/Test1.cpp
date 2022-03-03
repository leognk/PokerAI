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
#include "../AbstractInfoset/GroupedActionSeqs.h"

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

	abc::GroupedActionSeqs gpSeqs(bp::BLUEPRINT_GAME_NAME);
	gpSeqs.load();

	blueprint.loadStrat();
	blueprint.loadRegrets();

	const uint8_t round = 0;
	const unsigned handIdx = 0;

	abc::GroupedActionSeqs::seqIdx_t currSeq = 0;
	for (const uint8_t nLegalActions : gpSeqs.lens[round]) {
		for (uint8_t a = 0; a < nLegalActions; ++a) {
			auto seqIdx = gpSeqs.seqs[round][currSeq];
			const auto s = blueprint.strat[round][handIdx][seqIdx];
			const auto r = blueprint.regrets[round][handIdx][seqIdx];
			std::cout
				<< std::setw(4) << currSeq
				<< " | " << std::setw(5) << s
				<< " | " << std::setw(4) << opt::prettyPerc(s, uint16_t(1u << 15))
				<< " | " << r
				<< " | " << seqIdx << "\n";
			++currSeq;
		}
		std::cout << "\n";
	}
}