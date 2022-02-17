#ifndef BP_CONSTANTS_H
#define BP_CONSTANTS_H

#include "../GameEngine/GameState.h"
#include "../AbstractInfoset/ActionAbstraction.h"

#pragma warning(push)
#pragma warning(disable: 4244)

namespace bp {


#define ORIGINAL_BLUEPRINT 0
#define SIMPLE_BLUEPRINT 1

//#define MACRO_BLUEPRINT_NAME ORIGINAL_BLUEPRINT
#define MACRO_BLUEPRINT_NAME SIMPLE_BLUEPRINT


#if MACRO_BLUEPRINT_NAME == ORIGINAL_BLUEPRINT


////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// ORIGINAL VERSION ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////// HYPERPARAMETERS ///////////////////////////////////////


static const std::string BLUEPRINT_NAME = "BLUEPRINT";

static const uint64_t nSnapshots = 54; // Pluribus: 54

static const uint64_t snapshotBeginIter = 3.2e9; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 800e6; // Pluribus: 800e6 (200 min)

static const uint64_t discountEndIter = 1.6e9; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 40e6; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 800e6; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95

static const uint64_t preflopStratUpdatePeriod = 10e3; // Pluribus: 10e3

static const uint64_t checkpointPeriod = 1e8; // 1e8 (25 min)

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

static_assert(discountEndIter <= snapshotBeginIter);


//////////////////////////////////////// GAME PARAMETERS ///////////////////////////////////////


typedef uint8_t bckSize_t;
static const bckSize_t N_BCK = 200; // Pluribus: 200

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


#elif MACRO_BLUEPRINT_NAME == SIMPLE_BLUEPRINT


////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// SIMPLE VERSION ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////// HYPERPARAMETERS ///////////////////////////////////////


static const std::string BLUEPRINT_NAME = "SIMPLE_BLUEPRINT";

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

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

static_assert(discountEndIter <= snapshotBeginIter);


//////////////////////////////////////// GAME PARAMETERS ///////////////////////////////////////


typedef uint8_t bckSize_t;
static const bckSize_t N_BCK = 5; // Pluribus: 200

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


}

#pragma warning(pop)

#endif // BP_CONSTANTS_H