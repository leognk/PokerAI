#ifndef EGN_PLAYER_H
#define EGN_PLAYER_H

#include "GameState.h"

namespace egn {

// Base class for a player who can act given a game state.
class Player
{
public:
	// Return an action (a bet) to take on the given game state.
	virtual chips act(const GameState& state) = 0;
};

} // egn

#endif // EGN_PLAYER_H