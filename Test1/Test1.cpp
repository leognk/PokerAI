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

// Return hole cards with the worst rank when combined
// with the given board cards.
egn::Hand getWorstHand(const egn::Hand& boardCards)
{
    // Systematically try this hand before doing
    // a complete search.
    static const egn::Hand candidateHoleCards =
        egn::Hand(uint8_t(0)) + egn::Hand(uint8_t(5));

    omp::HandEvaluator eval;
    uint16_t boardRank = eval.evaluate(
        egn::Hand::empty() + boardCards);

    egn::Hand hand = egn::Hand::empty()
        + candidateHoleCards + boardCards;
    uint16_t worstRank = eval.evaluate(hand);
    if (worstRank == boardRank) return candidateHoleCards;

    // Do a complete search.
    egn::Hand worstHoleCards = candidateHoleCards;
    for (uint8_t c1 = 1; c1 < omp::CARD_COUNT; ++c1) {
        for (uint8_t c2 = 0; c2 < c1; ++c2) {
            egn::Hand holeCards = egn::Hand(c1) + egn::Hand(c2);
            egn::Hand hand = egn::Hand::empty()
                + holeCards + boardCards;
            uint16_t rank = eval.evaluate(hand);
            if (rank == boardRank)
                return holeCards;
            else if (rank < worstRank) {
                worstRank = rank;
                worstHoleCards = holeCards;
            }
        }
    }
    return worstHoleCards;
}

int main()
{
    std::ifstream file(hdt::compressedHandDataFile);

    // Loop over data of hands.
    while (true) {

        // Retrieve hand data.
        hdt::HandHistory hist;
        if (!(hdt::readCompressedData(file, hist))) break;

        // Create stakes array.
        std::array<egn::chips, opt::MAX_PLAYERS> stakes{};
        for (uint8_t i = 0; i < hist.initialStakes.size(); ++i)
            stakes[i] = hist.initialStakes[i];

        // Initialize GameState.
        egn::GameState state(hist.ante, hist.bb, stakes);
        // Set cards.
        if (hist.boardCards.countCards() == 5) {
            state.setBoardCards(hist.boardCards);
            egn::Hand worstHand = getWorstHand(hist.boardCards);
            for (uint8_t i = 0; i < hist.maxPlayers; ++i) {
                if (hist.hands[i] == egn::Hand::empty())
                    state.setHoleCards(i, worstHand);
                else
                    state.setHoleCards(i, hist.hands[i]);
            }
        }
        state.startNewHand(hist.dealer, false);

        // Replay the hand with GameState.
        for (uint8_t i = 0; i < hist.actions.size(); ++i) {
            for (const hdt::Action& a : hist.actions[i]) {

                // Verify that a.action is in state.actions.
                // As we always give the possibility to fold
                // (if a player leaves the table) and to call,
                // we just need to test for the raise.
                if (a.action == egn::RAISE) {
                    bool raiseInList = false;
                    for (uint8_t i = 0; i < state.nActions; ++i) {
                        if (state.actions[i] == egn::RAISE) {
                            raiseInList = true;
                            break;
                        }
                    }
                }

                // Verify that the bet value is correct.
                switch (a.action) {
                case egn::FOLD:
                    break;
                case egn::CALL:
                    break;
                case egn::RAISE:
                    if (a.bet < state.minRaise) {
                    }
                    else;
                    break;
                default:
                    throw std::runtime_error("Unknown action.");
                }

                state.action = a.action;
                state.bet = a.bet;
                state.nextState();
            }
        }

        // Verify that the rewards are correct.
        egn::dchips rake = 0;
        for (uint8_t i = 0; i < hist.maxPlayers; ++i) {
            if (hist.collectedPot[i]) {
                rake += state.rewards[i] - hist.rewards[i];
            }
            else;
        }
    }
}