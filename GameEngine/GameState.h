#ifndef EGN_GAMESTATE_H
#define EGN_GAMESTATE_H

#include "Hand.h"
#include "../OMPEval/omp/Random.h"
#include "../OMPEval/omp/HandEvaluator.h"
#include "../Optimus/Constants.h"
#include <vector>
#include <iostream>

namespace egn {

typedef uint32_t chips;
typedef int32_t dchips;

// A check is simply a null call.
enum class Action { fold, call, raise, allin };

inline std::ostream& operator<<(std::ostream& os, const Action& a)
{
	switch (a) {
	case Action::fold:
		return os << "fold";
	case Action::call:
		return os << "call";
	case Action::raise:
		return os << "raise";
	case Action::allin:
		return os << "all-in";
	default:
		return os;
	}
}

enum class Round { preflop, flop, turn, river };

inline Round& operator++(Round& r)
{
	r = Round(int(r) + 1);
	return r;
}

inline std::ostream& operator<<(std::ostream& os, const Round& r)
{
	switch (r) {
	case Round::preflop:
		return os << "preflop";
	case Round::flop:
		return os << "flop";
	case Round::turn:
		return os << "turn";
	case Round::river:
		return os << "river";
	default:
		return os;
	}
}

// Class defining a state of the poker game.
class GameState
{
public:
	// Set a player's stake to 0 if he is not active.
	// Set rngSeed to 0 to set a random seed.
	GameState(
		chips ante, chips bigBlind,
		const std::array<chips, opt::MAX_PLAYERS>& stakes,
		unsigned rngSeed = 0);

	void setAnte(chips ante);
	// Small blind is set to half the big blind.
	void setBigBlind(chips bigBlind);

	// The game could have finished if there was less than 2 acting players
	// left after charging the antes and the blinds.
	void startNewHand(uint8_t dealerIdx, bool dealRandomCards = true);
	void setHoleCards(uint8_t player, const Hand& hand);
	void setBoardCards(const Hand& boardCards);
	// bet is the action made by the current acting player.
	// It must be equal to 0 for a check or fold. It the player
	// has the possibility to check, we force him to do so (no fold).
	void nextState(chips bet);
	// Next active player, ie. non-zero stake player
	// (to set the dealer of the next hand).
	// Do not use it when a hand is running (because of all-in players).
	uint8_t& nextActive(uint8_t& i) const;

	std::array<chips, opt::MAX_PLAYERS> stakes{};

	// Legal actions for actingPlayer are given in the array actions
	// from index 0 to nActions excluded (always 2 or 3).
	// If the chosen action is of type fold, call or allin,
	// the bet to pass to nextState is the value of the variables
	// fold (always 0), call or allin respectively.
	// If the chosen action is of type raise, the bet must be
	// between minRaise and allin.
	uint8_t actingPlayer;
	chips fold = 0, call, minRaise, allin;
	std::array<Action, 3> actions{};
	uint8_t nActions;

	// Current round.
	Round round;
	// Whether the hand is finished.
	bool finished;
	// Rewards obtained by each player after the end of the hand.
	std::array<dchips, opt::MAX_PLAYERS> rewards{};

protected:
	typedef omp::XoroShiro128Plus Rng;
	typedef omp::FastUniformIntDistribution<unsigned, 16> CardDist;

	void resetPlayers();
	void resetBoard();
	void dealHoleCards(uint64_t& usedCardsMask);
	void dealBoardCards(uint64_t& usedCardsMask);
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
	void setRewards();

	Rng mRng;
	CardDist mCardDist;

	chips mAnte, mSB, mBB;

	// Players
	std::array<chips, opt::MAX_PLAYERS> mInitialStakes{};
	std::array<Hand, opt::MAX_PLAYERS> mHands{};
	// Bets since the start of a hand.
	std::array<chips, opt::MAX_PLAYERS> mBets{};
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
	Hand mBoardCards;
	// Sum of all pots
	chips mPot;
	// Using only one pot
	bool mOnePot;

	uint8_t mDealer;
	// Player making the action passed to nextState.
	uint8_t mCurrentActing;
	// Current number of chips to call (counting from the start of the hand)
	chips mToCall;
	// Largest raise (by) of the current round
	chips mLargestRaise;
	bool mAllInFlag;

	omp::HandEvaluator mEval;

}; // GameState

} // egn

#endif // EGN_GAMESTATE_H