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
    round = Round::preflop;
    finished = false;
    mAllInFlag = false;

    resetPlayers();
    resetBoard();

    // Charge antes and blinds.
    finished = chargeAnte();
    if (!finished)
        finished = chargeBlinds();
    if (finished)
        setRewards();

    // Deal cards.
    if (dealRandomCards) {
        uint64_t usedCardsMask = 0;
        dealHoleCards(usedCardsMask);
        dealBoardCards(usedCardsMask);
    }
}

void GameState::resetPlayers()
{
    mInitialStakes = stakes;
    mNAlive = 0;
    mNActing = 0;
    // The first acting player after the preflop is always
    // the player following the dealer.
    uint8_t firstPlayer = (mDealer + 1) % opt::MAX_PLAYERS;
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

void GameState::resetBoard()
{
    mBoardCards = Hand::empty();
    mPot = 0;
    mOnePot = true;
}

void GameState::dealHoleCards(uint64_t& usedCardsMask)
{
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

bool GameState::chargeAnte()
{
    uint8_t i = mFirstActing;
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
                showdown();
                return true;
            }
        }
        else {
            mPot += mAnte;
            mBets[i] += mAnte;
            stakes[i] -= mAnte;
        }
    } while (nextActing(i) != mFirstActing);

    // Only one player did not went all-in.
    if (mNActing == 1) {
        showdown();
        return true;
    }

    return false;
}

