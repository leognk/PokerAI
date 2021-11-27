#ifndef EGN_GAMESTATE_H
#define EGN_GAMESTATE_H

#include "Hand.h"
#include "../OMPEval/omp/Random.h"
#include "../OMPEval/omp/HandEvaluator.h"
#include "../Optimus/Constants.h"
#include <vector>

namespace egn {

// Class defining a state of the poker game.
class GameState
{
public:
	// Set a player's stake to 0 if he is not active.
	// Set rngSeed to 0 to set a random seed.
	GameState(
		uint32_t ante, uint32_t bigBlind,
		const std::array<uint32_t, opt::MAX_PLAYERS>& stakes,
		unsigned rngSeed = 0);

	void setAnte(uint32_t ante);
	// Small blind is set to half the big blind.
	void setBigBlind(uint32_t bigBlind);

	// Return whether the hand finished.
	// The game could have finished if there was less than 2 acting players
	// left after charging the antes and the blinds.
	bool startNewHand(uint8_t dealerIdx);
	// bet is the action made by the current acting player.
	// It must be equal to 0 for a check or fold. It the player
	// has the possibility to check, we force him to do so (no fold).
	// Return whether the hand finished.
	bool nextState(uint32_t bet);

	// Legal actions (uint32_t bet) for actingPlayer() are:
	// if      stake() <= call():     0 & stake()
	// else if notFacingFullRaise():  0 & call()
	// else if stake() <= minRaise(): (call() ? 0 & call() : 0) & stake()
	// else    minRaise() < stake():  (call() ? 0 & call() : 0) & [minRaise(), stake()]
	//
	// Equivalent to:
	// if notFacingFullRaise():
	//     0 & min(call(), stake())
	// else:
	//     case stake() <= call():              0 & stake()
	//     case call() < stake() <= minRaise(): 0 & call() & stake()
	//     case minRaise() < stake():           0 & call() & [minRaise(), stake()]
	uint8_t actingPlayer() const;
	bool notFacingFullRaise() const;
	uint32_t stake() const;
	// Chips to put to call
	uint32_t call() const;
	// Minimum chips to put to raise
	uint32_t minRaise() const;

	// Return the rewards obtained by each player after the end of the hand.
	std::array<int64_t, opt::MAX_PLAYERS> rewards() const;

	std::array<uint32_t, opt::MAX_PLAYERS> stakes;

private:
	typedef omp::XoroShiro128Plus Rng;
	typedef omp::FastUniformIntDistribution<unsigned, 16> CardDist;

	enum class Round { preflop, flop, turn, river };
	friend Round& operator++(Round& r);

	void resetPlayers();
	void resetBoard();
	void dealHoleCards(uint64_t& usedCardsMask);
	void dealBoardCards(uint64_t& usedCardsMask);
	void dealCards(omp::Hand& hand, unsigned nCards, uint64_t& usedCardsMask);
	// Return whether the hand finished.
	bool chargeAnte();
	// Return whether the hand finished.
	bool chargeBlinds();

	uint8_t& nextAlive(uint8_t& i) const;
	uint8_t& nextActing(uint8_t& i) const;
	void eraseAlive(uint8_t& i);
	void eraseActing(uint8_t& i);

	void showdown();
	std::vector<std::vector<uint8_t>> getRankings() const;

	Rng mRng;
	CardDist mCardDist;

	uint32_t mAnte, mSB, mBB;

	Round mRound;

	// Players
	std::array<uint32_t, opt::MAX_PLAYERS> mInitialStakes;
	std::array<omp::Hand, opt::MAX_PLAYERS> mHands;
	// Bets since the start of a hand.
	std::array<uint32_t, opt::MAX_PLAYERS> mBets;
	// Active players (were dealt cards and did not fold)
	std::array<uint8_t, opt::MAX_PLAYERS> mAlive;
	uint8_t mFirstAlive;
	uint8_t mNAlive;
	// Alive and did not go all-in yet.
	std::array<uint8_t, opt::MAX_PLAYERS> mActing;
	uint8_t mFirstActing;
	uint8_t mNActing;
	// Acted on the current round.
	std::array<bool, opt::MAX_PLAYERS> mActed;

	// Board
	omp::Hand mBoardCards;
	// Sum of all pots
	uint32_t mPot;
	// Using only one pot
	bool mOnePot;

	uint8_t mDealer;
	// Player making the action passed to nextState.
	uint8_t mCurrentActing;
	// Current number of chips to call (counting from the start of the hand)
	uint32_t mToCall;
	// Largest raise (by) of the current round
	uint32_t mLargestRaise;
	bool mAllInFlag;

	omp::HandEvaluator mEval;
}; // GameState

GameState::Round& operator++(GameState::Round& r)
{
	r = GameState::Round(int(r) + 1);
	return r;
}

} // egn

#endif // EGN_GAMESTATE_H