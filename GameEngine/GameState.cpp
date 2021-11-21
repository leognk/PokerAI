#include "GameState.h"
#include <random>
#include <numeric>

namespace egn
{

#pragma warning(suppress: 26495)
GameState::GameState(unsigned rngSeed) :
    mRng{ (rngSeed == 0) ? std::random_device{}() : rngSeed },
    mCardDist(0, omp::CARD_COUNT - 1)
{
}

void GameState::setAnte(uint16_t ante)
{
    mAnte = ante;
}

void GameState::setBigBlind(uint16_t bigBlind)
{
    mBB = bigBlind;
    mSB = mBB / 2;
}

void GameState::setStakes(std::array<uint32_t, opt::MAX_PLAYERS> stakes)
{
    mStakes = stakes;
}

void GameState::setStake(uint8_t playerIdx, uint32_t stake)
{
    mStakes[playerIdx] = stake;
}

void GameState::startNewHand(uint8_t dealerIdx)
{
    mDealer = dealerIdx;
    mRound = Round::preflop;

    resetPlayers();
    resetBoard();

    chargeAnte();
    chargeBlinds();

    // Deals cards.
    uint64_t usedCardsMask = 0;
    dealHoleCards(usedCardsMask);
    dealBoardCards(usedCardsMask);
}

void GameState::resetPlayers()
{
    mPlayers.clear();
    // The first acting player after the preflop is always
    // the player following the dealer.
    for (uint8_t i = mDealer + 1, j; i < mDealer + opt::MAX_PLAYERS + 1; ++i) {
        j = i % opt::MAX_PLAYERS;
        // Ignores inactive players.
        if (mStakes[j] == 0)
            continue;
        mPlayerHands[j] = omp::Hand::empty();
        mBets[j] = 0;
        mPlayers.push_back(j);
    }
    mNPlayers = mPlayers.size();
}

void GameState::resetBoard()
{
    mBoardCards = omp::Hand::empty();
    for (uint8_t i = 0; i < mPots.size(); ++i)
        mPots[i] = 0;
    mLastPot = 0;
}

void GameState::chargeAnte()
{
    for (uint8_t i : mPlayers)
        mStakes[i] -= mAnte;
    mPots[0] += mNPlayers * mAnte;
}

void GameState::chargeBlinds()
{
    mCurrentPlayer = mPlayers.begin();
    uint8_t sbPlayer, bbPlayer;
    // Heads-up
    if (mNPlayers == 2) {
        bbPlayer = *mCurrentPlayer;
        ++mCurrentPlayer;
        sbPlayer = *mCurrentPlayer;
    }
    else {
        sbPlayer = *mCurrentPlayer;
        ++mCurrentPlayer;
        bbPlayer = *mCurrentPlayer;
        ++mCurrentPlayer;
    }

    mBets[sbPlayer] += mSB;
    mPots[0] += mSB;
    mBets[bbPlayer] += mBB;
    mPots[0] += mBB;

    mInitiator = *mCurrentPlayer;
    mMinRaise = 2 * mBB;
    mLastRaise = mBB;
}

void GameState::dealHoleCards(uint64_t& usedCardsMask)
{
    for (uint8_t i : mPlayers) {
        dealCards(mPlayerHands[i], omp::HOLE_CARDS, usedCardsMask);
    }
}

void GameState::dealBoardCards(uint64_t& usedCardsMask)
{
    dealCards(mBoardCards, omp::BOARD_CARDS, usedCardsMask);
}

void GameState::dealCards(omp::Hand& hand, unsigned nCards, uint64_t& usedCardsMask)
{
    for (unsigned i = 0; i < nCards; ++i) {
        unsigned card;
        uint64_t cardMask;
        do {
            card = mCardDist(mRng);
            cardMask = 1ull << card;
        } while (usedCardsMask & cardMask);
        usedCardsMask |= cardMask;
        hand += omp::Hand(card);
    }
}

bool GameState::nextState(uint32_t bet)
{
    // Current player checks or folds.
    if (bet == 0) {
        // Fold
        if (mBets[*mCurrentPlayer] != mLastRaise) {
            mCurrentPlayer = mPlayers.erase(mCurrentPlayer);
            --mNPlayers;
        }
        // Check
        else {
            goNextPlayer();
        }
    }

    // Current player calls or raises.
    else {
        mStakes[*mCurrentPlayer] -= bet;
        mBets[*mCurrentPlayer] += bet;
        setMaxRaise();
        //mRoundPot += bet; //////////////////
        // Raise
        if (mBets[*mCurrentPlayer] >= mMinRaise) {
            mMinRaise = 2 * mBets[*mCurrentPlayer] - mLastRaise;
            mLastRaise = mBets[*mCurrentPlayer];
            mInitiator = *mCurrentPlayer;
        }
        // All-in not raising
        else if (mStakes[*mCurrentPlayer] == 0) {
            // Incomplete call
            if (mBets[*mCurrentPlayer] < mLastRaise) {
                //uint32_t transfer = (*mCurrentPlayer - mInitiator) * (mLastRaise - mBets[*mCurrentPlayer]);
                //mPots[mLastPot] -= transfer;
                //++mLastPot;
                //mPots[mLastPot] += transfer;
            }
            // Incomplete raise
            else if (mBets[*mCurrentPlayer] != mLastRaise) {

            }
        }
        goNextPlayer();
    }

    // End of the round (we went around the table)
    if (*mCurrentPlayer == mInitiator) {

        // Everybody folded but one.
        if (mNPlayers == 1) {
            mStakes[*mCurrentPlayer] += std::accumulate(
                mPots.begin(), mPots.begin() + mLastPot, 0U
            );
            return true;
        }

        // Showdown
        else if (mRound == Round::river) {
            std::vector<uint8_t> winners = evaluateHands();
            uint32_t gain = mPots[0] / winners.size();
            for (uint8_t i : winners) {
                mStakes[i] += gain;
            }
            // Remaining chips go to the first players after the dealer.
            uint8_t extra = mPots[0] % winners.size();
            for (uint8_t i = 0; i < extra; ++i) {
                ++mStakes[winners[i]];
            }
            return true;

            /////////////////////
            for (std::vector<uint8_t>& winners : )
        }

        // Goes to the next round.
        else {
            for (uint8_t i : mPlayers)
                mBets[i] = 0;
            mMinRaise = mBB;
            mLastRaise = 0;
            ++mRound;
            mCurrentPlayer = mPlayers.begin();
            mInitiator = *mCurrentPlayer;
            return false;
        }
    }
}

void GameState::setMaxRaise()
{
    // Set mMaxRaise to the second largest stake.
    uint32_t max = 0;
    for (uint8_t i : mPlayers) {
        if (mStakes[i] > max) {
            mMaxRaise = max;
            max = mStakes[i];
        }
        else if (mStakes[i] > mMaxRaise && mStakes[i] != max)
            mMaxRaise = mStakes[i];
    }
}

void GameState::goNextPlayer()
{
    // Skip all-in players
    do {
        if (++mCurrentPlayer == mPlayers.end())
            mCurrentPlayer = mPlayers.begin();
    } while (mStakes[*mCurrentPlayer] == 0);
}

std::vector<uint8_t> GameState::evaluateHands() const
{
    uint16_t bestRank = 0;
    std::vector<uint8_t> winners;
    for (uint8_t i : mPlayers) {
        Hand hand = mBoardCards + mPlayerHands[i];
        uint16_t rank = mEval.evaluate(hand);
        if (rank > bestRank) {
            bestRank = rank;
            winners = { i };
        }
        else if (rank == bestRank) {
            winners.push_back(i);
        }
    }
    return winners;
}

uint8_t GameState::currentPlayer() const
{
    return *mCurrentPlayer;
}

uint32_t GameState::allin() const
{
    return std::min(mStakes[*mCurrentPlayer], mMaxRaise);
}

uint32_t GameState::call() const
{
    return mLastRaise - mBets[*mCurrentPlayer];
}

uint32_t GameState::minRaise() const
{
    return mMinRaise - mBets[*mCurrentPlayer];
}

} // egn