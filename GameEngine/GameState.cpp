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

void GameState::resetPlayers()
{
    mPlayers.clear();
    // The first acting player after the preflop is always
    // the player following the dealer.
    for (uint8_t i = mDealer + 1, j; i < mDealer + opt::MAX_PLAYERS + 1; ++i) {
        j = i % opt::MAX_PLAYERS;
        // Ignores inactive players.
        if (!mStakes[j])
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
    mPot = 0;
    mOnePot = true;
}

void GameState::chargeAnte()
{
    for (uint8_t i : mPlayers)
        mStakes[i] -= mAnte;
    mPot += mNPlayers * mAnte;
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
    mPot += mSB;
    mBets[bbPlayer] += mBB;
    mPot += mBB;

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
    if (!bet) {
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
        mPot += bet;

        // If someone has gone all-in before and the bet is not a call,
        // it means we have to use side pots.
        if (mAllInFlag && mBets[*mCurrentPlayer] != mLastRaise)
            mOnePot = false;
        else if (!mStakes[*mCurrentPlayer])
            mAllInFlag = true;

        // Raise
        if (mBets[*mCurrentPlayer] >= mMinRaise) {
            mMinRaise = 2 * mBets[*mCurrentPlayer] - mLastRaise;
            mLastRaise = mBets[*mCurrentPlayer];
            mInitiator = *mCurrentPlayer;
        }
        // All-in not full raise
        else if (!mStakes[*mCurrentPlayer]) {
            // Incomplete call
            if (mBets[*mCurrentPlayer] < mLastRaise) {
                mOnePot = false;
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
            mStakes[*mCurrentPlayer] += mPot;
            return true;
        }

        // Showdown
        else if (mRound == Round::river) {
            showdown();
            return true;
        }

        // Goes to the next round.
        else {
            mMinRaise += mBB;
            ++mRound;
            mCurrentPlayer = mPlayers.begin();
            mInitiator = *mCurrentPlayer;
            return false;
        }
    }

    return false;
}

void GameState::goNextPlayer()
{
    // Skips players who went all-in except if it is the initiator
    // so that the round can end.
    do {
        if (++mCurrentPlayer == mPlayers.end())
            mCurrentPlayer = mPlayers.begin();
    } while (!mStakes[*mCurrentPlayer] && *mCurrentPlayer != mInitiator);
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
        for (uint8_t player : mPlayers) {
            Hand hand = mBoardCards + mPlayerHands[player];
            uint16_t rank = mEval.evaluate(hand);
            if (rank > bestRank) {
                bestRank = rank;
                winners = { player };
            }
            else if (rank == bestRank) {
                winners.push_back(player);
            }
        }
        return { winners };
    }

    // General case of multiple pots

    // Computes players' ranks.
    std::vector<uint16_t> ranks(mNPlayers);
    std::vector<uint8_t> players(mNPlayers);
    uint8_t i = 0;
    for (uint8_t player : mPlayers) {
        Hand hand = mBoardCards + mPlayerHands[player];
        ranks[i] = mEval.evaluate(hand);
        players[i] = player;
        ++i;
    }

    std::vector<uint8_t> range(mNPlayers);
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