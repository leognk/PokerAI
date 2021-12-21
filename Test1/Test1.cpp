#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>
#include <locale>
#include <bitset>
#include <boost/regex/icu.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <json/json.h>

#include "../GameEngine/GameState.h"
#include "../HandData/HandData.h"
#include "../Test-GameEngine/CustomStates.h"

namespace fs = std::filesystem;

int main()
{
    std::ifstream file(cus::customStatesPath);
    std::vector<cus::History> listHist;
    file >> listHist;

    for (const cus::History& hist : listHist) {

        // Initialize GameState.
        egn::GameState state(hist.ante, hist.bb, hist.initialStakes);
        state.startNewHand(hist.dealer, false);
        for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i)
            state.setHoleCards(i, hist.hands[i]);
        state.setBoardCards(hist.boardCards);

        // Loop over states.
        for (unsigned i = 0; i < hist.states.size() - 1; ++i) {
            state.nextState(
                hist.states[i].nextAction, hist.states[i].nextBet);
        }
    }
}