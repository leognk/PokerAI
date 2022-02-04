#ifndef ABC_SIMPLEABSTRACTINFOSET_H
#define ABC_SIMPLEABSTRACTINFOSET_H

#include "../GameEngine/GameState.h"
#include "ActionAbstraction.h"

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
		const std::vector<std::vector<std::vector<float>>>& betSizes) :
		state(ante, bigBlind, {}),
		actionAbc(betSizes)
	{
		std::fill(initialStakes.begin(), initialStakes.begin() + maxPlayers, initialStake);
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

	void nextState(uint8_t actionId)
	{
		actionAbc.setAction(actionAbc.legalActions[actionId], state, nRaises);
		roundActions.push_back(actionAbc.legalActions[actionId]);

		egn::Round oldRound = state.round;
		state.nextState();

		// New round.
		if (state.round != oldRound) {
			nRaises = 0;
			nPlayers = state.mNAlive;
			roundActions.clear();
		}

		actionAbc.calculateLegalActions(state, nRaises);
	}

#pragma warning(suppress: 4267)
	uint8_t nActions() const { return actionAbc.legalActions.size(); }

	egn::GameState state;

private:

	static const uint8_t dealer = egn::MAX_PLAYERS - 1;
	std::array<egn::chips, egn::MAX_PLAYERS> initialStakes{};

	// Number of raises done in the current round.
	uint8_t nRaises;

	// Number of players playing at the beginning of the current round.
	uint8_t nPlayers;
	// History of actions made in the current round.
	std::vector<uint8_t> roundActions;

	abc::ActionAbstraction actionAbc;

	friend class TreeTraverser;

}; // AbstractInfoset

} // abc

#endif // ABC_SIMPLEABSTRACTINFOSET_H