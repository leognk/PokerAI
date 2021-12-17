#include "../GameEngine/GameStatePrint.h"
#include "../RandomAI/RandomAI.h"
#include "../UserPlayer/UserPlayer.h"
#include <iostream>

int main()
{
    // Constants
    const unsigned rngSeed = 1;
    const egn::chips ante = 1, bigBlind = 10;
    const egn::chips stake = 1000;
    uint8_t dealer = 0;
    const double foldProba = 1. / 8;
    const double callProba = 6. / 8;
    const bool playToEnd = false;


    // Define game variables.
    std::array<egn::chips, opt::MAX_PLAYERS> stakes{
        stake, stake, stake, stake, stake, stake };
    std::string separatorLine = std::string(50, '_') + "\n\n";
    opt::UserPlayer user(separatorLine);
    opt::RandomAI randomAI(foldProba, callProba, rngSeed);
    std::array<egn::Player*, opt::MAX_PLAYERS> players{
        &user, &randomAI, &randomAI, &randomAI, &randomAI, &randomAI };
    egn::GameStatePrint state(ante, bigBlind, stakes, rngSeed, separatorLine);

    // Play until only one player remains.
    if (playToEnd) {
        uint8_t prevDealer = 0;
        do {
            state.startNewHand(dealer);
            state.printState(std::cout);
            while (!state.finished) {
                state.nextState(players[state.actingPlayer]->act(state));
                state.printState(std::cout);
            }
            state.printRewards(std::cout);
            prevDealer = dealer;
        } while (prevDealer != state.nextActive(dealer));
    }

    // Play one hand.
    else {
        state.startNewHand(dealer);
        state.printState(std::cout);
        while (!state.finished) {
            state.nextState(players[state.actingPlayer]->act(state));
            state.printState(std::cout);
        }
        state.printRewards(std::cout);
    }
}