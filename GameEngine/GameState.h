#ifndef EGN_GAMESTATE_H
#define EGN_GAMESTATE_H

#include "Hand.h"
#include "../OMPEval/omp/Random.h"
#include "../OMPEval/omp/HandEvaluator.h"
#include "../Optimus/Constants.h"
#include "../Utils/BitOp.h"
#include <iostream>
#include "../tracy/Tracy.hpp"

namespace egn {

typedef uint32_t chips;
typedef int32_t dchips;

// A check is simply a null call.
enum Action { FOLD, CALL, RAISE };
static const uint8_t N_ACTIONS = 3;

#pragma warning(suppress: 26812)
inline std::ostream& operator<<(std::ostream& os, const Action& a)
{
	switch (a) {
	case FOLD:
		return os << "fold";
	case CALL:
		return os << "call";
	case RAISE:
		return os << "raise";
	default:
		throw std::runtime_error("Unknown action.");
	}
}

inline Action actionFromString(const std::string& actionStr)
{
	if (actionStr == "fold")
		return FOLD;
	else if (actionStr == "call")
		return CALL;
	else if (actionStr == "raise")
		return RAISE;
	else
		throw std::runtime_error("String does not represent an Action.");
}

enum Round { PREFLOP, FLOP, TURN, RIVER };
static const uint8_t N_ROUNDS = 4;

#pragma warning(suppress: 26812)
inline Round& operator++(Round& r)
{
	r = Round(r + 1);
	return r;
}

inline std::string roundToString(const Round& r)
{
	switch (r) {
	case PREFLOP:
		return "preflop";
	case FLOP:
		return "flop";
	case TURN:
		return "turn";
	case RIVER:
		return "river";
	default:
		throw std::runtime_error("Unknown round.");
	}
}

inline std::string roundToString(const uint8_t& r)
{
	return roundToString(Round(r));
}

inline Round roundFromString(const std::string& roundStr)
{
	if (roundStr == "preflop")
		return PREFLOP;
	else if (roundStr == "flop")
		return FLOP;
	else if (roundStr == "turn")
		return TURN;
	else if (roundStr == "river")
		return RIVER;
	else
		throw std::runtime_error("String does not represent a Round.");
}

inline std::ostream& operator<<(std::ostream& os, const Round& r)
{
	return os << roundToString(r);
}

// Class defining a state of the poker game.
class GameState
{
public:
	GameState();

	// Set a player's stake to 0 if he is not active.
	// Set rngSeed to 0 to set a random seed.
	GameState(
		chips ante, chips bigBlind,
		const std::array<chips, MAX_PLAYERS>& stakes,
		unsigned rngSeed);

	void setAnte(chips ante0);
	// Small blind is set to half the big blind.
	void setBigBlind(chips bigBlind);

	// Call them BEFORE calling startNewHand.
	void resetUsedCards(); // Call this before setting cards.
	void setHoleCards(uint8_t player, const Hand& hand);
	void setHoleCards(uint8_t player, const uint8_t hand[]);
	void setRandomHoleCards(uint8_t player);
	void setRandomHoleCards(); // Set random cards for every player.
	void setBoardCards(const Hand& boardCards0);
	void setBoardCards(const uint8_t boardCards0[]);
	void setRandomBoardCards();

	// NB: The game could have finished if there was
	// less than 2 acting players left after charging
	// the antes and the blinds.
	void startNewHand(uint8_t dealerIdx, bool dealRandomCards = true);
	void nextState();

	bool isAlive(uint8_t i) const;
	bool isActing(uint8_t i) const;
	// Next active player, ie. non-zero stake player
	// (to set the dealer of the next hand).
	// Do NOT use it when a hand is running (because of all-in players).
	uint8_t& nextActive(uint8_t& i) const;
	uint8_t& nextAlive(uint8_t& i) const;
	uint8_t& nextActing(uint8_t& i) const;
	void addAlive(uint8_t i);
	void addActing(uint8_t i);
	void eraseAlive(uint8_t i);
	void eraseActing(uint8_t i);

	bool foundActivePlayers() const;

	dchips reward(uint8_t i) const;

	void saveRng(std::fstream& file) const;
	void loadRng(std::fstream& file);

