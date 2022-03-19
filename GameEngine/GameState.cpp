#include "GameState.h"
#include <random>
#include <numeric>

namespace egn {

std::array<std::array<uint8_t, MAX_PLAYERS>,
    GameState::NEXT_LOOKUP_SIZE> GameState::NEXT_LOOKUP{};

GameState::GameState() : mRng(0)
{
}

#pragma warning(suppress: 26495)
GameState::GameState(
    chips ante, chips bigBlind,
    const std::array<chips, MAX_PLAYERS>& stakes,
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

void GameState::setAnte(chips ante0)
{
    ante = ante0;
}

void GameState::setBigBlind(chips bigBlind)
{
    bb = bigBlind;
    sb = bb / 2;
}

void GameState::startNewHand(uint8_t dealerIdx, bool dealRandomCards)
{
    dealer = dealerIdx;
    round = PREFLOP;
    finished = false;
    pot = 0;
    resetPlayers();

    // Deal cards.
    if (dealRandomCards) {
        resetUsedCards();
        setRandomHoleCards();
        setRandomBoardCards();
    }

    // Charge antes and blinds.
    chargeAnte();
    if (!finished) chargeBlinds();
}

void GameState::resetPlayers()
{
    initialStakes = stakes;

    mAlive = 0;
    mActing = 0;
    nAlive = 0;
    nActing = 0;
    for (uint8_t i = 0; i < MAX_PLAYERS; ++i) {
        // Ignore inactive players.
        if (stakes[i]) {
            addAlive(i);
            addActing(i);
            mActed[i] = false;
        }
        // Reset bets for all players because it might have not
        // been cleaned-up in showdown if only one pot was used
        // and the bets of all players are used in showdown for
        // multiple pots.
        bets[i] = 0;
    }

    // The first acting player after the preflop is always
    // the player following the dealer.
    firstAlive = dealer;
    firstActing = nextAlive(firstAlive);
}

void GameState::resetUsedCards()
{
    usedCardsMask = 0;
}

void GameState::setHoleCards(uint8_t player, const Hand& hand)
{
    hands[player] = hand.getArr<omp::HOLE_CARDS>();
}

void GameState::setHoleCards(uint8_t player, const uint8_t hand[])
{
    hands[player][0] = hand[0];
    hands[player][1] = hand[1];
}

void GameState::setRandomHoleCards(uint8_t player)
{
    for (uint8_t i = 0; i < omp::HOLE_CARDS; ++i) {
        uint8_t card;
        uint64_t cardMask;
        do {
            card = mCardDist(mRng);
            cardMask = 1ull << card;
        } while (usedCardsMask & cardMask);
        usedCardsMask |= cardMask;
        hands[player][i] = card;
    }
}

void GameState::setRandomHoleCards()
{
    //ZoneScoped;
    uint8_t i = firstAlive;
    do {
        setRandomHoleCards(i);
    } while (nextAlive(i) != firstAlive);
}

void GameState::setBoardCards(const Hand& boardCards0)
{
    boardCards = boardCards0.getArr<omp::BOARD_CARDS>();
}

void GameState::setBoardCards(const uint8_t boardCards0[])
{
    for (uint8_t i = 0; i < omp::BOARD_CARDS; ++i)
        boardCards[i] = boardCards0[i];
}

void GameState::setRandomBoardCards()
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

void GameState::chargeAnte()
{
    //ZoneScoped;
    if (ante == 0) return;

    uint8_t i = firstAlive;
    do {
        // The player must all-in on the ante.
        if (stakes[i] <= ante) {
            pot += stakes[i];
            bets[i] += stakes[i];
            stakes[i] = 0;
            eraseActing(i);
            // Everybody went all-in.
            // We deal with this case here to
            // avoid infinite loop.
            if (!nActing) {
                endGame();
                return;
            }
        }
        else {
            pot += ante;
            bets[i] += ante;
            stakes[i] -= ante;
        }
    } while (nextAlive(i) != firstAlive);

    // Only one player did not went all-in on the ante.
    if (nActing == 1) endGame();
}

void GameState::chargeBlinds()
{
    //ZoneScoped;
    // Find out the sb and bb players.
    actingPlayer = firstAlive;
    uint8_t sbPlayer, bbPlayer;
    // Heads-up
    if (nAlive == 2) {
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
        if (stakes[sbPlayer] <= sb) {
            pot += stakes[sbPlayer];
            bets[sbPlayer] += stakes[sbPlayer];
            stakes[sbPlayer] = 0;
            eraseActing(sbPlayer);
        }
        else {
            pot += sb;
            bets[sbPlayer] += sb;
            stakes[sbPlayer] -= sb;
        }
    }

    // Charge the bb.
    // Verify that the bb did not all-in on the ante.
    if (isActing(bbPlayer)) {
        // The player must all-in on the bb.
        if (stakes[bbPlayer] <= bb) {
            pot += stakes[bbPlayer];
            bets[bbPlayer] += stakes[bbPlayer];
            stakes[bbPlayer] = 0;
            eraseActing(bbPlayer);
        }
        else {
            pot += bb;
            bets[bbPlayer] += bb;
            stakes[bbPlayer] -= bb;
        }
    }

    // Everybody went all-in.
    if (!nActing) {
        endGame();
        return;
    }

    // Even if the bb was incomplete, the amount
    // to call is still the bb.
    mToCall = ante + bb;
    mLargestRaise = bb;
    mMaxBet = std::max(std::max(bets[sbPlayer], bets[bbPlayer]), ante);

    // Everybody went all-in but one.
    if (nActing == 1 && bets[firstActing] == mMaxBet) {
        endGame();
        return;
    }

    setLegalActions();
}

void GameState::populateNextLookup()
{
    for (uint16_t playerMask = 1; playerMask < NEXT_LOOKUP_SIZE; ++playerMask) {
        for (uint8_t current = 0; current < MAX_PLAYERS; ++current) {
            uint8_t next = current;
            do {
                (++next) %= MAX_PLAYERS;
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
        (++i) %= MAX_PLAYERS;
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
    ++nAlive;
}

void GameState::addActing(uint8_t i)
{
    opt::setBit(mActing, i);
    ++nActing;
}

void GameState::eraseAlive(uint8_t i)
{
    opt::toggleBit(mAlive, i);
    --nAlive;
    if (i == firstAlive)
        nextAlive(firstAlive);
}

void GameState::eraseActing(uint8_t i)
{
    opt::toggleBit(mActing, i);
    --nActing;
    if (i == firstActing)
        nextActing(firstActing);
}

bool GameState::foundActivePlayers() const
{
    bool foundOne = false;
    for (uint8_t i = 0; i < MAX_PLAYERS; ++i) {
        if (stakes[i]) {
            if (foundOne) return true;
            foundOne = true;
        }
    }
    return false;
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
            pot += call;
            bets[actingPlayer] += call;
            stakes[actingPlayer] -= call;
            // All-in
            if (!stakes[actingPlayer])
                eraseActing(actingPlayer);
            else
                // mMaxBet must be updated if there is only
                // one acting player remaining and the legal call is
                // larger than the posted all-ins.
                mMaxBet = bets[actingPlayer];
        }
        break;

    case RAISE:
        pot += bet;
        bets[actingPlayer] += bet;
        stakes[actingPlayer] -= bet;

        mLargestRaise = std::max(bets[actingPlayer] - mToCall, mLargestRaise);
        mToCall = bets[actingPlayer];
        mMaxBet = mToCall;
        // All-in
        if (!stakes[actingPlayer])
            eraseActing(actingPlayer);
        break;

    default:
        throw std::runtime_error("Unknown action.");
    }

    // Everybody folded but one.
    if (nAlive == 1) {
        stakes[firstAlive] += pot;
        finished = true;
        return;
    }

    // Everybody went all-in.
    if (!nActing) {
        endGame();
        return;
    }

    mActed[actingPlayer] = true;
    nextActing(actingPlayer);

    // Max bet is matched.
    if (bets[actingPlayer] == mMaxBet) {

        // Everybody went all-in but one.
        if (nActing == 1) {
            endGame();
            return;
        }

        // End of the round (we went around the table)
        if (mActed[actingPlayer]) {

            // Showdown
            if (round == RIVER) {
                endGame();
                return;
            }

            // Go to the next round.
            else {
                ++round;
                mLargestRaise = bb;
                uint8_t i = firstActing;
                do {
                    mActed[i] = false;
                } while (nextActing(i) != firstActing);
                actingPlayer = firstActing;
            }
        }
    }
    setLegalActions();
}

void GameState::setLegalActions()
{
    //ZoneScoped;
    chips legalCall = mToCall - bets[actingPlayer];
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
        stakes[mRankings[0]] += pot;
        return;
    }

    // One pot and multiple winners
    // (deal with this specific case to speed up the computation)
    else if (onePot) {
        // Distribute the gains to each winner.
        chips gain = pot / mCumNSameRanks[1];
        // Remaining chips go to the first players after the dealer.
        uint8_t extra = pot % mCumNSameRanks[1];
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
        if (!pot)
            return;

        uint8_t nSameRank = mCumNSameRanks[k] - mCumNSameRanks[k - 1];

        // One winner for this pot
        // (deal with this specific case to speed up the computation)
        if (nSameRank == 1) {
            uint8_t winner = mRankings[mCumNSameRanks[k - 1]];
            chips winnerBet = bets[winner];
            if (!winnerBet)
                continue;
            // Build the pot corresponding to the winner's bet.
            chips currentPot = 0;
            for (uint8_t player = 0; player < MAX_PLAYERS; ++player) {
                if (!bets[player])
                    continue;
                chips due = std::min(winnerBet, bets[player]);
                currentPot += due;
                pot -= due;
                bets[player] -= due;
            }
            stakes[winner] += currentPot;
            continue;
        }

        // General case: multiple pots and multiple winners

        // Build sorted bets of players of the same current rank.
        uint8_t nBets = 0;
        for (uint8_t i = mCumNSameRanks[k - 1]; i < mCumNSameRanks[k]; ++i) {
            // Skip null bets.
            if (bets[mRankings[i]])
                mSortedBets[nBets++] = bets[mRankings[i]];
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
                if (mGiveGain[mRankings[i]] && !bets[mRankings[i]]) {
                    mGiveGain[mRankings[i]] = false;
                    --nWinners;
                }
            }
            // Build the pot corresponding to winnerBet.
            chips currentPot = 0;
            for (uint8_t player = 0; player < MAX_PLAYERS; ++player) {
                if (!bets[player])
                    continue;
                chips due = std::min(mSortedBets[b], bets[player]);
                currentPot += due;
                pot -= due;
                bets[player] -= due;
            }
            // Distribute the gains to each winner.
            chips gain = currentPot / nWinners;
            // Remaining chips go to the first players after the dealer
            uint8_t extra = currentPot % nWinners;
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
    chips prevBet = bets[firstAlive];
    uint8_t i = firstAlive;
    nextAlive(i);
    do {
        if (bets[i] != prevBet) return false;
    } while (nextAlive(i) != firstAlive);
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
        uint8_t i = firstAlive;
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
        } while (nextAlive(i) != firstAlive);
        return;
    }

    // General case of multiple pots

    // Compute players' ranks.
    uint8_t player = firstAlive;
    for (uint8_t i = 0; i < nAlive; ++i) {
        omp::Hand hand = getPlayerHand(player);
        mRanks[player] = mEval.evaluate(hand);
        mRankings[i] = player;
        nextAlive(player);
    }

    // stable_sort is needed to preserve the order of the players
    // with the same rank for the distribution of the extras
    // in clockwise order.
    std::stable_sort(
        mRankings.begin(), mRankings.begin() + nAlive,
        [&](uint8_t i, uint8_t j) { return mRanks[i] > mRanks[j]; }
    );

    // Build mCumNSameRanks.
    mNRanks = 1;
    mCumNSameRanks[1] = 1;
    for (uint8_t i = 1; i < nAlive; ++i) {
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

dchips GameState::reward(uint8_t i) const
{
    return dchips(stakes[i]) - dchips(initialStakes[i]);
}

void GameState::saveRng(std::fstream& file) const
{
    mRng.save(file);
    mCardDist.save(file);
}

void GameState::loadRng(std::fstream& file)
{
    mRng.load(file);
    mCardDist.load(file);
}

} // egn