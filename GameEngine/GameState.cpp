#include "GameState.h"
#include <random>
#include <numeric>

namespace egn {

#pragma warning(suppress: 26495)
GameState::GameState(
    chips ante, chips bigBlind,
    const std::array<chips, opt::MAX_PLAYERS>& stakes,
    unsigned rngSeed) :

    stakes(stakes),
    mRng{ (!rngSeed) ? std::random_device{}() : rngSeed },
    mCardDist(0, omp::CARD_COUNT - 1)
{
    setAnte(ante);
    setBigBlind(bigBlind);
}

void GameState::setAnte(chips ante)
{
    mAnte = ante;
}

void GameState::setBigBlind(chips bigBlind)
{
    mBB = bigBlind;
    mSB = mBB / 2;
}

void GameState::startNewHand(uint8_t dealerIdx, bool dealRandomCards)
{
    mDealer = dealerIdx;
    round = PREFLOP;
    finished = false;
    mPot = 0;
    resetPlayers();

    // Deal cards.
    if (dealRandomCards) {
        uint64_t usedCardsMask = 0;
        dealHoleCards(usedCardsMask);
        dealBoardCards(usedCardsMask);
    }

    // Charge antes and blinds.
    chargeAnte();
    if (finished) return;
    chargeBlinds();
}

void GameState::resetPlayers()
{
    mInitialStakes = stakes;
    mNAlive = 0;
    mNActing = 0;
    // The first acting player after the preflop is always
    // the player following the dealer.
    const uint8_t firstPlayer = (mDealer + 1) % opt::MAX_PLAYERS;
    uint8_t i = firstPlayer;
    do {
        // Ignore inactive players.
        if (stakes[i]) {
            mBets[i] = 0;
            mAlive[i] = true;
            ++mNAlive;
            mActing[i] = true;
            ++mNActing;
            mActed[i] = false;
        }
        else {
            mAlive[i] = false;
        }
        (++i) %= opt::MAX_PLAYERS;
    } while (i != firstPlayer);
    mFirstAlive = mDealer;
    nextAlive(mFirstAlive);
    mFirstActing = mFirstAlive;
}

void GameState::dealHoleCards(uint64_t& usedCardsMask)
{
    ZoneScoped;
    uint8_t i = mFirstAlive;
    do {
        std::array<uint8_t, omp::HOLE_CARDS> hand;
        for (unsigned j = 0; j < omp::HOLE_CARDS; ++j) {
            uint8_t card;
            uint64_t cardMask;
            do {
                card = mCardDist(mRng);
                cardMask = 1ull << card;
            } while (usedCardsMask & cardMask);
            usedCardsMask |= cardMask;
            hand[j] = card;
        }
        mHands[i] = Hand(hand);
    } while (nextAlive(i) != mFirstAlive);
}

void GameState::dealBoardCards(uint64_t& usedCardsMask)
{
    ZoneScoped;
    mBoardCards = Hand::empty();
    for (unsigned i = 0; i < omp::BOARD_CARDS; ++i) {
        unsigned card;
        uint64_t cardMask;
        do {
            card = mCardDist(mRng);
            cardMask = 1ull << card;
        } while (usedCardsMask & cardMask);
        usedCardsMask |= cardMask;
        mBoardCards += card;
    }
}

void GameState::setHoleCards(uint8_t player, const Hand& hand)
{
    mHands[player] = hand;
}

void GameState::setBoardCards(const Hand& boardCards)
{
    mBoardCards = boardCards;
}

void GameState::chargeAnte()
{
    ZoneScoped;
    uint8_t i = mFirstAlive;
    do {
        // The player must all-in on the ante.
        if (stakes[i] <= mAnte) {
            mPot += stakes[i];
            mBets[i] += stakes[i];
            stakes[i] = 0;
            eraseActing(i);
            // Everybody went all-in.
            // We deal with this case here to
            // avoid infinite loop.
            if (!mNActing) {
                endGame();
                return;
            }
        }
        else {
            mPot += mAnte;
            mBets[i] += mAnte;
            stakes[i] -= mAnte;
        }
    } while (nextAlive(i) != mFirstAlive);

    // Only one player did not went all-in on the ante.
    if (mNActing == 1)
        endGame();
}

void GameState::chargeBlinds()
{
    ZoneScoped;
    // Find out the sb and bb players.
    mCurrentActing = mFirstAlive;
    uint8_t sbPlayer, bbPlayer;
    // Heads-up
    if (mNAlive == 2) {
        bbPlayer = mCurrentActing;
        sbPlayer = nextAlive(mCurrentActing);
    }
    else {
        sbPlayer = mCurrentActing;
        bbPlayer = nextAlive(mCurrentActing);
        nextActing(mCurrentActing);
    }

    // Charge the sb.
    // Verify that the sb did not all-in on the ante.
    if (mActing[sbPlayer]) {
        // The player must all-in on the sb.
        if (stakes[sbPlayer] <= mSB) {
            mPot += stakes[sbPlayer];
            mBets[sbPlayer] += stakes[sbPlayer];
            stakes[sbPlayer] = 0;
            eraseActing(sbPlayer);
        }
        else {
            mPot += mSB;
            mBets[sbPlayer] += mSB;
            stakes[sbPlayer] -= mSB;
        }
    }

    // Charge the bb.
    // Verify that the bb did not all-in on the ante.
    if (mActing[bbPlayer]) {
        // The player must all-in on the bb.
        if (stakes[bbPlayer] <= mBB) {
            mPot += stakes[bbPlayer];
            mBets[bbPlayer] += stakes[bbPlayer];
            stakes[bbPlayer] = 0;
            eraseActing(bbPlayer);
        }
        else {
            mPot += mBB;
            mBets[bbPlayer] += mBB;
            stakes[bbPlayer] -= mBB;
        }
    }

    // Everybody went all-in.
    if (!mNActing) {
        endGame();
        return;
    }

    // Even if the bb was incomplete, the amount
    // to call is still the bb.
    mToCall = mAnte + mBB;
    mLargestRaise = mBB;
    mMaxBet = std::max(std::max(mBets[sbPlayer], mBets[bbPlayer]), mAnte);

    // Everybody went all-in but one.
    if (mNActing == 1 && mBets[mFirstActing] == mMaxBet) {
        endGame();
        return;
    }

    setLegalActions();
}

uint8_t& GameState::nextActive(uint8_t& i) const
{
    do {
        (++i) %= opt::MAX_PLAYERS;
    } while (!stakes[i]);
    return i;
}

// mNAlive must be non-zero to avoid infinite loop.
uint8_t& GameState::nextAlive(uint8_t& i) const
{
    do {
        (++i) %= opt::MAX_PLAYERS;
    } while (!mAlive[i]);
    return i;
}

// mNActing must be non-zero to avoid infinite loop.
uint8_t& GameState::nextActing(uint8_t& i) const
{
    do {
        (++i) %= opt::MAX_PLAYERS;
    } while (!mActing[i]);
    return i;
}

void GameState::eraseAlive(uint8_t& i)
{
    mAlive[i] = false;
    --mNAlive;
    if (i == mFirstAlive)
        nextAlive(mFirstAlive);
}

void GameState::eraseActing(uint8_t& i)
{
    mActing[i] = false;
    --mNActing;
    // If there is no acting player left,
    // we leave mFirstActing as it is.
    if (i == mFirstActing && mNActing)
        nextActing(mFirstActing);
}

void GameState::nextState()
{
    ZoneScoped;
    switch (action) {

    case FOLD:
        eraseAlive(mCurrentActing);
        eraseActing(mCurrentActing);
        break;

    case CALL:
        if (call) {
            mPot += call;
            mBets[mCurrentActing] += call;
            stakes[mCurrentActing] -= call;
            // All-in
            if (!stakes[mCurrentActing])
                eraseActing(mCurrentActing);
            else
                // mMaxBet might have to be updated if there is only
                // one acting player remaining and the legal call is
                // larger than the posted all-ins.
                mMaxBet = mBets[mCurrentActing];
        }
        break;

    case RAISE:
        mPot += bet;
        mBets[mCurrentActing] += bet;
        stakes[mCurrentActing] -= bet;

        mLargestRaise = std::max(mBets[mCurrentActing] - mToCall, mLargestRaise);
        mToCall = mBets[mCurrentActing];
        mMaxBet = mToCall;
        // All-in
        if (!stakes[mCurrentActing])
            eraseActing(mCurrentActing);
        break;

    default:
        throw std::runtime_error("Unknown action.");
    }

    // Everybody folded but one.
    if (mNAlive == 1) {
        stakes[mFirstAlive] += mPot;
        finished = true;
        setRewards();
        return;
    }

    // Everybody went all-in.
    if (!mNActing) {
        endGame();
        return;
    }

    mActed[mCurrentActing] = true;
    nextActing(mCurrentActing);

    // Everybody folded but one.
    if (mNActing == 1 && mBets[mCurrentActing] == mMaxBet) {
        endGame();
        return;
    }

    // End of the round (we went around the table)
    if (mActed[mCurrentActing] && mBets[mCurrentActing] == mMaxBet) {

        // Showdown
        if (round == RIVER) {
            endGame();
            return;
        }

        // Go to the next round.
        else {
            ++round;
            mLargestRaise = mBB;
            uint8_t i = mFirstActing;
            do {
                mActed[i] = false;
            } while (nextActing(i) != mFirstActing);
            mCurrentActing = mFirstActing;
        }
    }
    setLegalActions();
}

void GameState::setLegalActions()
{
    ZoneScoped;
    actingPlayer = mCurrentActing;

    chips legalCall = mToCall - mBets[mCurrentActing];
    call = std::min(legalCall, stakes[mCurrentActing]);
    minRaise = legalCall + mLargestRaise;
    allin = stakes[mCurrentActing];
    
    // Stake less than a complete call
    // or not facing a full raise.
    if (stakes[mCurrentActing] <= legalCall
        || (mActed[mCurrentActing]
            && legalCall && legalCall < mLargestRaise)) {
        actions[0] = FOLD;
        actions[1] = CALL;
        nActions = 2;
        legalCase = 0;
    }
    else if (!legalCall) {
        actions[0] = CALL;
        actions[1] = RAISE;
        nActions = 2;
        legalCase = 1;
    }
    else {
        actions[0] = FOLD;
        actions[1] = CALL;
        actions[2] = RAISE;
        nActions = 3;
        legalCase = 2;
    }
}

void GameState::endGame()
{
    showdown();
    finished = true;
    setRewards();
}

#pragma warning(push)
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
void GameState::showdown()
{
    ZoneScoped;
    bool onePot = onePotUsed();
    setRankings(onePot);

    // One pot and one winner
    // (deal with this specific case to speed up the computation)
    if (onePot && mCumNSameRanks[1] == 1) {
        stakes[mRankings[0]] += mPot;
        return;
    }

    // One pot and multiple winners
    // (deal with this specific case to speed up the computation)
    else if (onePot) {
        // Distribute the gains to each winner.
        chips gain = mPot / mCumNSameRanks[1];
        // Remaining chips go to the first players after the dealer.
        uint8_t extra = mPot % mCumNSameRanks[1];
        for (uint8_t i = 0; i < mCumNSameRanks[1]; ++i) {
            stakes[mRankings[i]] += gain;
            if (extra) {
                ++stakes[mRankings[i]];
                --extra;
            }
        }
        return;
    }

    // General case: multiple pots
    for (uint8_t k = 1; k < mNRanks + 1; ++k) {

        // If the sum of all pots has been emptied, exit.
        if (!mPot)
            return;

        uint8_t nSameRank = mCumNSameRanks[k] - mCumNSameRanks[k - 1];

        // One winner for this pot
        // (deal with this specific case to speed up the computation)
        if (nSameRank == 1) {
            uint8_t winner = mRankings[mCumNSameRanks[k - 1]];
            chips winnerBet = mBets[winner];
            if (!winnerBet)
                continue;
            // Build the pot corresponding to the winner's bet.
            chips pot = 0;
            for (uint8_t player = 0; player < opt::MAX_PLAYERS; ++player) {
                if (!mBets[player])
                    continue;
                chips due = std::min(winnerBet, mBets[player]);
                pot += due;
                mPot -= due;
                mBets[player] -= due;
            }
            stakes[winner] += pot;
            continue;
        }

        // General case: multiple pots and multiple winners

        // Build sorted bets of players of the same current rank.
        uint8_t nBets = 0;
        for (uint8_t i = mCumNSameRanks[k - 1]; i < mCumNSameRanks[k]; ++i) {
            // Skip null bets.
            if (mBets[mRankings[i]])
                mSortedBets[nBets++] = mBets[mRankings[i]];
        }
        // Skip if nobody in this bracket has a gain.
        if (!nBets) continue;
        std::sort(mSortedBets.begin(), mSortedBets.begin() + nBets);
        // Remove duplicates.
        nBets = std::unique(mSortedBets.begin(), mSortedBets.begin() + nBets)
            - mSortedBets.begin();
        // Build adjacent differences of sorted bets.
        std::adjacent_difference(
            mSortedBets.begin(), mSortedBets.begin() + nBets, mSortedBets.begin());

        // Flags for winners eligible for a gain.
        for (uint8_t i = mCumNSameRanks[k - 1]; i < mCumNSameRanks[k]; ++i)
            mGiveGain[mRankings[i]] = true;
        uint8_t nWinners = nSameRank;

        // Each winnerBet will correspond to one pot for the flagged players
        // of the current rank's bracket to share.
        for (uint8_t b = 0; b < nBets; ++b) {
            // Unflag winners who are not eligible for the current pot.
            for (uint8_t i = mCumNSameRanks[k - 1]; i < mCumNSameRanks[k]; ++i) {
                if (mGiveGain[mRankings[i]] && !mBets[mRankings[i]]) {
                    mGiveGain[mRankings[i]] = false;
                    --nWinners;
                }
            }
            // Build the pot corresponding to winnerBet.
            chips pot = 0;
            for (uint8_t player = 0; player < opt::MAX_PLAYERS; ++player) {
                if (!mBets[player])
                    continue;
                chips due = std::min(mSortedBets[b], mBets[player]);
                pot += due;
                mPot -= due;
                mBets[player] -= due;
            }
            // Distribute the gains to each winner.
            chips gain = pot / nWinners;
            // Remaining chips go to the first players after the dealer
            uint8_t extra = pot % nWinners;
            for (uint8_t i = mCumNSameRanks[k - 1]; i < mCumNSameRanks[k]; ++i) {
                if (mGiveGain[mRankings[i]]) {
                    stakes[mRankings[i]] += gain;
                    if (extra) {
                        ++stakes[mRankings[i]];
                        --extra;
                    }
                }
            }
        }
    }
}
#pragma warning(pop)

bool GameState::onePotUsed() const
{
    chips prevBet = mBets[mFirstAlive];
    uint8_t i = mFirstAlive;
    nextAlive(i);
    do {
        if (mBets[i] != prevBet) return false;
    } while (nextAlive(i) != mFirstAlive);
    return true;
}

void GameState::setRankings(bool onePot)
{
    ZoneScoped;
    // One pot
    // (deal with this specific case to speed up the computation)
    if (onePot) {
        mNRanks = 1;
        uint16_t bestRank = 0;
        uint8_t i = mFirstAlive;
        do {
            Hand hand = mBoardCards + mHands[i];
            uint16_t rank = mEval.evaluate(hand);
            if (rank > bestRank) {
                bestRank = rank;
                mRankings[0] = i;
                mCumNSameRanks[1] = 1;
            }
            else if (rank == bestRank)
                mRankings[mCumNSameRanks[1]++] = i;
        } while (nextAlive(i) != mFirstAlive);
        return;
    }

    // General case of multiple pots

    // Compute players' ranks.
    uint8_t player = mFirstAlive;
    for (uint8_t i = 0; i < mNAlive; ++i) {
        Hand hand = mBoardCards + mHands[player];
        mRanks[player] = mEval.evaluate(hand);
        mRankings[i] = player;
        nextAlive(player);
    }

    // stable_sort is needed to preserve the order of the players
    // with the same rank for the distribution of the extras
    // in clockwise order.
    std::stable_sort(
        mRankings.begin(), mRankings.begin() + mNAlive,
        [&](uint8_t i, uint8_t j) { return mRanks[i] > mRanks[j]; }
    );

    // Build mCumNSameRanks.
    mNRanks = 1;
    mCumNSameRanks[1] = 1;
    for (uint8_t i = 1; i < mNAlive; ++i) {
        if (mRanks[mRankings[i]] != mRanks[mRankings[i - 1]]) {
#pragma warning(suppress: 26451)
            mCumNSameRanks[mNRanks + 1] = mCumNSameRanks[mNRanks] + 1;
            ++mNRanks;
        }
        else
            ++mCumNSameRanks[mNRanks];
    }
}

void GameState::setRewards()
{
    for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i)
        rewards[i] = dchips(stakes[i]) - dchips(mInitialStakes[i]);
}

} // egn