#include "GameState.h"
#include <random>
#include <numeric>

namespace egn
{

#pragma warning(suppress: 26495)
GameState::GameState(unsigned rngSeed) :
    mRng{ (!rngSeed) ? std::random_device{}() : rngSeed },
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
    mAllInFlag = false;

    resetPlayers();
    resetBoard();

    chargeAnte();
    chargeBlinds();

    // Deals cards.
    uint64_t usedCardsMask = 0;
    dealHoleCards(usedCardsMask);
    dealBoardCards(usedCardsMask);
}

uint8_t& GameState::nextAlive(uint8_t& i) const
{
    do {
        ++i %= opt::MAX_PLAYERS;
    } while (!mAlive[i]);
    return i;
}

uint8_t& GameState::nextActing(uint8_t& i) const
{
    do {
        ++i %= opt::MAX_PLAYERS;
    } while (!mActing[i]);
    return i;
}

void GameState::eraseAlive(uint8_t& i)
{
    mAlive[i] = false;
    if (i == mFirstAlive)
        nextAlive(mFirstAlive);
    --mNAlive;
}

void GameState::eraseActing(uint8_t& i)
{
    mActing[i] = false;
    if (i == mFirstActing)
        nextActing(mFirstActing);
    --mNActing;
}

void GameState::resetPlayers()
{
    mNAlive = 0;
    // The first acting player after the preflop is always
    // the player following the dealer.
    uint8_t i = mDealer + 1;
    do {
        // Ignores inactive players.
        if (!mStakes[i])
            continue;
        mHands[i] = omp::Hand::empty();
        mBets[i] = 0;
        mAlive[i] = true;
        ++mNAlive;
        mActing[i] = true;
        ++mNActing;
        mActed[i] = false;
    } while (++i % opt::MAX_PLAYERS != mDealer + 1);
    mFirstAlive = nextAlive(mDealer);
    mFirstActing = nextActing(mDealer);
}

void GameState::resetBoard()
{
    mBoardCards = omp::Hand::empty();
    mPot = 0;
    mOnePot = true;
}

void GameState::chargeAnte()
{
    uint8_t i = mFirstAlive;
    do {
        mStakes[i] -= mAnte;
    } while (nextAlive(i) != mFirstAlive);
    mPot += mNAlive * mAnte;
}

void GameState::chargeBlinds()
{
    mCurrentActing = mFirstActing;
    uint8_t sbPlayer, bbPlayer;
    // Heads-up
    if (mNAlive == 2) {
        bbPlayer = mCurrentActing;
        sbPlayer = nextActing(mCurrentActing);
    }
    else {
        sbPlayer = mCurrentActing;
        bbPlayer = nextActing(mCurrentActing);
        nextActing(mCurrentActing);
    }

    mBets[sbPlayer] += mSB;
    mPot += mSB;
    mBets[bbPlayer] += mBB;
    mPot += mBB;

    mToCall = mBB;
    mLargestRaise = mBB;
}

void GameState::dealHoleCards(uint64_t& usedCardsMask)
{
    uint8_t i = mFirstAlive;
    do {
        dealCards(mHands[i], omp::HOLE_CARDS, usedCardsMask);
    } while (nextAlive(i) != mFirstAlive);
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
    mActed[mCurrentActing] = true;

    // Current player folds.
    if (!bet && mBets[mCurrentActing] != mToCall) {
        eraseAlive(mCurrentActing);
        eraseActing(mCurrentActing);
    }

    // Current player calls or raises.
    else {
        mStakes[mCurrentActing] -= bet;
        mBets[mCurrentActing] += bet;
        mPot += bet;

        uint32_t raise = mBets[mCurrentActing] - mToCall;

        // If someone has gone all-in before and the bet is not a call,
        // it means we have to use side pots.
        if (mAllInFlag && raise)
            mOnePot = false;

        // Raise
        if (raise >= mLargestRaise) {
            mToCall = mBets[mCurrentActing];
            mLargestRaise = raise;
        }
        // All-in not full raise
        else if (!mStakes[mCurrentActing]) {
            // Incomplete call
            if (raise < 0) {
                mOnePot = false;
            }
            // Incomplete raise
            else if (raise) {
                mToCall = mBets[mCurrentActing];
            }
        }

        // Goes all-in.
        if (!mStakes[mCurrentActing]) {
            mAllInFlag = true;
            eraseActing(mCurrentActing);
        }
    }

    // Everybody folded but one.
    if (mNAlive == 1) {
        mStakes[mFirstAlive] += mPot;
        return true;
    }

    if (mNActing)
        nextActing(mCurrentActing);
    // Everybody went all-in.
    else {
        showdown();
        return true;
    }

    // End of the round (we went around the table)
    if (mActed[mCurrentActing] && mToCall == mBets[mCurrentActing]) {

        // Showdown
        if (mRound == Round::river || mNActing == 1) {
            showdown();
            return true;
        }

        // Goes to the next round.
        else {
            ++mRound;
            mLargestRaise = mBB;
            uint8_t i = mFirstActing;
            do {
                mActed[i] = false;
            } while (nextActing(i) != mFirstActing);
            mCurrentActing = mFirstActing;
            return false;
        }
    }

    return false;
}

