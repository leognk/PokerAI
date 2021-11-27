#ifndef EGN_TOURNAMENT_H
#define EGN_TOURNAMENT_H

#include "GameState.h"
#include "Player.h"

namespace egn {

// Class for playing a poker tournament given some Players objects.
class Tournament
{
public:
	// Set rngSeed to 0 to set a random seed.
	Tournament(
		uint32_t ante, uint32_t bigBlind,
		const std::array<uint32_t, opt::MAX_PLAYERS>& stakes,
		const std::array<Player, opt::MAX_PLAYERS>& players,
		uint8_t dealerIdx,
		unsigned rngSeed = 0);

	void playToEnd();

private:
	void initNextActive();
	void playOneHand();
	uint8_t nActive() const;
	uint8_t& nextActive(uint8_t& i) const;

	GameState mState;
	std::array<Player, opt::MAX_PLAYERS> mPlayers;

	std::array<uint8_t, opt::MAX_PLAYERS> mNextActive;
	uint8_t mDealer;
	uint8_t mNActive;
};

} // egn

#endif // EGN_TOURNAMENT_H