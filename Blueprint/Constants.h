#ifndef BP_CONSTANTS_H
#define BP_CONSTANTS_H

#include "../GameEngine/GameState.h"
#include <vector>

namespace bp {

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK = 200;

static const egn::chips ANTE = 0;
static const egn::chips BIG_BLIND = 100;
static const egn::chips INITIAL_STAKE = 20000;

static const std::vector<std::vector<std::vector<float>>> BET_SIZES = {
	{
		{ 1, 1.25, 1.5, 1.75, 2, 3, 4, 6, 8, 15, 25, 35, 50 },
		{ 0.5, 1, 2, 4, 8, 15, 25, 50 },
		{ 0.5, 1, 2 },
		{ 1 }
	},
	{
		{ 0.25, 0.5, 1, 2, 4 },
		{ 0.5, 1, 2 },
		{ 1 }
	},
	{
		{ 0.5, 1 },
		{ 1 }
	},
	{
		{ 0.5, 1 },
		{ 1 }
	}
};

}

#endif // BP_CONSTANTS_H