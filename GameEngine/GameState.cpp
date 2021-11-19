#include "GameState.h"
#include <random>

namespace egn
{

GameState::GameState(
	uint16_t ante,
	uint16_t smallBlind,
	uint16_t bigBlind,
	const std::array<uint32_t, opt::MAX_PLAYERS>& stakes
) :
	mAnte(ante),
    mSB(smallBlind),
    mBB(bigBlind),

    mRng{ std::random_device{}() },
    mCardDist(0, omp::CARD_COUNT - 1),

    mStakes(stakes),
    mPlayersHands(),
    mBets(),

    mBoardCards(),
    mPots(),

    mCurrentRound(),

    mNPlayers(),
    mPlayers(),
    mCurrentPlayer(),
    mDealer(),
    mOpeningPlayer(),
    mLastBet()
{
    startNewHand();
}

void GameState::startNewHand()
{
    resetPlayers();
    resetBoard();
    mCurrentRound = Round::preflop;

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
        mPlayersHands[j] = omp::Hand::empty();
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

    mOpeningPlayer = *mCurrentPlayer;
    mLastBet = mBB;
}

void GameState::dealHoleCards(uint64_t& usedCardsMask)
{
    for (uint8_t i : mPlayers) {
        dealCards(mPlayersHands[i], omp::HOLE_CARDS, usedCardsMask);
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

void GameState::nextState(uint32_t bet)
{
    // Current player checks or folds.
    if (bet == 0) {
        // Fold
        if (mBets[*mCurrentPlayer] < mLastBet) {
            mCurrentPlayer = mPlayers.erase(mCurrentPlayer);
            --mNPlayers;
        }
        // Check
        else
            goNextPlayer();
    }

    // Current player calls, bets or raises.
    else {
        mStakes[*mCurrentPlayer] -= bet;
        mBets[*mCurrentPlayer] += bet;
        mPots[0] += bet;
        // Bet or raise
        if (mBets[*mCurrentPlayer] > mLastBet) {
            mLastBet = mBets[*mCurrentPlayer];
            mOpeningPlayer = *mCurrentPlayer;
        }
        goNextPlayer();
    }

    // End of the round (we went around the table)
    if (*mCurrentPlayer == mOpeningPlayer) {

        // Everybody folded but one.
        if (mNPlayers == 1)
            mStakes[*mCurrentPlayer] += mPots[0];

        // Showdown
        else if (mCurrentRound == Round::river) {

        }

        // Goes to the next round.
        else {
            for (uint8_t i : mPlayers)
                mBets[i] = 0;
            mLastBet = 0;
            ++mCurrentRound;
            mCurrentPlayer = mPlayers.begin();
            mOpeningPlayer = *mCurrentPlayer;
        }
    }
}

void GameState::goNextPlayer()
{
    if (++mCurrentPlayer == mPlayers.end())
        mCurrentPlayer = mPlayers.begin();
}

} // egn