bool GameState::chargeBlinds()
{
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

    // Less than 2 players did not go all-in.
    if (mNActing <= 1) {
        showdown();
        return true;
    }

    // Even if the bb was incomplete, the amount
    // to call is still the bb.
    mToCall = mAnte + mBB;
    mLargestRaise = mBB;

    setLegalActions();
    return false;
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

void GameState::nextState(chips bet)
{
    mActed[mCurrentActing] = true;

    // Current player checks or folds.
    if (!bet) {
        // Current player folds.
        if (mBets[mCurrentActing] != mToCall) {
            eraseAlive(mCurrentActing);
            eraseActing(mCurrentActing);
        }
    }

    // Current player calls or raises.
    else {
        mPot += bet;
        mBets[mCurrentActing] += bet;
        stakes[mCurrentActing] -= bet;

        dchips raise = dchips(mBets[mCurrentActing]) - dchips(mToCall);

        // If someone has gone all-in before and the bet is not a call,
        // it means we have to use side pots.
        if (mAllInFlag && raise)
            mOnePot = false;

        // Raise
        if (raise >= dchips(mLargestRaise)) {
            mToCall = mBets[mCurrentActing];
            mLargestRaise = raise;
        }
        // All-in not full raise
        else if (!stakes[mCurrentActing]) {
            // Incomplete call
            if (raise < 0) {
                mOnePot = false;
            }
            // Incomplete raise
            else if (raise) {
                mToCall = mBets[mCurrentActing];
            }
        }

        // Go all-in.
        if (!stakes[mCurrentActing]) {
            mAllInFlag = true;
            eraseActing(mCurrentActing);
        }
    }

    // Everybody folded but one.
    if (mNAlive == 1) {
        stakes[mFirstAlive] += mPot;
        finished = true;
        setRewards();
        return;
    }

    if (mNActing)
        nextActing(mCurrentActing);
    // Everybody went all-in.
    else {
        showdown();
        finished = true;
        setRewards();
        return;
    }

    // End of the round (we went around the table)
    if (mActed[mCurrentActing] && mBets[mCurrentActing] == mToCall) {

        // Showdown
        if (round == Round::river || mNActing == 1) {
            showdown();
            finished = true;
            setRewards();
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
    actingPlayer = mCurrentActing;

    call = mToCall - mBets[mCurrentActing];
    minRaise = call + mLargestRaise;
    allin = stakes[mCurrentActing];

    // Determine legal actions.
    if (allin <= call) {
        actions[0] = Action::fold;
        actions[1] = Action::allin;
        nActions = 2;
    }
    // Not facing a full raise.
    else if (call && call < mLargestRaise) {
        actions[0] = Action::fold;
        actions[1] = Action::call;
        nActions = 2;
    }
    else if (allin <= minRaise) {
        if (call) {
            actions[0] = Action::fold;
            actions[1] = Action::call;
            actions[2] = Action::allin;
            nActions = 3;
        }
        else {
            actions[0] = Action::call;
            actions[1] = Action::allin;
            nActions = 2;
        }
    }
    else {
        if (call) {
            actions[0] = Action::fold;
            actions[1] = Action::call;
            actions[2] = Action::raise;
            nActions = 3;
        }
        else {
            actions[0] = Action::call;
            actions[1] = Action::raise;
            nActions = 2;
        }
    }
}

void GameState::showdown()
{
    std::vector<std::vector<uint8_t>> rankings = getRankings();

    // One pot and one winner
    // (deal with this specific case to speed up the computation)
    if (mOnePot && rankings[0].size() == 1) {
        stakes[rankings[0][0]] += mPot;
        return;
    }

    // One pot and multiple winners
    // (deal with this specific case to speed up the computation)
    else if (mOnePot) {
        // Distribute the gains to each winner.
#pragma warning(suppress: 4267)
        chips gain = mPot / rankings[0].size();
        // Remaining chips go to the first players after the dealer.
#pragma warning(suppress: 4267)
        uint8_t extra = mPot % rankings[0].size();
        for (uint8_t i : rankings[0]) {
            stakes[i] += gain;
            if (extra) {
                ++stakes[i];
                --extra;
            }
        }
        return;
    }

    // General case: multiple pots
    for (std::vector<uint8_t>& sameRankPlayers : rankings) {

        // If the sum of all pots has been emptied, exit.
        if (!mPot)
            return;

        // One winner for this pot
        // (deal with this specific case to speed up the computation)
        if (sameRankPlayers.size() == 1) {
            if (!mBets[sameRankPlayers[0]])
                continue;
            // Build the pot corresponding to the winner's bet.
            chips pot = 0;
            for (uint8_t player = 0; player < opt::MAX_PLAYERS; ++player) {
                if (!mBets[player])
                    continue;
                chips due = std::min(mBets[sameRankPlayers[0]], mBets[player]);
                pot += due;
                mPot -= due;
                mBets[player] -= due;
            }
            stakes[sameRankPlayers[0]] += pot;
            continue;
        }

        // General case: multiple pots and multiple winners

        // Build ordered bets of players of the same current rank.
        std::vector<chips> orderedBets;
        orderedBets.reserve(sameRankPlayers.size());
        for (uint8_t i = 0; i < sameRankPlayers.size(); ++i) {
            // Skip null bets.
            if (mBets[sameRankPlayers[i]])
                orderedBets.push_back(mBets[sameRankPlayers[i]]);
        }
        // Skip if nobody in this bracket has a gain.
        if (!orderedBets.size())
            continue;
        std::sort(orderedBets.begin(), orderedBets.end());
        // Remove duplicates.
        orderedBets.erase(
            std::unique(orderedBets.begin(), orderedBets.end()),
            orderedBets.end()
        );

        // Flag for winners eligible for a gain.
        std::vector<bool> giveGain(sameRankPlayers.size(), true);
#pragma warning(suppress: 4267)
        uint8_t nWinners = sameRankPlayers.size();

        // Each winnerBet will correspond to one pot for the flagged players
        // of the current rank's bracket to share.
        for (chips winnerBet : orderedBets) {
            // Unflag winners who are not eligible for the current pot.
            for (uint8_t i = 0; i < sameRankPlayers.size(); ++i) {
                if (giveGain[i] && !mBets[sameRankPlayers[i]]) {
                    giveGain[i] = false;
                    --nWinners;
                }
            }
            // Build the pot corresponding to winnerBet.
            chips pot = 0;
            for (uint8_t player = 0; player < opt::MAX_PLAYERS; ++player) {
                if (!mBets[player])
                    continue;
                chips due = std::min(winnerBet, mBets[player]);
                pot += due;
                mPot -= due;
                mBets[player] -= due;
            }
            // Distribute the gains to each winner.
            chips gain = pot / nWinners;
            // Remaining chips go to the first players after the dealer
            uint8_t extra = pot % nWinners;
            for (uint8_t i = 0; i < sameRankPlayers.size(); ++i) {
                if (giveGain[i]) {
                    stakes[sameRankPlayers[i]] += gain;
                    if (extra) {
                        ++stakes[sameRankPlayers[i]];
                        --extra;
                    }
                }
            }
        }
    }
    return;
}

std::vector<std::vector<uint8_t>> GameState::getRankings() const
{
    // One pot
    // (deal with this specific case to speed up the computation)
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

    // Compute players' ranks.
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

    // Build players' rankings.
    std::vector<std::vector<uint8_t>> rankings{ { players[range[0]] } };
    for (uint8_t i = 1; i < mNAlive; ++i) {
        if (ranks[range[i]] != ranks[range[i - 1]])
            rankings.emplace_back(std::vector<uint8_t>{ players[range[i]] });
        else
            rankings.back().push_back(players[range[i]]);
    }

    return rankings;
}

void GameState::setRewards()
{
    for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i)
        rewards[i] = dchips(stakes[i]) - dchips(mInitialStakes[i]);
}

} // egn