#include "GameState.h"
#include <random>
#include <numeric>

namespace egn {

std::array<std::array<uint8_t, opt::MAX_PLAYERS>,
    GameState::NEXT_LOOKUP_SIZE> GameState::NEXT_LOOKUP{};

#pragma warning(suppress: 26495)
GameState::GameState(
    chips ante, chips bigBlind,
    const std::array<chips, opt::MAX_PLAYERS>& stakes,
    unsigned rngSeed) :

    stakes(stakes),
    mRng{ (!rngSeed) ? std::random_device{}() : rngSeed },
    mCardDist(0, omp::CARD_COUNT - 1)
{
    // Do one time initialization of static data NEXT_LOOKUP.
    static bool initVar = (populateNextLookup(), true);
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
    if (!finished) chargeBlinds();
}

void GameState::resetPlayers()
{
    mInitialStakes = stakes;

    mAlive = 0;
    mActing = 0;
    mNAlive = 0;
    mNActing = 0;
    for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i) {
        // Ignore inactive players.
        if (stakes[i]) {
            addAlive(i);
            addActing(i);
            mBets[i] = 0;
            mActed[i] = false;
        }
    }

    // The first acting player after the preflop is always
    // the player following the dealer.
    mFirstAlive = mDealer;
    mFirstActing = nextAlive(mFirstAlive);
}

void GameState::dealHoleCards(uint64_t& usedCardsMask)
{
    //ZoneScoped;
    uint8_t i = mFirstAlive;
    do {
        for (uint8_t j = 0; j < omp::HOLE_CARDS; ++j) {
            uint8_t card;
            uint64_t cardMask;
            do {
                card = mCardDist(mRng);
                cardMask = 1ull << card;
            } while (usedCardsMask & cardMask);
            usedCardsMask |= cardMask;
            hands[i][j] = card;
        }
    } while (nextAlive(i) != mFirstAlive);
}

void GameState::dealBoardCards(uint64_t& usedCardsMask)
{
    //ZoneScoped;
    for (uint8_t i = 0; i < omp::BOARD_CARDS; ++i) {
        uint8_t card;
        uint64_t cardMask;
        do {
            card = mCardDist(mRng);
            cardMask = 1ull << card;
        } while (usedCardsMask & cardMask);
        usedCardsMask |= cardMask;
        boardCards[i] = card;
    }
}

void GameState::setHoleCards(uint8_t player, const Hand& hand)
{
    hands[player] = hand.getArr<omp::HOLE_CARDS>();
}

void GameState::setBoardCards(const Hand& boardCards0)
{
    boardCards = boardCards0.getArr<omp::BOARD_CARDS>();
}

void GameState::chargeAnte()
{
    //ZoneScoped;
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
    if (mNActing == 1) endGame();
}

