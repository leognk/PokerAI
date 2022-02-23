#ifndef BP_CONSTANTS_H
#define BP_CONSTANTS_H

#include "../GameEngine/GameState.h"
#include "../AbstractInfoset/ActionAbstraction.h"
#include "../LosslessAbstraction/hand_index.h"

#pragma warning(push)
#pragma warning(disable: 4244)


namespace bp {


#define ORIGINAL_BLUEPRINT 2
#define MEDIUM_BLUEPRINT 1
#define SIMPLE_BLUEPRINT 0

#define ORIGINAL_BLUEPRINT_BUILD 2
#define MEDIUM_BLUEPRINT_BUILD 1
#define SIMPLE_BLUEPRINT_BUILD 0


#define MACRO_BLUEPRINT_NAME ORIGINAL_BLUEPRINT
//#define MACRO_BLUEPRINT_NAME MEDIUM_BLUEPRINT
//#define MACRO_BLUEPRINT_NAME SIMPLE_BLUEPRINT

#define BLUEPRINT_BUILD_SIZE ORIGINAL_BLUEPRINT_BUILD
//#define BLUEPRINT_BUILD_SIZE MEDIUM_BLUEPRINT_BUILD
//#define BLUEPRINT_BUILD_SIZE SIMPLE_BLUEPRINT_BUILD


#if MACRO_BLUEPRINT_NAME == ORIGINAL_BLUEPRINT


static const std::string BLUEPRINT_GAME_NAME = "ORIGINAL_BLUEPRINT";

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = abc::PREFLOP_SIZE; // Pluribus: 169
static const bckSize_t N_BCK_FLOP = 200; // Pluribus: 200
static const bckSize_t N_BCK_TURN = 200; // Pluribus: 200
static const bckSize_t N_BCK_RIVER = 200; // Pluribus: 200

static const uint8_t MAX_PLAYERS = 6; // Pluribus: 6

static const egn::chips ANTE = 0; // Pluribus: 0
static const egn::chips BIG_BLIND = 100; // Pluribus: 100
static const egn::chips INITIAL_STAKE = 10e3; // Pluribus: 10e3

static const abc::betSizes_t BET_SIZES = {
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


#elif MACRO_BLUEPRINT_NAME == MEDIUM_BLUEPRINT


static const std::string BLUEPRINT_GAME_NAME = "MEDIUM_BLUEPRINT";

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = 50; // Pluribus: 169
static const bckSize_t N_BCK_FLOP = 50; // Pluribus: 200
static const bckSize_t N_BCK_TURN = 50; // Pluribus: 200
static const bckSize_t N_BCK_RIVER = 50; // Pluribus: 200

static const uint8_t MAX_PLAYERS = 6; // Pluribus: 6

static const egn::chips ANTE = 0; // Pluribus: 0
static const egn::chips BIG_BLIND = 100; // Pluribus: 100
static const egn::chips INITIAL_STAKE = 10e3; // Pluribus: 10e3

static const abc::betSizes_t BET_SIZES = {
	{
		{ 2, 4, 6, 8, 15, 25, 35 },
		{ 0.5, 1, 2, 4, 8, 15, 25 },
		{ 0.5, 1, 2 },
		{ 1 }
	},
	{
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


#elif MACRO_BLUEPRINT_NAME == SIMPLE_BLUEPRINT


static const std::string BLUEPRINT_GAME_NAME = "SIMPLE_BLUEPRINT";

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = 5; // Pluribus: 169
static const bckSize_t N_BCK_FLOP = 5; // Pluribus: 200
static const bckSize_t N_BCK_TURN = 5; // Pluribus: 200
static const bckSize_t N_BCK_RIVER = 5; // Pluribus: 200

static const uint8_t MAX_PLAYERS = 3; // Pluribus: 6

static const egn::chips ANTE = 0; // Pluribus: 0
static const egn::chips BIG_BLIND = 100; // Pluribus: 100
static const egn::chips INITIAL_STAKE = 10e3; // Pluribus: 10e3

static const abc::betSizes_t BET_SIZES = {
	{
		{ 1, 2 },
		{ 1 }
	},
	{
		{ 1, 2 },
		{ 1 }
	},
	{
		{ 1, 2 },
		{ 1 }
	},
	{
		{ 1, 2 },
		{ 1 }
	}
};


#endif // MACRO_BLUEPRINT_NAME


#if BLUEPRINT_BUILD_SIZE == ORIGINAL_BLUEPRINT_BUILD


static const std::string BLUEPRINT_BUILD_NAME = "ORIGINAL_BUILD";

static const uint64_t nSnapshots = 54; // Pluribus: 54

static const uint64_t snapshotBeginIter = 3.2e9; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 800e6; // Pluribus: 800e6 (200 min)

static const uint64_t discountEndIter = 1.6e9; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 40e6; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 800e6; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95
static const int32_t pruneThreshold = -300e6; // Pluribus: -300e6
static const int32_t minRegret = -310e6; // Pluribus: -310e6

static const uint64_t preflopStratUpdatePeriod = 10e3; // Pluribus: 10e3

static const uint64_t checkpointPeriod = 1e8; // 1e8 (25 min)
static const uint64_t printPeriod = 4e6; // 4e6 (1 min)

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

static_assert(discountEndIter <= snapshotBeginIter);


#elif BLUEPRINT_BUILD_SIZE == MEDIUM_BLUEPRINT_BUILD


static const std::string BLUEPRINT_BUILD_NAME = "MEDIUM_BUILD";

static const uint64_t nSnapshots = 10; // Pluribus: 54

static const uint64_t snapshotBeginIter = 260e3; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 388e3; // Pluribus: 800e6 (200 min)

static const uint64_t discountEndIter = 130e3; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 3.3e3; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 65e3; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95
static const int32_t pruneThreshold = -300e6; // Pluribus: -300e6
static const int32_t minRegret = -310e6; // Pluribus: -310e6

static const uint64_t preflopStratUpdatePeriod = 2; // Pluribus: 10e3

static const uint64_t checkpointPeriod = 375e3; // 1e8 (25 min)
static const uint64_t printPeriod = 10e3; // 4e6 (1 min)

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

static_assert(discountEndIter <= snapshotBeginIter);


#elif BLUEPRINT_BUILD_SIZE == SIMPLE_BLUEPRINT_BUILD


static const std::string BLUEPRINT_BUILD_NAME = "SIMPLE_BUILD";

static const uint64_t nSnapshots = 3; // Pluribus: 54

static const uint64_t snapshotBeginIter = 100; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 25; // Pluribus: 800e6 (200 min)

static const uint64_t discountEndIter = 50; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 2; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 25; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95
static const int32_t pruneThreshold = -300e6; // Pluribus: -300e6
static const int32_t minRegret = -310e6; // Pluribus: -310e6

static const uint64_t preflopStratUpdatePeriod = 2; // Pluribus: 10e3

static const uint64_t checkpointPeriod = 5; // 1e8 (25 min)
static const uint64_t printPeriod = 5; // 4e6 (1 min)

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

static_assert(discountEndIter <= snapshotBeginIter);


#endif // BLUEPRINT_BUILD_SIZE


static const std::string BLUEPRINT_NAME = BLUEPRINT_GAME_NAME + "_" + BLUEPRINT_BUILD_NAME;


}

#pragma warning(pop)

#endif // BP_CONSTANTS_H