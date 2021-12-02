#ifndef EGN_GAMESTATE_H
#define EGN_GAMESTATE_H

#include "Hand.h"
#include "../OMPEval/omp/Random.h"
#include "../OMPEval/omp/HandEvaluator.h"
#include "../Optimus/Constants.h"
#include <vector>

namespace egn {

// A check is simply a null call.
enum class Action { fold, call, raise, allin };

enum class Round { preflop, flop, turn, river };

inline
Round& operator++(Round& r)
{
	r = Round(int(r) + 1);
	return r;
}

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

	// The game could have finished if there was less than 2 acting players
	// left after charging the antes and the blinds.
	void startNewHand(uint8_t dealerIdx);
	// bet is the action made by the current acting player.
	// It must be equal to 0 for a check or fold. It the player
	// has the possibility to check, we force him to do so (no fold).
	void nextState(uint32_t bet);

	// Return the rewards obtained by each player after the end of the hand.
	std::array<int64_t, opt::MAX_PLAYERS> rewards() const;

	std::array<uint32_t, opt::MAX_PLAYERS> stakes{};

	// Legal actions for actingPlayer are given in the array actions
	// from index 0 to nActions excluded (always 2 or 3).
	// If the chosen action is of type fold, call or allin,
	// the uint32_t bet to pass to nextState is the value of the variables
	// fold (always 0), call or allin respectively.
	// If the chosen action is of type raise, the uint32_t bet must be
	// between minRaise and allin.
	uint8_t actingPlayer;
	uint32_t fold = 0, call, minRaise, allin;
	std::array<Action, 3> actions{};
	uint8_t nActions;

	// Whether the hand is finished.
	bool finished;

private:
	typedef omp::XoroShiro128Plus Rng;
	typedef omp::FastUniformIntDistribution<unsigned, 16> CardDist;

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

	void setLegalActions();

	void showdown();
	std::vector<std::vector<uint8_t>> getRankings() const;

	Rng mRng;
	CardDist mCardDist;

	uint32_t mAnte, mSB, mBB;

	Round mRound;

	// Players
	std::array<uint32_t, opt::MAX_PLAYERS> mInitialStakes{};
	std::array<omp::Hand, opt::MAX_PLAYERS> mHands{};
	// Bets since the start of a hand.
	std::array<uint32_t, opt::MAX_PLAYERS> mBets{};
	// Active players (were dealt cards and did not fold)
	std::array<uint8_t, opt::MAX_PLAYERS> mAlive{};
	uint8_t mFirstAlive;
	uint8_t mNAlive;
	// Alive and did not go all-in yet.
	std::array<uint8_t, opt::MAX_PLAYERS> mActing{};
	uint8_t mFirstActing;
	uint8_t mNActing;
	// Acted on the current round.
	std::array<bool, opt::MAX_PLAYERS> mActed{};

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

} // egn

#endif // EGN_GAMESTATE_H