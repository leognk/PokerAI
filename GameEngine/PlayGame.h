#ifndef EGN_PLAYGAME_H
#define EGN_PLAYGAME_H

#include "GameState.h"
#include "Player.h"

namespace egn {

// Class for playing poker hands given some Players objects.
class PlayGame
{
public:
	// Set a player's stake to 0 if he is not active.
	// Set rngSeed to 0 to set a random seed.
	PlayGame(
		uint32_t ante, uint32_t bigBlind,
		const std::array<uint32_t, opt::MAX_PLAYERS>& stakes,
		const std::array<Player*, opt::MAX_PLAYERS>& players,
		uint8_t dealerIdx,
		unsigned rngSeed = 0);

	void playToEnd();
	void playAndReset();

private:
	void playOneHand();

	GameState mState;
	std::array<Player*, opt::MAX_PLAYERS> mPlayers;
	uint8_t mDealer;
	std::array<uint32_t, opt::MAX_PLAYERS> mStakes;

}; // PlayGame

} // egn

#endif // EGN_PLAYGAME_H