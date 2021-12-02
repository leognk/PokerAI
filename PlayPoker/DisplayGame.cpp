#include "../GameEngine/PlayGame.h"
#include "../RandomAI/RandomAI.h"
#include <iostream>

int main()
{
    // Constants
    const unsigned rngSeed = 1;
    const uint32_t ante = 1, bigBlind = 10;
    const uint32_t stake = 1000;
    const uint8_t dealer = 0;
    const double foldProba = 1. / 3;
    const double callProba = 1. / 3;


    // Define game variables.
    std::array<uint32_t, opt::MAX_PLAYERS> stakes{
        stake, stake, stake, stake, stake, stake };
    opt::RandomAI randomAI(foldProba, callProba, rngSeed);
    std::array<egn::Player, opt::MAX_PLAYERS> players{
        randomAI, randomAI, randomAI, randomAI, randomAI, randomAI };
    egn::PlayGame play(ante, bigBlind, stakes, players, dealer, rngSeed);

    egn::GameState state(ante, bigBlind, stakes, rngSeed);
    //state.startNewHand(dealer);
    //std::cout << state;
    //state.nextState(randomAI.act(state));
    //std::cout << state;
}