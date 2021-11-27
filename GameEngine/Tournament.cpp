#include "Tournament.h"

namespace egn {

#pragma warning(suppress: 26495)
Tournament::Tournament(
	uint32_t ante, uint32_t bigBlind,
	const std::array<uint32_t, opt::MAX_PLAYERS>& stakes,
	const std::array<Player, opt::MAX_PLAYERS>& players,
	uint8_t dealerIdx,
	unsigned rngSeed) :

	mState(ante, bigBlind, stakes, rngSeed),
	mPlayers(players),
	mDealer(dealerIdx)
{
	initActive();
}

void Tournament::initActive()
{
	// Current active player.
	// We assume that the initial dealer that was given
	// is always active.
	uint8_t current = mDealer;
	// Next active player.
	uint8_t next = mDealer + 1;
	mNActive = 1;
	do {
		// If the next player is active, add him.
		if (mState.stakes[next]) {
			mNextActive[current] = next;
			mPrevActive[next] = current;
			++mNActive;
			current = next;
		}
		(++next) %= opt::MAX_PLAYERS;
	} while (next != mDealer + 1);
}

void Tournament::playToEnd()
{
	while (mNActive != 1) {
		playOneHand();
	}
}

void Tournament::playOneHand()
{
	bool finished = mState.startNewHand(mDealer);
	while (!finished) {
		finished = mState.nextState(
			mPlayers[mState.actingPlayer()].act(mState));
	}
}

void Tournament::updateNextActive()
{
	uint8_t i = mDealer;
	do {
		// Player i got eliminated.
		if (!mState.stakes[i]) {
			mNextActive[mPrevActive[i]] = mNextActive[i];
			mPrevActive[mNextActive[i]] = mPrevActive[i];
			--mNActive;
		}
		i = mNextActive[i];
	} while (i != mDealer); // PROBLEM IF mDealer IS NOT ACTIVE ANYMORE. And if there is only 2 are less active left?
}

} // egn