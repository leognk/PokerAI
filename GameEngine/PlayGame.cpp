#include "PlayGame.h"
#include <iostream>

namespace egn {

#pragma warning(suppress: 26495)
PlayGame::PlayGame(
	uint32_t ante, uint32_t bigBlind,
	const std::array<uint32_t, opt::MAX_PLAYERS>& stakes,
	const std::array<Player*, opt::MAX_PLAYERS>& players,
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
	uint8_t prevDealer = 0;
	do {
		playOneHand();
		prevDealer = mDealer;
	} while (prevDealer != mState.nextActive(mDealer));
}

void PlayGame::playAndReset()
{
	playOneHand();
	mState.stakes = mStakes;
	mState.nextActive(mDealer);
}

void PlayGame::playOneHand()
{
	mState.startNewHand(mDealer);
	while (!mState.finished) {
		mState.nextState(mPlayers[mState.actingPlayer]->act(mState));
	}
}

} // egn