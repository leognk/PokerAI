#ifndef EGN_ACTION_H
#define EGN_ACTION_H

#include <cstdint>

namespace egn {

struct Action
{
	enum class ActionType { fold, check, bet };
	uint32_t bet = 0;
};

} // egn

#endif // EGN_ACTION_H