#include "pch.h"
#include "../Blueprint/BlueprintAI.h"
#include "../RandomAI/RandomAI.h"

TEST(BlueprintAITest, AbcInfoActingPlayerIsCorrect) {

    const unsigned rngSeed = 1;

    const uint64_t endIter = 10000;

    const egn::chips minAnte = 0;
    const egn::chips maxAnte = 100;

    const egn::chips minBB = 2;
    const egn::chips maxBB = 1000;

    const egn::chips maxStake = egn::chips(1e5);

    const uint8_t minNBlueprintAI = 1;
    const uint8_t maxNBlueprintAI = bp::MAX_PLAYERS;


    omp::XoroShiro128Plus rng(rngSeed);

    auto anteDist = omp::FastUniformIntDistribution<egn::chips>(minAnte, maxAnte);
    auto bbDist = omp::FastUniformIntDistribution<egn::chips>(minBB, maxBB);
    auto nPlayersDist = omp::FastUniformIntDistribution<uint8_t>(2, bp::MAX_PLAYERS);
    auto nBlueprintAIDist = omp::FastUniformIntDistribution<uint8_t>(minNBlueprintAI, maxNBlueprintAI);
    auto playerPosition1Dist = omp::FastUniformIntDistribution<uint8_t>(0, egn::MAX_PLAYERS - 1);
    auto playerPosition2Dist = omp::FastUniformIntDistribution<uint8_t>(0, bp::MAX_PLAYERS - 1);
    auto initialStakeDist = omp::FastUniformIntDistribution<egn::chips>(1, maxStake);

    // Load blueprint.
    bp::Blueprint blueprint(
        bp::BP_GAME_NAMESPACE::BLUEPRINT_GAME_NAME,
        bp::BP_BUILD_NAMESPACE::BLUEPRINT_BUILD_NAME,
        (unsigned)rng());
    blueprint.loadStrat();

    // Blueprint AI
    auto blueprintAI = BLUEPRINT_AI_BUILDER(
        bp::BP_GAME_NAMESPACE, bp::BP_BUILD_NAMESPACE, 2, &blueprint, (unsigned)rng());

    std::array<egn::chips, egn::MAX_PLAYERS> initialStakes;
    const std::vector<egn::Player*> updatePlayers = { &blueprintAI };
    std::array<egn::Player*, egn::MAX_PLAYERS> players;
    std::vector<uint8_t> playerPositions;
    std::vector<opt::RandomAI> randomPlayers;

    // Simulate random games.

    for (uint64_t currIter = 0; currIter < endIter; ++currIter) {

        // Ante
        const egn::chips ante = anteDist(rng);

        // BB
        bbDist.init((std::max)(2 * ante, minBB), maxBB);
        const egn::chips bigBlind = bbDist(rng);

        blueprintAI.setBB(bigBlind);

        // nPlayers
        const uint8_t nPlayers = nPlayersDist(rng);

        // Stakes
        initialStakeDist.init(ante + bigBlind + 1, maxStake);
        initialStakes.fill(0);
        for (uint8_t i = 0; i < nPlayers; ++i) {
            uint8_t pos;
            do {
                pos = playerPosition1Dist(rng);
            } while (initialStakes[pos]);
#pragma warning(suppress: 28020)
            initialStakes[pos] = initialStakeDist(rng);
        }

        // playerPositions
        playerPositions.clear();
        for (uint8_t i = 0; i < egn::MAX_PLAYERS; ++i) {
            if (initialStakes[i]) playerPositions.push_back(i);
        }

        // Dealer
        playerPosition2Dist.init(0, nPlayers - 1);
        const uint8_t firstDealer = playerPositions[playerPosition2Dist(rng)];

        // Put blueprintAI in players.
        nBlueprintAIDist.init(minNBlueprintAI, (std::min)(nPlayers, maxNBlueprintAI));
        const uint8_t nBlueprintAI = nBlueprintAIDist(rng);
        for (uint8_t i = 0; i < nBlueprintAI; ++i) {
            playerPosition2Dist.init(0, (uint8_t)playerPositions.size() - 1);
            const uint8_t pos = playerPosition2Dist(rng);
            players[playerPositions[pos]] = &blueprintAI;
            playerPositions.erase(playerPositions.begin() + pos);
        }

        // Build randomAIs.
        const uint8_t nRandomAI = nPlayers - nBlueprintAI;
        randomPlayers.clear();
        for (uint8_t i = 0; i < nRandomAI; ++i) {
            double foldProba = (double)rng() / (std::numeric_limits<uint64_t>::max)();
            double callProba;
            if (foldProba == 1.0) callProba = 0.0;
            else callProba = (double)rng() / (std::numeric_limits<uint64_t>::max)();
            randomPlayers.push_back(opt::RandomAI(foldProba, callProba, (unsigned)rng()));
        }

        // Put randomAI in players.
        for (uint8_t i = 0; i < nRandomAI; ++i) {
            playerPosition2Dist.init(0, (uint8_t)playerPositions.size() - 1);
            const uint8_t pos = playerPosition2Dist(rng);
            players[playerPositions[pos]] = &randomPlayers[i];
            playerPositions.erase(playerPositions.begin() + pos);
        }

        // Play until only one player remains.
        egn::GameState state(ante, bigBlind, initialStakes, (unsigned)rng());
        uint8_t dealer = firstDealer;
        do {
            state.startNewHand(dealer);
            for (const auto& p : updatePlayers) p->reset(state);
            while (!state.finished) {

                if (!blueprintAI.abcInfo.state.finished) {
                    EXPECT_EQ(blueprintAI.abcInfo.state.round, state.round);
                    EXPECT_EQ(blueprintAI.abcInfo.state.actingPlayer, state.actingPlayer);
                }

                players[state.actingPlayer]->act(state);
                for (const auto& p : updatePlayers) p->update(state);
                state.nextState();
            }

            // Eliminate players who have a stake smaller than ante + BB.
            for (egn::chips& x : state.stakes) {
                if (x < ante + bigBlind + 1) x = 0;
            }
            state.nextActive(dealer);
        } while (state.foundActivePlayers());
    }
}