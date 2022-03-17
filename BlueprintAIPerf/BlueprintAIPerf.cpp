#include "../Blueprint/BlueprintAI.h"
#include "../RandomAI/RandomAI.h"

// Simulate random games with 1 blueprintAI vs randomAIs (randomly built).
int main()
{
    const double endDuration = 2;
    const unsigned rngSeed = 2;


    // Rng
    omp::XoroShiro128Plus rng(rngSeed);

    // Initial stakes
    std::array<egn::chips, egn::MAX_PLAYERS> initialStakes;
    for (uint8_t i = 0; i < bp::MAX_PLAYERS; ++i)
        initialStakes[i] = bp::INITIAL_STAKE;

    // Load blueprint.
    bp::Blueprint blueprint(
        bp::BP_GAME_NAMESPACE::BLUEPRINT_GAME_NAME,
        bp::BP_BUILD_NAMESPACE::BLUEPRINT_BUILD_NAME,
        (unsigned)rng());
    blueprint.loadStrat();

    // Blueprint AI
    auto blueprintAI = BLUEPRINT_AI_BUILDER(
        bp::BP_GAME_NAMESPACE, bp::BP_BUILD_NAMESPACE, bp::BIG_BLIND, &blueprint, (unsigned)rng());
    auto bpAIPosDist = omp::FastUniformIntDistribution<uint8_t>(0, bp::MAX_PLAYERS - 1);

    // Random AIs
    const uint8_t nRandomAI = bp::MAX_PLAYERS - 1;
    std::vector<opt::RandomAI> randomAIs;
    for (uint8_t i = 0; i < nRandomAI; ++i)
        randomAIs.push_back(opt::RandomAI(0.0, 1.0, (unsigned)rng()));

    // All players
    const std::vector<egn::Player*> updatePlayers = { &blueprintAI };
    std::array<egn::Player*, egn::MAX_PLAYERS> players;

    // Simulate random games.

    const opt::time_t startTime = opt::getTime();
    double currDuration = 0;
    uint64_t gameCount = 0;

    // Stats
    double bpAIAvgGain = 0;
    double bpAIGainStd = 0;
    double bpAIMinGain = 0;
    double bpAIMaxGain = 0;
    double bpAIAccGain = 0;
    double bpAIMinAccGain = 0;

    while (currDuration < endDuration) {

        // Choose a seat for blueprintAI.
        const uint8_t bpAIPos = bpAIPosDist(rng);
        players[bpAIPos] = &blueprintAI;

        // Build randomAIs and put them in players.
        uint8_t pos = 0;
        for (uint8_t i = 0; i < nRandomAI; ++i) {
            randomAIs[i].setRandomProbas(rng);
            if (pos == bpAIPos) ++pos;
            players[pos++] = &randomAIs[i];
        }

        // Play until only one player remains.

        egn::GameState state(bp::ANTE, bp::BIG_BLIND, initialStakes, (unsigned)rng());
        uint8_t dealer = 0;

        do {
            // Play a hand.
            state.startNewHand(dealer);
            for (const auto& p : updatePlayers) p->reset(state);
            while (!state.finished) {
                players[state.actingPlayer]->act(state);
                for (const auto& p : updatePlayers) p->update(state);
                state.nextState();
            }
            ++gameCount;

            // Calculate mean and std of blueprintAI's gain online.
            const double r = (double)state.reward(bpAIPos);
            const double delta = r - bpAIAvgGain;
            bpAIAvgGain += delta / gameCount;
            bpAIGainStd += delta * (r - bpAIAvgGain);

            // Update other stats.
            if (r < bpAIMinGain) bpAIMinGain = r;
            if (r > bpAIMaxGain) bpAIMaxGain = r;
            bpAIAccGain += r;
            if (bpAIAccGain < bpAIMinAccGain) bpAIMinAccGain = bpAIAccGain;

            // Eliminate players who have a stake smaller than ante + BB + 1.
            for (egn::chips& x : state.stakes) {
                if (x < bp::ANTE + bp::BIG_BLIND + 1) x = 0;
            }

            // Move the button.
            state.nextActive(dealer);

        } while (state.foundActivePlayers());

        currDuration = opt::getDuration(startTime);
    }

    // Calculate mean and std of blueprintAI's gain.
    bpAIAvgGain /= bp::BIG_BLIND;
    bpAIGainStd = std::sqrt(bpAIGainStd / (gameCount - 1));
    bpAIGainStd /= bp::BIG_BLIND;

    // Calculate other stats.
    bpAIMinGain /= bp::BIG_BLIND;
    bpAIMaxGain /= bp::BIG_BLIND;
    bpAIMinAccGain /= bp::BIG_BLIND;

    // Print stats.
    std::cout
        << opt::prettyNumDg(currDuration, 3, true) << "s\n"
        << opt::prettyNumDg(gameCount, 3, true) << "games\n"
        << opt::prettyNumDg(gameCount / currDuration, 3, true) << "games/s\n\n"

        << "Avg gain: " << opt::prettyNumDg(bpAIAvgGain, 3, true) << "BB/game\n"
        << "Std gain: " << opt::prettyNumDg(bpAIGainStd, 3, true) << "BB/game\n\n"

        << "Min gain: " << opt::prettyNumDg(bpAIMinGain, 3, true) << "BB\n"
        << "Max gain: " << opt::prettyNumDg(bpAIMaxGain, 3, true) << "BB\n"
        << "Min acc gain: " << opt::prettyNumDg(bpAIMinAccGain, 3, true) << "BB\n";
}