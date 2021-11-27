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
	initNextActive();
}

void Tournament::initNextActive()
{
	uint8_t prevActive = 0;
	while (!mState.stake(prevActive))
		++prevActive;
	mNActive = 1;
	for (uint8_t i = prevActive + 1; i < opt::MAX_PLAYERS; ++i) {
		if (mState.stake(prevActive)) {
			mNextActive[prevActive] = i;
			prevActive = i;
			++mNActive;
		}
	}
}

void Tournament::playToEnd()
{
	while (nActive() != 1) {
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

uint8_t Tournament::nActive() const
{
	uint8_t res = 0;
	for (uint8_t i = 0; i < opt::MAX_PLAYERS; ++i) {
		if (mState.stake(i))
			++res;
	}
	return res;
}

uint8_t& Tournament::nextActive(uint8_t& i) const
{

}

} // egn