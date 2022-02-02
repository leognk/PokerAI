#include "pch.h"
#include "../GameEngine/GameState.h"
#include "../HandData/HandData.h"
#include "CustomStates.h"
#include <fstream>

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

// Verify that the GameState is coherent at each step
// with hands data retrieved from online poker rooms.
// This test allows GameState to confront with real data
// but it cannot be thoroughly inspected at each step
// (we only have hand histories, not the complete states).
TEST(GameStateTest, CoherentWithData)
{
    std::ifstream file(hdt::compressedHandDataFile);

    // Loop over data of hands.
    while (true) {

        // Retrieve hand data.
        hdt::HandHistory hist;
        if (!(hdt::readCompressedData(file, hist))) break;

        EXPECT_LE(hist.maxPlayers, egn::MAX_PLAYERS);

        // Create stakes array.
        std::array<egn::chips, egn::MAX_PLAYERS> stakes{};
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

                EXPECT_FALSE(state.finished);
                EXPECT_EQ(egn::Round(i), state.round);
                EXPECT_EQ(a.player, state.actingPlayer);

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
                    EXPECT_TRUE(raiseInList);
                }

                // Verify that the bet value is correct.
                switch (a.action) {
                case egn::FOLD:
                    break;
                case egn::CALL:
                    EXPECT_EQ(a.bet, state.call);
                    break;
                case egn::RAISE:
                    if (a.bet < state.minRaise) {
                        EXPECT_EQ(a.bet, state.allin);
                        EXPECT_GT(a.bet, state.call);
                    }
                    else
                        EXPECT_LE(a.bet, state.allin);
                    break;
                default:
                    throw std::runtime_error("Unknown action.");
                }

                state.action = a.action;
                state.bet = a.bet;
                state.nextState();
            }
        }
        EXPECT_TRUE(state.finished);

        // Verify that the rewards are correct.
        egn::dchips rake = 0;
        for (uint8_t i = 0; i < hist.maxPlayers; ++i) {
            if (hist.collectedPot[i]) {
                EXPECT_GE(state.rewards[i], hist.rewards[i]);
                rake += state.rewards[i] - hist.rewards[i];
            }
            else
                EXPECT_EQ(hist.rewards[i], state.rewards[i]);
        }
        EXPECT_EQ(hist.rake, rake);
    }
}

// Verify that the GameState is equal at each step
// to the handcrafted states.
// This test allows GameState to be thoroughly inspected
// at each step and with some twisted situations,
// but not to confront with real data.
TEST(GameStateTest, VerifyWithCustomStates)
{
    std::ifstream file(cus::customStatesPath);
    std::vector<cus::History> listHist;
    file >> listHist;

    for (const cus::History& hist : listHist) {

        // Initialize GameState.
        egn::GameState state(hist.ante, hist.bb, hist.initialStakes);
        state.setBoardCards(hist.boardCards);
        for (uint8_t i = 0; i < egn::MAX_PLAYERS; ++i)
            state.setHoleCards(i, hist.hands[i]);
        state.startNewHand(hist.dealer, false);

        // Loop over states.
        for (const cus::State& cState : hist.states) {

            EXPECT_EQ(state.finished, cState.finished);
            for (uint8_t i = 0; i < egn::MAX_PLAYERS; ++i)
                EXPECT_EQ(state.stakes[i], cState.stakes[i]);

            if (cState.finished) break;

            EXPECT_EQ(state.actingPlayer, cState.actingPlayer);
            EXPECT_EQ(state.call, cState.call);
            EXPECT_EQ(state.minRaise, cState.minRaise);
            EXPECT_EQ(state.allin, cState.allin);
            EXPECT_EQ(state.nActions, cState.nActions);
            for (uint8_t i = 0; i < state.nActions; ++i)
                EXPECT_EQ(state.actions[i], cState.actions[i]);
            EXPECT_EQ(state.round, cState.round);

            state.action = cState.nextAction;
            state.bet = cState.nextBet;
            state.nextState();
        }

        for (uint8_t i = 0; i < egn::MAX_PLAYERS; ++i)
            EXPECT_EQ(state.rewards[i], hist.rewards[i]);
    }
}