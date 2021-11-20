#include "GameState.h"
#include <random>

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
        mPots[0] += bet;
        // Raise
        if (mBets[*mCurrentPlayer] > mLastRaise) {
            mMinRaise = 2 * mBets[*mCurrentPlayer] - mLastRaise;
            mLastRaise = mBets[*mCurrentPlayer];
            mInitiator = *mCurrentPlayer;
        }
        goNextPlayer();
    }

    // End of the round (we went around the table)
    if (*mCurrentPlayer == mInitiator) {

        // Everybody folded but one.
        if (mNPlayers == 1) {
            mStakes[*mCurrentPlayer] += mPots[0];
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

void GameState::goNextPlayer()
{
    if (++mCurrentPlayer == mPlayers.end())
        mCurrentPlayer = mPlayers.begin();
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

uint32_t GameState::stake() const
{
    return mStakes[*mCurrentPlayer];
}

uint32_t GameState::call() const
{
    return mLastRaise;
}

uint32_t GameState::minRaise() const
{
    return mMinRaise;
}

} // egn