void GameState::chargeBlinds()
{
    //ZoneScoped;
    // Find out the sb and bb players.
    actingPlayer = mFirstAlive;
    uint8_t sbPlayer, bbPlayer;
    // Heads-up
    if (mNAlive == 2) {
        bbPlayer = actingPlayer;
        sbPlayer = nextAlive(actingPlayer);
    }
    else {
        sbPlayer = actingPlayer;
        bbPlayer = nextAlive(actingPlayer);
        nextActing(actingPlayer);
    }

    // Charge the sb.
    // Verify that the sb did not all-in on the ante.
    if (isActing(sbPlayer)) {
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
    if (isActing(bbPlayer)) {
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

void GameState::populateNextLookup()
{
    for (uint16_t playerMask = 1; playerMask < NEXT_LOOKUP_SIZE; ++playerMask) {
        for (uint8_t current = 0; current < opt::MAX_PLAYERS; ++current) {
            uint8_t next = current;
            do {
                (++next) %= opt::MAX_PLAYERS;
            } while (!(opt::checkBit(playerMask, next)));
#pragma warning(suppress: 28020)
            NEXT_LOOKUP[playerMask][current] = next;
        }
    }
}

bool GameState::isAlive(uint8_t i) const
{
    return opt::checkBit(mAlive, i);
}

bool GameState::isActing(uint8_t i) const
{
    return opt::checkBit(mActing, i);
}

uint8_t& GameState::nextActive(uint8_t& i) const
{
    do {
        (++i) %= opt::MAX_PLAYERS;
    } while (!stakes[i]);
    return i;
}

uint8_t& GameState::nextAlive(uint8_t& i) const
{
    i = NEXT_LOOKUP[mAlive][i];
    return i;
}

uint8_t& GameState::nextActing(uint8_t& i) const
{
    i = NEXT_LOOKUP[mActing][i];
    return i;
}

void GameState::addAlive(uint8_t i)
{
    opt::setBit(mAlive, i);
    ++mNAlive;
}

void GameState::addActing(uint8_t i)
{
    opt::setBit(mActing, i);
    ++mNActing;
}

void GameState::eraseAlive(uint8_t i)
{
    opt::toggleBit(mAlive, i);
    --mNAlive;
    if (i == mFirstAlive)
        nextAlive(mFirstAlive);
}

void GameState::eraseActing(uint8_t i)
{
    opt::toggleBit(mActing, i);
    --mNActing;
    if (i == mFirstActing)
        nextActing(mFirstActing);
}

void GameState::nextState()
{
    //ZoneScoped;
    switch (action) {

    case FOLD:
        eraseAlive(actingPlayer);
        eraseActing(actingPlayer);
        break;

    case CALL:
        if (call) {
            mPot += call;
            mBets[actingPlayer] += call;
            stakes[actingPlayer] -= call;
            // All-in
            if (!stakes[actingPlayer])
                eraseActing(actingPlayer);
            else
                // mMaxBet must be updated if there is only
                // one acting player remaining and the legal call is
                // larger than the posted all-ins.
                mMaxBet = mBets[actingPlayer];
        }
        break;

    case RAISE:
        mPot += bet;
        mBets[actingPlayer] += bet;
        stakes[actingPlayer] -= bet;

        mLargestRaise = std::max(mBets[actingPlayer] - mToCall, mLargestRaise);
        mToCall = mBets[actingPlayer];
        mMaxBet = mToCall;
        // All-in
        if (!stakes[actingPlayer])
            eraseActing(actingPlayer);
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

    mActed[actingPlayer] = true;
    nextActing(actingPlayer);

    // Everybody folded but one.
    if (mNActing == 1 && mBets[actingPlayer] == mMaxBet) {
        endGame();
        return;
    }

    // End of the round (we went around the table)
    if (mActed[actingPlayer] && mBets[actingPlayer] == mMaxBet) {

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
            actingPlayer = mFirstActing;
        }
    }
    setLegalActions();
}

void GameState::setLegalActions()
{
    //ZoneScoped;
    chips legalCall = mToCall - mBets[actingPlayer];
    call = std::min(legalCall, stakes[actingPlayer]);
    minRaise = legalCall + mLargestRaise;
    allin = stakes[actingPlayer];
    
    // Stake less than a complete call
    // or not facing a full raise.
    if (stakes[actingPlayer] <= legalCall
        || (mActed[actingPlayer]
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
    //ZoneScoped;
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
    //ZoneScoped;
    // One pot
    // (deal with this specific case to speed up the computation)
    if (onePot) {
        mNRanks = 1;
        uint16_t bestRank = 0;
        uint8_t i = mFirstAlive;
        do {
            omp::Hand hand = getPlayerHand(i);
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
        omp::Hand hand = getPlayerHand(player);
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

omp::Hand GameState::getPlayerHand(uint8_t i) const
{
    omp::Hand hand = omp::Hand::empty();
    hand += omp::Hand(hands[i]);
    for (uint8_t c = 0; c < omp::BOARD_CARDS; ++c)
        hand += omp::Hand(boardCards[c]);
    return hand;
}

void GameState::setRewards()
{
    for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i)
        rewards[i] = dchips(stakes[i]) - dchips(mInitialStakes[i]);
}

} // egn