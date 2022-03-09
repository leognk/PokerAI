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
};

} // egn

#endif // EGN_PLAYER_H