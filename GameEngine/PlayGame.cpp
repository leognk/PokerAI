#include "PlayGame.h"
#include <iostream>

namespace egn {

#pragma warning(suppress: 26495)
PlayGame::PlayGame(
	chips ante, chips bigBlind,
	const std::array<chips, MAX_PLAYERS>& stakes,
	const std::array<Player*, MAX_PLAYERS>& players,
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
		mState.nextActive(mDealer);
	} while (mState.foundActivePlayers());
}

void PlayGame::playAndReset()
{
	playOneHand();
	mState.stakes = mStakes;
	mState.nextActive(mDealer);
}

void PlayGame::playOneHand()
{
	//ZoneScoped;
	mState.startNewHand(mDealer);
	while (!mState.finished) {
		mPlayers[mState.actingPlayer]->act(mState);
		mState.nextState();
	}
}

} // egn