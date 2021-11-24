#ifndef EGN_GAMESTATE_H
#define EGN_GAMESTATE_H

#include "Hand.h"
#include "../OMPEval/omp/Random.h"
#include "../OMPEval/omp/HandEvaluator.h"
#include "../Optimus/Constants.h"
#include <list>
#include <vector>

namespace egn {

// Class defining a state of the poker game.
class GameState
{
public:
	// Set rngSeed to 0 to set a random seed.
	GameState(unsigned rngSeed = 0);

	void setAnte(uint16_t ante);
	// Small blind is set to half the big blind.
	void setBigBlind(uint16_t bigBlind);
	void setStakes(std::array<uint32_t, opt::MAX_PLAYERS> stakes);
	void setStake(uint8_t playerIdx, uint32_t stake);

	void startNewHand(uint8_t dealerIdx);
	// bet is the action made by the current acting player.
	// It must be equal to 0 for a check or fold. It the player
	// has the possibility to check, we force him to do so (no fold).
	// Returns whether the hand finished.
	bool nextState(uint32_t bet);

	// Legal actions (uint32_t bet) for currentPlayer() are:
	// if      stake() <= call():     0 & stake()
	// else if notFacingFullRaise():  0 & call()
	// else if stake() <= minRaise(): 0 & call() & stake()
	// else    minRaise() < stake():  0 & call() & [minRaise(), stake()]
	//
	// Equivalent to:
	// if notFacingFullRaise():
	//     0 & min(call(), stake())
	// else:
	//     case stake() <= call():              0 & stake()
	//     case call() < stake() <= minRaise(): 0 & call() & stake()
	//     case minRaise() < stake():           0 & call() & [minRaise(), stake()]
	uint8_t currentPlayer() const;
	bool notFacingFullRaise() const;
	uint32_t stake() const;
	// Chips to give to call
	uint32_t call() const;
	// Minimum chips to give to raise
	uint32_t minRaise() const;

private:
	typedef omp::XoroShiro128Plus Rng;
	typedef omp::FastUniformIntDistribution<unsigned, 16> CardDist;

	enum class Round { preflop, flop, turn, river };
	friend Round& operator++(Round& r);
	
	void resetPlayers();
	void resetBoard();
	void chargeAnte();
	void chargeBlinds();
	void dealHoleCards(uint64_t& usedCardsMask);
	void dealBoardCards(uint64_t& usedCardsMask);
	void dealCards(omp::Hand& hand, unsigned nCards, uint64_t& usedCardsMask);
	void goNextPlayer();
	void showdown();
	std::vector<std::vector<uint8_t>> getRankings() const;

	uint16_t mAnte, mSB, mBB;

	Rng mRng;
	CardDist mCardDist;

	// Players
	// Set a player's stake to 0 if he is not active.
	std::array<uint32_t, opt::MAX_PLAYERS> mStakes;
	std::array<omp::Hand, opt::MAX_PLAYERS> mPlayerHands;
	// Bets since the start of the hand
	std::array<uint32_t, opt::MAX_PLAYERS> mBets;

	// Board
	omp::Hand mBoardCards;
	// Sum of all pots
	uint32_t mPot;
	bool mOnePot;

	Round mRound;

	// Number of players still alive
	uint8_t mNPlayers;
	// Indices of players still alive
	// Alive players are ordered starting from the player following the dealer.
	std::list<uint8_t> mPlayers;
	// Player making the action passed to nextState.
	std::list<uint8_t>::iterator mCurrentPlayer;
	uint8_t mDealer;
	// First acting player of the round or last player who raised.
	uint8_t mInitiator;
	// Largest raise (by) of the current round.
	uint32_t mLargestRaise;
	// Current number of chips to call (counting from the start of the hand).
	uint32_t mToCall;
	bool mAllInFlag;

	omp::HandEvaluator mEval;
};

GameState::Round& operator++(GameState::Round& r)
{
	r = GameState::Round(int(r) + 1);
	return r;
}

} // egn

#endif // EGN_GAMESTATE_H