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
            std::vector<std::vector<uint8_t>> rankings = getRankings();

            // One pot and one winner
            // (deals with this specific case to speed up the computation)
            if (mLastPot == 0 && rankings[0].size() == 1) {
                mStakes[rankings[0][0]] += mPots[0];
                return true;
            }

            // One pot and several winners
            // (deals with this specific case to speed up the computation)
            else if (mLastPot == 0) {
                uint32_t gain = mPots[0] / rankings[0].size();
                for (uint8_t i : rankings[0]) {
                    mStakes[i] += gain;
                }
                // Remaining chips go to the first players after the dealer
                uint8_t extra = mPots[0] % rankings[0].size();
                for (uint8_t i = 0; i < extra; ++i) {
                    ++mStakes[rankings[0][i]];
                }
                return true;
            }
            
            // General case: several pots

            // Computes the number of winners of each pot.
            std::array<uint8_t, opt::MAX_PLAYERS> nWinners;
            bool oneWinnerPerPot = true;
            uint8_t headPot = 0;
            for (std::vector<uint8_t>& sameRankPlayers : rankings) {
                uint8_t maxLastPot = 0;
                for (uint8_t player : sameRankPlayers) {
                    for (uint8_t pot = headPot; pot <= mPlayerLastPots[player]; ++pot) {
                        ++nWinners[pot];
                        if (nWinners[pot] != 1)
                            oneWinnerPerPot = false;
                    }
                    maxLastPot = std::max(maxLastPot, mPlayerLastPots[player]);
                }
                headPot = maxLastPot + 1;
                if (headPot > mLastPot)
                    break;
            }

            // One winner per pot
            // (deals with this specific case to speed up the computation)
            if (oneWinnerPerPot) {
                headPot = 0;
                for (std::vector<uint8_t>& sameRankPlayers : rankings) {
                    for (uint8_t pot = headPot; pot <= mPlayerLastPots[sameRankPlayers[0]]; ++pot) {
                        mStakes[sameRankPlayers[0]] += mPots[pot];
                    }
                    headPot = mPlayerLastPots[sameRankPlayers[0]] + 1;
                    if (headPot > mLastPot)
                        break;
                }
                return true;
            }

            // Computes the gains and extras of each pot.
            std::array<uint32_t, opt::MAX_PLAYERS> gains;
            std::array<uint8_t, opt::MAX_PLAYERS> extras;
            for (uint8_t i = 0; i <= mLastPot; ++i) {
                gains[i] = mPots[i] / nWinners[i];
                extras[i] = mPots[i] % nWinners[i];
            }

            // Distributes the gains and extras to the winners.
            headPot = 0;
            for (std::vector<uint8_t>& sameRankPlayers : rankings) {
                uint8_t maxLastPot = 0;
                for (uint8_t player : sameRankPlayers) {
                    for (uint8_t pot = headPot; pot <= mPlayerLastPots[player]; ++pot) {
                        mStakes[player] += gains[pot];
                        if (extras[pot] != 0) {
                            ++mStakes[player];
                            --extras[pot];
                        }
                    }
                    maxLastPot = std::max(maxLastPot, mPlayerLastPots[player]);
                }
                headPot = maxLastPot + 1;
                if (headPot > mLastPot)
                    break;
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
    // Skip all-in players
    do {
        if (++mCurrentPlayer == mPlayers.end())
            mCurrentPlayer = mPlayers.begin();
    } while (mStakes[*mCurrentPlayer] == 0);
}

std::vector<std::vector<uint8_t>> GameState::getRankings() const
{
    // Computes players' ranks.
    std::vector<uint16_t> ranks(mNPlayers);
    std::vector<uint8_t> players(mNPlayers);
    std::vector<uint8_t> range(mNPlayers);
    uint16_t bestRank = 0;
    std::vector<uint8_t> bestRankPlayers;
    uint8_t i = 0;
    for (uint8_t player : mPlayers) {
        Hand hand = mBoardCards + mPlayerHands[player];
        ranks[i] = mEval.evaluate(hand);
        players[i] = player;
        range[i] = i;
        if (ranks[i] > bestRank) {
            bestRank = ranks[i];
            bestRankPlayers = { player };
        }
        else if (ranks[i] == bestRank) {
            bestRankPlayers.push_back(player);
        }
        ++i;
    }

    // One pot
    // (deals with this specific case to speed up the computation)
    if (mLastPot == 0)
        return { bestRankPlayers };

    // stable_sort is needed to preserve the order of the players
    // with the same rank for the distribution of the extras
    // in clockwise order.
    std::stable_sort(
        range.begin(), range.end(),
        [&](uint8_t i, uint8_t j) { return ranks[i] > ranks[j]; }
    );

    // Builds players' rankings.
    std::vector<std::vector<uint8_t>> rankings;
    for (auto i = range.begin(); i != range.end(); ++i) {
        std::vector<uint8_t> sameRankPlayers = { players[*i] };
        while (i + 1 != range.end() && ranks[*i] == ranks[*(i + 1)]) {
            ++i;
            sameRankPlayers.push_back(players[*i]);
        }
        rankings.emplace_back(sameRankPlayers);
    }

    return rankings;
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
    return mLastRaise - mBets[*mCurrentPlayer];
}

uint32_t GameState::minRaise() const
{
    return mMinRaise - mBets[*mCurrentPlayer];
}

} // egn