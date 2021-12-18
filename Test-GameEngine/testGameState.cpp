#include "pch.h"
#include "../GameEngine/GameState.h"
#include "../HandData/HandData.h"
#include <fstream>

TEST(GameStateTest, CoherentWithData)
{
    std::ifstream file(hdt::compressedHandDataFile);

    // Loop over data of hands.
    //################################################
    unsigned id = 1;
    unsigned id0 = 267;
    //////////////////////////////////////////////////
    while (true) {

        // Retrieve hand data.
        hdt::HandHistory hist;
        if (!(hdt::readCompressedData(file, hist))) break;
        //################################################
        if (id == id0)
            std::cout << hist << "\n\n";
        //////////////////////////////////////////////////

        EXPECT_LE(hist.maxPlayers, opt::MAX_PLAYERS);

        // Create stakes array.
        std::array<egn::chips, opt::MAX_PLAYERS> stakes{};
        for (uint8_t i = 0; i < hist.initialStakes.size(); ++i)
            stakes[i] = hist.initialStakes[i];

        // Initialize GameState.
        egn::GameState state(hist.ante, hist.bb, stakes);
        state.startNewHand(hist.dealer, false);
        for (uint8_t i = 0; i < hist.maxPlayers; ++i)
            state.setHoleCards(i, hist.hands[i]);
        state.setBoardCards(hist.boardCards);

        // Replay the hand with GameState.
        for (uint8_t i = 0; i < hist.actions.size(); ++i) {
            for (const hdt::Action& a : hist.actions[i]) {

                EXPECT_FALSE(state.finished);
                EXPECT_EQ(egn::Round(i), state.round);
                EXPECT_EQ(a.player, state.actingPlayer);

                // Verify that a.action is in state.actions.
                bool actionInList = false;
                for (uint8_t i = 0; i < state.nActions; ++i) {
                    if (state.actions[i] == a.action
                        || (a.action == egn::Action::allin
                            && state.actions[i] == egn::Action::raise
                            && a.bet == state.allin)) {
                        actionInList = true;
                        break;
                    }
                }
                EXPECT_TRUE(actionInList);

                // Verify that the bet value is correct.
                switch (a.action) {
                case egn::Action::fold:
                    EXPECT_EQ(a.bet, state.fold);
                    break;
                case egn::Action::call:
                    EXPECT_EQ(a.bet, state.call);
                    break;
                case egn::Action::raise:
                    EXPECT_LE(state.minRaise, a.bet);
                    EXPECT_LE(a.bet, state.allin);
                    break;
                case egn::Action::allin:
                    EXPECT_EQ(a.bet, state.allin);
                    break;
                default:
                    throw std::runtime_error("Unknown action.");
                }

                state.nextState(a.bet);
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
        ++id; //////////////////////////////////////////////////
    }
}