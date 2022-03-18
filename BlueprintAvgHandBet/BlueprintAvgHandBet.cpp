#include "../Blueprint/Blueprint.h"

int main()
{
	const double endDuration = 10;
	const unsigned rngSeed = 1;
	const std::string dir = opt::dataDir + "Blueprint/Tests/AvgHandBet/";

	// Create folder.
	std::filesystem::create_directory(dir);

	std::vector<std::vector<double>> avgHandBet = {
		std::vector<double>(bp::N_BCK_PREFLOP),
		std::vector<double>(bp::N_BCK_FLOP),
		std::vector<double>(bp::N_BCK_TURN),
		std::vector<double>(bp::N_BCK_RIVER)
	};

	std::vector<std::vector<uint64_t>> updatesCount = {
		std::vector<uint64_t>(bp::N_BCK_PREFLOP),
		std::vector<uint64_t>(bp::N_BCK_FLOP),
		std::vector<uint64_t>(bp::N_BCK_TURN),
		std::vector<uint64_t>(bp::N_BCK_RIVER)
	};

	bp::Blueprint blueprint(bp::BLUEPRINT_GAME_NAME, bp::BLUEPRINT_BUILD_NAME);
	blueprint.loadStrat();

	bp::abcInfo_t abcInfo(
		bp::MAX_PLAYERS,
		bp::ANTE,
		bp::BIG_BLIND,
		bp::INITIAL_STAKE,
		bp::BET_SIZES,
		bp::BLUEPRINT_GAME_NAME,
		rngSeed);

	// Simulate random games with the blueprint strategy and
	// accumulate the hands bets.
	const opt::time_t startTime = opt::getTime();
	double currDuration = 0;
	uint64_t gameCount = 0;
	while (currDuration < endDuration) {
		abcInfo.resetStakes();
		abcInfo.startNewHand(true);
		while (!abcInfo.state.finished) {
			const auto round = abcInfo.roundIdx();
			const auto handIdx = abcInfo.handIdx();
			const uint8_t a = blueprint.chooseAction(abcInfo);
			const egn::chips bet = abcInfo.nextStateWithBet(a, true);
			avgHandBet[round][handIdx] += (double)bet;
			++updatesCount[round][handIdx];
		}
		if (++gameCount % uint64_t(10e3) == 0)
			currDuration = opt::getDuration(startTime);
	}

	// Normalize the sums in avgHandBet.
	for (uint8_t r = 0; r < avgHandBet.size(); ++r) {
		for (size_t i = 0; i < avgHandBet[r].size(); ++i)
			avgHandBet[r][i] /= updatesCount[r][i];
	}

	// Save avgHandBet.
	opt::save2DVector(avgHandBet, dir + bp::blueprintName() + ".bin");

	// Print stats.
	std::cout
		<< opt::prettyNumDg(currDuration, 3, true) << "s\n"
		<< opt::prettyNumDg(gameCount, 3, true) << "games\n"
		<< opt::prettyNumDg(gameCount / currDuration, 3, true) << "games/s\n"
		<< opt::prettyNumDg(currDuration / gameCount, 3, true) << "s/game\n";
}