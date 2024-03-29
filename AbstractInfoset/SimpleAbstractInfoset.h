#ifndef ABC_SIMPLEABSTRACTINFOSET_H
#define ABC_SIMPLEABSTRACTINFOSET_H

#include "../GameEngine/GameState.h"
#include "ActionAbstraction.h"
#include "ActionSeq.h"

namespace abc {

class TreeTraverser;

// Simpler version of AbstractInfoset class used for tree traversal.
class SimpleAbstractInfoset
{
public:

	SimpleAbstractInfoset(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const betSizes_t& betSizes) :

		dealer(maxPlayers - 1),
		state(ante, bigBlind, {}, 0),
		actionAbc(betSizes)
	{
		std::fill(initialStakes.begin(), initialStakes.begin() + maxPlayers, initialStake);
	}

	SimpleAbstractInfoset& operator=(const SimpleAbstractInfoset& other)
	{
		if (this == &other) return *this;

		state = other.state;
		nRaises = other.nRaises;
		nPlayers = other.nPlayers;
		roundActions = other.roundActions;
		actionAbc = other.actionAbc;

		return *this;
	}

	void startNewHand()
	{
		// Reset member variables.
		nRaises = 0;
		nPlayers = egn::MAX_PLAYERS;
		roundActions.clear();

		state.stakes = initialStakes;
		state.startNewHand(dealer);

		actionAbc.calculateLegalActions(state, nRaises);
	}

	void nextState(uint8_t action)
	{
		actionAbc.setAction(action, state, nRaises);
		roundActions.push_back(action);

		egn::Round oldRound = state.round;
		state.nextState();

		// New round.
		if (state.round != oldRound) {
			nRaises = 0;
			nPlayers = state.nAlive;
			roundActions.clear();
		}

		actionAbc.calculateLegalActions(state, nRaises);
	}

#pragma warning(suppress: 4267)
	uint8_t nActions() const { return actionAbc.legalActions.size(); }

	egn::GameState state;

private:

	const uint8_t dealer;
	std::array<egn::chips, egn::MAX_PLAYERS> initialStakes{};

	// Number of raises done in the current round.
	uint8_t nRaises;

	// Number of players playing at the beginning of the current round.
	uint8_t nPlayers;
	// History of actions made in the current round stored in a compressed format.
	StdActionSeq roundActions;

	ActionAbstraction actionAbc;

	friend class TreeTraverser;

}; // AbstractInfoset

} // abc

#endif // ABC_SIMPLEABSTRACTINFOSET_H