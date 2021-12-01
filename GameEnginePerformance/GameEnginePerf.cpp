#include "../GameEngine/PlayGame.h"
#include "../RandomAI/RandomAI.h"
#include <iostream>
#include <chrono>

int main()
{
    // Constants
    const double maxDuration = 5;
    const unsigned rngSeed = 1;
    const uint32_t ante = 1, bigBlind = 10;
    const uint32_t stake = 1000;
    const double foldProba = 1. / 3, callProba = 1. / 3;


    // Define game variables.
    std::array<uint32_t, opt::MAX_PLAYERS> stakes{
        stake, stake, stake, stake, stake, stake };
    opt::RandomAI randomAI(foldProba, callProba, rngSeed);
    std::array<egn::Player, opt::MAX_PLAYERS> players{
        randomAI, randomAI, randomAI, randomAI, randomAI, randomAI};
    egn::PlayGame play(ante, bigBlind, stakes, players, 0, rngSeed);

    // Simulate games.
    uint64_t gameCount = 0;
    double duration = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    while (duration < maxDuration) {
        play.playAndReset();
        ++gameCount;
        if (gameCount % 100 == 0) {
            auto t2 = std::chrono::high_resolution_clock::now();
            duration = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        }
    }

    // Print results.
    std::cout << "Duration: " << duration << " s" << std::endl;
    std::cout << "Game count: " << gameCount << " games" << std::endl;
    std::cout << 1e9 * duration / gameCount << " nanosec/game" << std::endl;
    std::cout << gameCount / duration << " game/s" << std::endl;
}