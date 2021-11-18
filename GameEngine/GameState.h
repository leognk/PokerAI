#ifndef EGN_GAMESTATE_H
#define EGN_GAMESTATE_H

#include "Hand.h"
#include "../OMPEval/omp/Random.h"
#include <vector>
#include <list>

namespace egn {

// Class defining a state of the poker game.
class GameState
{
public:
	GameState(
		uint16_t ante,
		uint16_t smallBlind,
		uint16_t bigBlind,
		const std::vector<uint32_t>& stakes // Each player's cash.
	);

	// bet is the action made by the current acting player.
	// It must be equal to 0 for a check or fold. It the player
	// has the possibility to check, we force him to do so (no fold).
	void nextState(uint32_t bet);

private:
	typedef omp::XoroShiro128Plus Rng;
	typedef omp::FastUniformIntDistribution<unsigned, 16> CardDist;

	struct Player
	{
		bool active = true;
		uint32_t stake = 0;
		omp::Hand hand = omp::Hand::empty();
		bool alive = true;
		uint32_t bet = 0;
		int64_t gain = 0;
	};

	struct Board
	{
		omp::Hand cards = omp::Hand::empty();
		// Main pot at index 0, followed by side pots.
		std::vector<uint32_t> pots = std::vector<uint32_t>(1);
	};

	enum class Round { preflop, flop, turn, river };
	friend Round& operator++(Round& r);

	void startNewHand();
	void resetPlayers();
	void resetAlivePlayers();
	void chargeAnte();
	void chargeBlinds();
	void dealHoleCards(uint64_t& usedCardsMask);
	void dealBoardCards(uint64_t& usedCardsMask);
	void dealCards(omp::Hand& hand, unsigned nCards, uint64_t& usedCardsMask);

	uint16_t mAnte;
	uint16_t mSmallBlind;
	uint16_t mBigBlind;

	Rng mRng;
	CardDist mCardDist;

	// Maximum number of players.
	uint8_t mNPlayers;
	std::vector<Player> mPlayers;
	Board mBoard;

	Round mCurrentRound;

	uint8_t mNAlivePlayers;
	// Alive players are ordered starting from the player following the dealer.
	std::list<uint8_t> mAlivePlayersIds;
	// Player making the action passed to nextState.
	std::list<uint8_t>::iterator mCurrentPlayerIdx;
	uint8_t mDealerIdx;
	// First acting player of the round or last player who bet.
	uint8_t mOpeningPlayerIdx;
	uint32_t mLastBet;

	// DO NOT FORGET TO ROTATE THE DEALER!
};

GameState::Round& operator++(GameState::Round& r)
{
	r = GameState::Round(int(r) + 1);
	return r;
}

} // egn

#endif // EGN_GAMESTATE_H