	// Stakes at the beginning of the hand.
	std::array<chips, MAX_PLAYERS> initialStakes{};
	std::array<chips, MAX_PLAYERS> stakes{};
	std::array<std::array<uint8_t, omp::HOLE_CARDS>,
		MAX_PLAYERS> hands;
	std::array<uint8_t, omp::BOARD_CARDS> boardCards;

	// action must be set before calling nextState.
	Action action;
	// bet must also be set if action is RAISE.
	chips bet;

	// Legal actions for actingPlayer are given in the array actions
	// from index 0 to nActions excluded (always 2 or 3).
	// If the chosen action is of type raise, a bet
	// between minRaise and allin must be specified.
	// If allin <= minRaise, the only possible bet is allin.
	uint8_t actingPlayer;
	chips call, minRaise, allin;
	std::array<Action, N_ACTIONS> actions{};
	uint8_t nActions;

	// Current situation in which the acting player is in
	// to choose a legal action. There are 3 possible cases.
	uint8_t legalCase;
	static const uint8_t nLegalCases = 3;
	// Masks giving the legal actions for each legal case.
	static constexpr std::array<std::array<bool, nLegalCases>, N_ACTIONS> actionMasks = { {
		{ true, true, false },
		{ false, true, true },
		{ true, true, true }
	} };

	chips ante, sb, bb;

	// Current round.
	Round round;
	// Sum of all pots
	chips pot;
	// Bets since the start of a hand.
	std::array<chips, MAX_PLAYERS> bets{};
	// Whether the hand is finished.
	bool finished;

	uint8_t dealer;

	// To loop over alive players, ie. players who were dealt cards and did not fold.
	uint8_t firstAlive;
	uint8_t nAlive;

	// To loop over acting players, ie. alive players who did not go all-in yet.
	uint8_t firstActing;
	uint8_t nActing;

protected:
	typedef omp::XoroShiro128Plus Rng;
	typedef omp::FastUniformIntDistribution<unsigned, 16> CardDist;

	void resetPlayers();
	void chargeAnte();
	void chargeBlinds();

	static void populateNextLookup();

	void setLegalActions();

	void endGame();
	void showdown();
	bool onePotUsed() const;
	void setRankings(bool onePot);
	omp::Hand getPlayerHand(uint8_t i) const;

	Rng mRng;
	CardDist mCardDist;

	uint64_t usedCardsMask;
	
	// Acted on the current round.
	std::array<bool, MAX_PLAYERS> mActed{};

	// Next player's index lookup table.
	// Give the mask of players and the current player to get
	// the next player.
	static const uint16_t NEXT_LOOKUP_SIZE = 1U << MAX_PLAYERS;
	static std::array<std::array<uint8_t, MAX_PLAYERS>,
		NEXT_LOOKUP_SIZE> NEXT_LOOKUP;

	// Mask of alive players, ie. players who were dealt cards and did not fold.
	uint16_t mAlive;
	// Mask of acting players, ie. alive players who did not go all-in yet.
	uint16_t mActing;

	// Current number of chips to call (counting from the start of the hand)
	chips mToCall;
	// Similar to mToCall, but it can be different at the beginning
	// if the posted BB is incomplete.
	chips mMaxBet;
	// Largest raise (by) of the current round
	chips mLargestRaise;

	// Players ranked from best to worst hand (they are nAlive).
	std::array<uint8_t, MAX_PLAYERS> mRankings{};
	// Cumulated number of players with the same rank ordered
	// from best to worst rank, such that players from
	// mRankings[mCumNSameRanks[i-1]] to mRankings[mCumNSameRanks[i]]
	// (excluded) are players with the i-th best rank.
	// NB: mCumNSameRanks[0] = 0
	std::array<uint8_t, MAX_PLAYERS + 1> mCumNSameRanks{};
	// Size of mCumNSameRanks - 1
	uint8_t mNRanks;

	// Used in showdown.
	std::array<chips, MAX_PLAYERS> mSortedBets{};
	std::array<bool, MAX_PLAYERS> mGiveGain{};

	// Used in setRankings.
	std::array<uint16_t, MAX_PLAYERS> mRanks{};
	omp::HandEvaluator mEval;

}; // GameState

} // egn

#endif // EGN_GAMESTATE_H