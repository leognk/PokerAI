#include "GameStatePrint.h"
#include "../OMPEval/omp/Constants.h"
#include <iostream>
#include <iomanip>
#include <string>

namespace egn {

#pragma warning(suppress: 26495)
GameStatePrint::GameStatePrint(
    chips ante, chips bigBlind,
    const std::array<chips, opt::MAX_PLAYERS>& stakes,
    unsigned rngSeed,
    std::string separatorLine) :

    GameState(ante, bigBlind, stakes, rngSeed),
    mSeparatorLine(separatorLine)
{
}

void GameStatePrint::startNewHand(uint8_t dealerIdx)
{
    mFirstActive = dealerIdx;
    nextActive(mFirstActive);
    for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i)
        mLastActions[i] = "";
    mPrevActionRound = Round::preflop;
    mPrevActing = opt::MAX_PLAYERS + 1;

    GameState::startNewHand(dealerIdx);

    // Initialize mRoundBets.
    for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i) {
        if (mBets[i] > mAnte)
            mRoundBets[i] = mBets[i] - mAnte;
        else
            mRoundBets[i] = 0;
    }
}

void GameStatePrint::nextState(chips bet)
{
    // We went to the next round.
    if (mPrevActionRound != round) {
        // Reset mLastActions.
        for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i) {
            if (mLastActions[i] != "fold"
                && mLastActions[i] != "all-in")
                mLastActions[i] = "";
        }
        for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i) {
            mRoundBets[i] = 0;
        }
    }

    if (bet == allin)
        mLastActions[actingPlayer] = "all-in";
    else if (bet == call) {
        if (call)
            mLastActions[actingPlayer] = "call";
        else
            mLastActions[actingPlayer] = "check";
    }
    else if (bet == fold)
        mLastActions[actingPlayer] = "fold";
    else
        mLastActions[actingPlayer] = "raise";
    mLastBets[actingPlayer] = bet;
    mPrevActionRound = round;
    mPrevActing = mCurrentActing;
    mRoundBets[actingPlayer] += bet;

    GameState::nextState(bet);
}

// Do not skip all-in players.
uint8_t& GameStatePrint::nextActiveInGame(uint8_t& i) const
{
    do {
        (++i) %= opt::MAX_PLAYERS;
    } while (!stakes[i] && !mAlive[i]);
    return i;
}

std::ostream& GameStatePrint::printState(std::ostream& os) const
{
    os << "Round: " << round;
    os << " | Board: " << mBoardCards;
    os << " | Pot: " << mPot << std::endl << std::endl;

    unsigned stakeMaxChars = maxChars(stakes);
    unsigned betMaxChars = maxChars(mRoundBets);
    unsigned actionMaxChars = maxCharsString(mLastActions);
    unsigned lastBetMaxChars = maxChars(mLastBets);

    uint8_t i = mFirstActive;
    do {
        if (i == actingPlayer && !finished)
            os << "*";
        else if (i == mPrevActing)
            os << "-";
        else
            os << " ";
        os << unsigned(i);
        os << " | cards: " << mHands[i];
        os << " | stake: " << std::setw(stakeMaxChars) << stakes[i];
        os << " | bet: " << std::setw(betMaxChars) << mRoundBets[i];
        // Print player's last action.
        os << " | ";
        if (!mLastActions[i].empty()) {
            os << std::left << std::setw(actionMaxChars) << mLastActions[i];
            os << std::right;
            if (mLastBets[i] && mLastActions[i] != "all-in")
                os << " " << std::setw(lastBetMaxChars) << mLastBets[i];
        }
        os << std::endl;
    } while (nextActiveInGame(i) != mFirstActive);

    return os << mSeparatorLine;
}

std::ostream& GameStatePrint::printRewards(std::ostream& os) const
{
    os << "Rewards:" << "\n\n";
    unsigned rewardMaxChars = maxChars(rewards);
    uint8_t i = mFirstActive;
    do {
        os << unsigned(i) << ": "
            << std::setw(rewardMaxChars) << rewards[i]
            << " | " << handCategory(i) << std::endl;
    } while (nextActiveInGame(i) != mFirstActive);
    return os << mSeparatorLine;
}

template<typename Container>
unsigned GameStatePrint::maxChars(Container c) const
{
    unsigned res = 0;
    uint8_t i = mFirstActive;
    do {
#pragma warning(suppress: 4267)
        unsigned nChars = std::to_string(c[i]).size();
        if (res < nChars)
            res = nChars;
    } while (nextActiveInGame(i) != mFirstActive);
    return res;
}

template<typename Container>
unsigned GameStatePrint::maxCharsString(Container c) const
{
    unsigned res = 0;
    uint8_t i = mFirstActive;
    do {
        if (res < c[i].size())
#pragma warning(suppress: 4267)
            res = c[i].size();
    } while (nextActiveInGame(i) != mFirstActive);
    return res;
}

std::string GameStatePrint::handCategory(uint8_t player) const
{
    Hand hand = mBoardCards + mHands[player];
    uint16_t rank = mEval.evaluate(hand);
    switch (rank >> omp::HAND_CATEGORY_SHIFT) {
    case 1: return "High card";
    case 2: return "Pair";
    case 3: return "Two pair";
    case 4: return "Three of a king";
    case 5: return "Straight";
    case 6: return "Flush";
    case 7: return "Full house";
    case 8: return "Four of a kind";
    case 9: return "Straight flush";
    default: return "";
    }
}

} // egn