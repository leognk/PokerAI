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
		chips ante, chips bigBlind,
		const std::array<chips, MAX_PLAYERS>& stakes,
		const std::array<Player*, MAX_PLAYERS>& players,
		uint8_t dealerIdx,
		unsigned rngSeed = 0);

	void playToEnd();
	void playAndReset();

private:
	void playOneHand();

	GameState mState;
	std::array<Player*, MAX_PLAYERS> mPlayers;
	uint8_t mDealer;
	std::array<chips, MAX_PLAYERS> mStakes;

}; // PlayGame

} // egn

#endif // EGN_PLAYGAME_H