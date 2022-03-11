#ifndef EGN_PLAYER_H
#define EGN_PLAYER_H

#include "GameState.h"

namespace egn {

// Base class for a player who can act given a game state.
class Player
{
public:
	// Set GameState's action and bet member variables.
	// (bet need to be set only for a raise)
	virtual void act(GameState& state) = 0;

	// Inform the player about the last action made by an opponent.
	// Call it AFTER calling the opponent's act method and
	// BEFORE calling state.nextState().
	virtual void update(const GameState& state) {}
};

} // egn

#endif // EGN_PLAYER_H