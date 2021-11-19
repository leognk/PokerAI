#ifndef EGN_GAMESTATE_H
#define EGN_GAMESTATE_H

#include "Hand.h"
#include "../OMPEval/omp/Random.h"
#include "../Optimus/Constants.h"
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
		const std::array<uint32_t, opt::MAX_PLAYERS>& stakes // Each player's cash.
	);

	// bet is the action made by the current acting player.
	// It must be equal to 0 for a check or fold. It the player
	// has the possibility to check, we force him to do so (no fold).
	void nextState(uint32_t bet);

private:
	typedef omp::XoroShiro128Plus Rng;
	typedef omp::FastUniformIntDistribution<unsigned, 16> CardDist;

	enum class Round { preflop, flop, turn, river };
	friend Round& operator++(Round& r);

	void startNewHand();
	void resetPlayers();
	void resetBoard();
	void chargeAnte();
	void chargeBlinds();
	void dealHoleCards(uint64_t& usedCardsMask);
	void dealBoardCards(uint64_t& usedCardsMask);
	void dealCards(omp::Hand& hand, unsigned nCards, uint64_t& usedCardsMask);
	void goNextPlayer();

	uint16_t mAnte, mSB, mBB;

	Rng mRng;
	CardDist mCardDist;

	// Players
	// Set a player's stake to 0 if he is not active.
	std::array<uint32_t, opt::MAX_PLAYERS> mStakes;
	std::array<omp::Hand, opt::MAX_PLAYERS> mPlayersHands;
	std::array<uint32_t, opt::MAX_PLAYERS> mBets;

	// Board
	omp::Hand mBoardCards;
	// Main pot at index 0, followed by side pots.
	std::array<uint32_t, opt::MAX_PLAYERS - 1> mPots;

	Round mCurrentRound;

	// Number of players still alive
	uint8_t mNPlayers;
	// Indices of players still alive
	// Alive players are ordered starting from the player following the dealer.
	std::list<uint8_t> mPlayers;
	// Player making the action passed to nextState.
	std::list<uint8_t>::iterator mCurrentPlayer;
	uint8_t mDealer;
	// First acting player of the round or last player who bet.
	uint8_t mOpeningPlayer;
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