void GameState::showdown()
{
    std::vector<std::vector<uint8_t>> rankings = getRankings();

    // One pot and one winner
    // (deals with this specific case to speed up the computation)
    if (mOnePot && rankings[0].size() == 1) {
        mStakes[rankings[0][0]] += mPot;
        return;
    }

    // One pot and multiple winners
    // (deals with this specific case to speed up the computation)
    else if (mOnePot) {
        // Distributes the gains to each winner.
        uint32_t gain = mPot / rankings[0].size();
        for (uint8_t i : rankings[0]) {
            mStakes[i] += gain;
        }
        // Remaining chips go to the first players after the dealer.
        uint8_t extra = mPot % rankings[0].size();
        for (uint8_t i = 0; i < extra; ++i) {
            ++mStakes[rankings[0][i]];
        }
        return;
    }

    // General case: multiple pots
    for (std::vector<uint8_t>& sameRankPlayers : rankings) {

        // One winner for this pot
        // (deals with this specific case to speed up the computation)
        if (sameRankPlayers.size() == 1) {
            if (!mBets[sameRankPlayers[0]])
                continue;
            // Builds the pot corresponding to the winner's bet.
            uint32_t pot = 0;
            for (uint8_t player = 0; player < opt::MAX_PLAYERS; ++player) {
                if (!mBets[player])
                    continue;
                uint32_t due = std::min(mBets[sameRankPlayers[0]], mBets[player]);
                pot += due;
                mBets[player] -= due;
            }
            mStakes[sameRankPlayers[0]] += pot;
            continue;
        }

        // Builds ordered bets of the same rank players.
        std::vector<uint32_t> orderedBets(sameRankPlayers.size());
        for (uint8_t i = 0; i < orderedBets.size(); ++i)
            orderedBets[i] = mBets[sameRankPlayers[i]];
        std::sort(orderedBets.begin(), orderedBets.end());
        // Removes duplicates.
        orderedBets.erase(
            std::unique(orderedBets.begin(), orderedBets.end()),
            orderedBets.end()
        );

        // Skips if nobody in this bracket has a gain.
        if (orderedBets.size() == 1 && !orderedBets[0])
            continue;

        // Flags for winners eligible for a gain.
        std::vector<bool> giveGain(sameRankPlayers.size(), true);
        uint8_t nWinners = sameRankPlayers.size();

        // Each non-zero winnerBet will correspond to one pot
        // for the flagged players of the current rank's bracket to share.
        for (uint32_t winnerBet : orderedBets) {
            if (!winnerBet)
                continue;
            // Unflags winners who are not eligible for the current pot.
            for (uint8_t i : sameRankPlayers) {
                if (giveGain[i] && !mBets[i]) {
                    giveGain[i] = false;
                    --nWinners;
                }
            }
            // Builds the pot corresponding to winnerBet.
            uint32_t pot = 0;
            for (uint8_t player = 0; player < opt::MAX_PLAYERS; ++player) {
                if (!mBets[player])
                    continue;
                uint32_t due = std::min(winnerBet, mBets[player]);
                pot += due;
                mBets[player] -= due;
            }
            // Distributes the gains to each winner.
            uint32_t gain = pot / nWinners;
            // Remaining chips go to the first players after the dealer
            uint8_t extra = pot % nWinners;
            for (uint8_t i : sameRankPlayers) {
                if (giveGain[i]) {
                    mStakes[i] += gain;
                    if (extra--)
                        ++mStakes[i];
                }
            }
        }
    }
    return;
}

std::vector<std::vector<uint8_t>> GameState::getRankings() const
{

    // One pot
    // (deals with this specific case to speed up the computation)
    if (mOnePot) {
        uint16_t bestRank = 0;
        std::vector<uint8_t> winners;
        uint8_t i = mFirstAlive;
        do {
            Hand hand = mBoardCards + mHands[i];
            uint16_t rank = mEval.evaluate(hand);
            if (rank > bestRank) {
                bestRank = rank;
                winners = { i };
            }
            else if (rank == bestRank) {
                winners.push_back(i);
            }
        } while (nextAlive(i) != mFirstAlive);
        return { winners };
    }

    // General case of multiple pots

    // Computes players' ranks.
    std::vector<uint16_t> ranks(mNAlive);
    std::vector<uint8_t> players(mNAlive);
    uint8_t player = mFirstAlive;
    for (uint8_t i = 0; i < mNAlive; ++i) {
        Hand hand = mBoardCards + mHands[player];
        ranks[i] = mEval.evaluate(hand);
        players[i] = player;
        nextAlive(player);
    }

    std::vector<uint8_t> range(mNAlive);
    std::iota(range.begin(), range.end(), uint8_t(0));
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

uint8_t GameState::actingPlayer() const
{
    return mCurrentActing;
}

bool GameState::notFacingFullRaise() const
{
    return call() && call() < mLargestRaise;
}

uint32_t GameState::stake() const
{
    return mStakes[mCurrentActing];
}

uint32_t GameState::call() const
{
    return mToCall - mBets[mCurrentActing];
}

uint32_t GameState::minRaise() const
{
    return call() + mLargestRaise;
}

} // egn