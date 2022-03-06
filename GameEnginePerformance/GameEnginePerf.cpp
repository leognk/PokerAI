#include "../GameEngine/PlayGame.h"
#include "../RandomAI/RandomAI.h"
#include "../Utils/Time.h"
#include "../Utils/StringManip.h"

int main()
{
    const double endDuration = 1;
    const unsigned rngSeed = 1;
    const egn::chips ante = 0;
    const egn::chips bigBlind = 10;
    const egn::chips stake = 1000;
    const double foldProba = 1.0 / 3;
    const double callProba = 1.0 / 3;

    std::array<egn::chips, egn::MAX_PLAYERS> stakes{
        stake, stake, stake, stake, stake, stake };

    opt::RandomAI randomAI(foldProba, callProba, rngSeed);

    std::array<egn::Player*, egn::MAX_PLAYERS> players{
        &randomAI, &randomAI, &randomAI, &randomAI, &randomAI, &randomAI };

    egn::PlayGame play(ante, bigBlind, stakes, players, 0, rngSeed);

    // Simulate games.
    const opt::time_t startTime = opt::getTime();
    double currDuration = 0;
    uint64_t gameCount = 0;
    while (currDuration < endDuration) {
        play.playAndReset();
        if (++gameCount % uint64_t(1e3) == 0)
            currDuration = opt::getDuration(startTime);
    }

	// Print stats.
    std::cout
        << opt::prettyNumDg(currDuration, 3, true) << "s\n"
        << opt::prettyNumDg(gameCount, 3, true) << "games\n"
        << opt::prettyNumDg(gameCount / currDuration, 3, true) << "games/s\n"
        << opt::prettyNumDg(currDuration / gameCount, 3, true) << "s/game\n";
}