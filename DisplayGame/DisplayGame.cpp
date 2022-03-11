#include "../GameEngine/GameStatePrint.h"
#include "../RandomAI/RandomAI.h"
#include "../UserPlayer/UserPlayer.h"
#include "../Blueprint/BlueprintAI.h"

int main()
{
    const egn::chips ante = 0;
    const egn::chips bigBlind = 100;

    const uint8_t firstDealer = 0;
    const unsigned rngSeed = 1;

    const std::string separatorLine = std::string(50, '_') + "\n\n";
    
    // Stakes
    std::array<egn::chips, egn::MAX_PLAYERS> stakes = { 10000, 10000, 10000 };

    // Random AI
    opt::RandomAI randomAI(1.0 / 8, 6.0 / 8, rngSeed);

    // Blueprint AI
    

    // User player
    opt::UserPlayer user(separatorLine);

    // All players
    std::vector<egn::Player*> players = { &randomAI, &randomAI, &user };


    // Play until only one player remains.
    egn::GameStatePrint state(ante, bigBlind, stakes, rngSeed, separatorLine);
    uint8_t dealer = firstDealer;
    uint8_t prevDealer;
    do {
        state.startNewHand(dealer);
        state.printState(std::cout);
        while (!state.finished) {
            players[state.actingPlayer]->act(state);
            state.nextState();
            state.printState(std::cout);
        }
        state.printRewards(std::cout);
        prevDealer = dealer;
    } while (prevDealer != state.nextActive(dealer));
}