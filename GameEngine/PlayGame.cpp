#include "PlayGame.h"

namespace egn {

#pragma warning(suppress: 26495)
PlayGame::PlayGame(
	uint32_t ante, uint32_t bigBlind,
	const std::array<uint32_t, opt::MAX_PLAYERS>& stakes,
	const std::array<Player, opt::MAX_PLAYERS>& players,
	uint8_t dealerIdx,
	unsigned rngSeed) :

	mState(ante, bigBlind, stakes, rngSeed),
	mPlayers(players),
	mDealer(dealerIdx),
	mStakes(stakes)
{
}

void PlayGame::playToEnd()
{
	// The loop ends when only one active player
	// remains (the winner).
	// We use the forward-moving button rule:
	// the button is moved clockwise.
	do {
		playOneHand();
	} while (mDealer != nextActive(mDealer));
}

void PlayGame::playAndReset()
{
	playOneHand();
	mState.stakes = mStakes;
	nextActive(mDealer);
}

void PlayGame::playOneHand()
{
	bool finished = mState.startNewHand(mDealer);
	while (!finished) {
		finished = mState.nextState(
			mPlayers[mState.actingPlayer].act(mState));
	}
}

uint8_t& PlayGame::nextActive(uint8_t& i)
{
	do {
		(++i) %= opt::MAX_PLAYERS;
	} while (!mState.stakes[i]);
	return i;
}

} // egn