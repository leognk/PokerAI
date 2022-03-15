#include "../GameEngine/GameStatePrint.h"
#include "../RandomAI/RandomAI.h"
#include "../UserPlayer/UserPlayer.h"
#include "../Blueprint/BlueprintAI.h"

int main()
{
    const uint8_t maxPlayers = 3;

    const egn::chips ante = 0;
    const egn::chips bigBlind = 100;

    const uint8_t firstDealer = 0;
    const unsigned rngSeed = 8;

    const std::string separatorLine = std::string(50, '_') + "\n\n";
    
    // Stakes
    const std::array<egn::chips, egn::MAX_PLAYERS> initialStakes = { 10000, 10000, 10000 };

    // Random AI
    opt::RandomAI randomAI(1.0 / 8, 6.0 / 8, rngSeed);

    // Blueprint AI
    bp::Blueprint blueprint(
        bp::simple::BLUEPRINT_GAME_NAME,
        bp::medium::BLUEPRINT_BUILD_NAME,
        rngSeed);
    blueprint.loadStrat();
    auto blueprintAI = BLUEPRINT_AI_BUILDER(
        bp::simple, bp::medium, bigBlind, &blueprint, rngSeed);

    // User player
    opt::UserPlayer user(separatorLine);

    // All players
    const std::vector<egn::Player*> uniquePlayers = { &randomAI, &blueprintAI, &user };
    const std::vector<egn::Player*> players = { &randomAI, &blueprintAI, &randomAI };


    // Play until only one player remains.
    egn::GameStatePrint state(ante, bigBlind, initialStakes, rngSeed, separatorLine);
    uint8_t dealer = firstDealer;
    do {
        //state.stakes = initialStakes;
        state.startNewHand(dealer);
        for (const auto& p : uniquePlayers)
            p->reset(state);
        state.printState(std::cout);
        while (!state.finished) {
            players[state.actingPlayer]->act(state);
            for (const auto& p : uniquePlayers) p->update(state);
            state.nextState();
            state.printState(std::cout);
        }
        state.printRewards(std::cout);
        state.nextActive(dealer);
    } while (state.foundActivePlayers());
}