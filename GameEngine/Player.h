#ifndef EGN_PLAYER_H
#define EGN_PLAYER_H

#include "GameState.h"

namespace egn {

// Base class for a player who can act given a game state.
class Player
{
public:
	// Return an action and a bet (only useful for a raise).
	virtual std::pair<Action, chips> act(const GameState& state) = 0;
};

} // egn

#endif // EGN_PLAYER_H