#include "GameState.h"
#include <random>

namespace egn
{

GameState::GameState(
	uint16_t ante,
	uint16_t smallBlind,
	uint16_t bigBlind,
	const std::vector<uint32_t>& stakes
) :
	mAnte(ante),
	mSmallBlind(smallBlind),
	mBigBlind(bigBlind),

    mRng{ std::random_device{}() },
    mCardDist(0, omp::CARD_COUNT - 1),

	mNPlayers(stakes.size()),
    mPlayers(mNPlayers),
    mBoard(),

    mCurrentRound(Round::preflop),

    mNAlivePlayers(0),
    mAlivePlayersIds(),
    mCurrentPlayerIdx(),
    mDealerIdx(0),
    mOpeningPlayerIdx(0),
    mLastBet(0)
{
    // Initializes players' stakes.
    for (size_t i = 0; i < mNPlayers; ++i) {
        mPlayers[i].stake = stakes[i];
    }
    startNewHand();
}

void GameState::startNewHand()
{
    resetPlayers();
    mBoard = Board();
    mCurrentRound = Round::preflop;
    resetAlivePlayers();

    chargeAnte();
    chargeBlinds();

    // Deals cards.
    uint64_t usedCardsMask = 0;
    dealHoleCards(usedCardsMask);
    dealBoardCards(usedCardsMask);
}

void GameState::resetPlayers()
{
    // Resets the players.
    for (Player& player : mPlayers) {
        if (!player.active)
            continue;
        player.hand = omp::Hand::empty();
        player.alive = true;
        player.gain = 0;
    }
}

void GameState::resetAlivePlayers()
{
    mAlivePlayersIds.clear();
    mNAlivePlayers = 0;
    // The first acting player after the preflop is always
    // the player following the dealer.
    uint8_t i = mDealerIdx + 1;
    do {
        if (!mPlayers[i].active)
            continue;
        mAlivePlayersIds.push_back(i);
        ++mNAlivePlayers;
        i = (i + 1) % mNPlayers;
    } while (i != mDealerIdx + 1);
}

void GameState::chargeAnte()
{
    for (uint8_t i : mAlivePlayersIds) {
        mPlayers[i].stake -= mAnte;
        mBoard.pots[0] += mAnte;
    }
}

void GameState::chargeBlinds()
{
    mCurrentPlayerIdx = mAlivePlayersIds.begin();
    uint8_t smallBlindIdx;
    uint8_t bigBlindIdx;
    // Heads-up
    if (mNAlivePlayers == 2) {
        bigBlindIdx = *mCurrentPlayerIdx;
        ++mCurrentPlayerIdx;
        smallBlindIdx = *mCurrentPlayerIdx;
    }
    else {
        smallBlindIdx = *mCurrentPlayerIdx;
        ++mCurrentPlayerIdx;
        bigBlindIdx = *mCurrentPlayerIdx;
        ++mCurrentPlayerIdx;
    }

    mPlayers[smallBlindIdx].bet += mSmallBlind;
    mBoard.pots[0] += mSmallBlind;
    mPlayers[bigBlindIdx].bet += mBigBlind;
    mBoard.pots[0] += mBigBlind;

    mLastBet = mBigBlind;
    mOpeningPlayerIdx = *mCurrentPlayerIdx;
}

void GameState::dealHoleCards(uint64_t& usedCardsMask)
{
    for (Player& player : mPlayers) {
        dealCards(player.hand, omp::HOLE_CARDS, usedCardsMask);
    }
}

void GameState::dealBoardCards(uint64_t& usedCardsMask)
{
    dealCards(mBoard.cards, omp::BOARD_CARDS, usedCardsMask);
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
    Player& currentPlayer = mPlayers[*mCurrentPlayerIdx];

    // Current player checks or folds.
    if (bet == 0) {
        // Fold
        if (currentPlayer.bet < mLastBet) {
            currentPlayer.alive = false;
            currentPlayer.bet = 0;
            mCurrentPlayerIdx = mAlivePlayersIds.erase(mCurrentPlayerIdx);
            --mNAlivePlayers;
        }
    }

    // Current player calls, bets or raises.
    else {
        currentPlayer.stake -= bet;
        currentPlayer.bet += bet;
        currentPlayer.gain -= bet;
        mBoard.pots[0] += bet;
        // Bet or raise
        if (currentPlayer.bet > mLastBet) {
            mLastBet = currentPlayer.bet;
            mOpeningPlayerIdx = *mCurrentPlayerIdx;
        }
    }

    // Goes to the next player.
    if (++mCurrentPlayerIdx == mAlivePlayersIds.end())
        mCurrentPlayerIdx = mAlivePlayersIds.begin();

    // End of the round (we went around the table)
    if (*mCurrentPlayerIdx == mOpeningPlayerIdx) {
        // Everybody folded but one.
        if (mNAlivePlayers == 1) {
            mPlayers[*mCurrentPlayerIdx].stake += mBoard.pots[0];
            mPlayers[*mCurrentPlayerIdx].bet = 0;
            mPlayers[*mCurrentPlayerIdx].gain += mBoard.pots[0];
        }
        // Showdown
        else if (mCurrentRound == Round::river) {

        }
        // Goes to the next round.
        else {
            for (uint8_t i : mAlivePlayersIds)
                mPlayers[i].bet = 0;
            mLastBet = 0;
            ++mCurrentRound;
            mCurrentPlayerIdx = mAlivePlayersIds.begin();
            mOpeningPlayerIdx = *mCurrentPlayerIdx;
        }
    }
}

} // egn