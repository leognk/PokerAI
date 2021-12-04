#ifndef EGN_GAMESTATEPRINT_H
#define EGN_GAMESTATEPRINT_H

#include "GameState.h"

namespace egn {

// Class extending GameState to print it.
class GameStatePrint : public GameState
{
public:
	GameStatePrint(
		uint32_t ante, uint32_t bigBlind,
		const std::array<uint32_t, opt::MAX_PLAYERS>& stakes,
		unsigned rngSeed = 0,
		std::string separatorLine = "");

	void startNewHand(uint8_t dealerIdx);
	void nextState(uint32_t bet);
	uint8_t& nextActiveInGame(uint8_t& i) const;

	std::ostream& printState(std::ostream& os) const;
	std::ostream& printRewards(std::ostream& os) const;

private:
	template<typename Container>
	unsigned maxChars(Container c) const;
	template<typename Container>
	unsigned maxCharsString(Container c) const;
	std::string handCategory(uint8_t player) const;

	std::string mSeparatorLine;

	uint8_t mFirstActive;
	std::array<uint32_t, opt::MAX_PLAYERS> mRoundBets;
	std::array<std::string, opt::MAX_PLAYERS> mLastActions;
	std::array<uint32_t, opt::MAX_PLAYERS> mLastBets{};
	Round mPrevActionRound;
	uint8_t mPrevActing;

}; // GameStatePrint

} // egn

#endif // EGN_GAMESTATEPRINT_H