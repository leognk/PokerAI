#ifndef BP_CONSTANTS_H
#define BP_CONSTANTS_H

#include "../GameEngine/GameState.h"
#include "../AbstractInfoset/ActionAbstraction.h"
#include "../LosslessAbstraction/hand_index.h"
#include "../Utils/StringManip.h"
#include "../Utils/Constants.h"

#pragma warning(push)
#pragma warning(disable: 4244)


namespace bp {


//#define BP_GAME_NAMESPACE original
#define BP_GAME_NAMESPACE large
//#define BP_GAME_NAMESPACE medium
//#define BP_GAME_NAMESPACE simple
//#define BP_GAME_NAMESPACE test

//#define BP_BUILD_NAMESPACE original
#define BP_BUILD_NAMESPACE large
//#define BP_BUILD_NAMESPACE medium
//#define BP_BUILD_NAMESPACE simple
//#define BP_BUILD_NAMESPACE test


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace original {


static const std::string BLUEPRINT_GAME_NAME = "ORIGINAL_BLUEPRINT";

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = abc::PREFLOP_SIZE; // Pluribus: 169
static const bckSize_t N_BCK_FLOP = 200; // Pluribus: 200
static const bckSize_t N_BCK_TURN = 200; // Pluribus: 200
static const bckSize_t N_BCK_RIVER = 200; // Pluribus: 200

static const uint8_t MAX_PLAYERS = 6; // Pluribus: 6

static const egn::chips ANTE = 0; // Pluribus: 0
static const egn::chips BIG_BLIND = 100; // Pluribus: 100
static const egn::chips INITIAL_STAKE = 10000; // Pluribus: 10e3

static const abc::betSizes_t BET_SIZES = {
	{
		{ 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2, 3, 4, 6, 8, 15, 25 },
		{ 0.5, 1, 1.5, 2, 4, 8, 15, 25 },
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

static const uint8_t maxNBets = 13;


} // original


namespace large {


static const std::string BLUEPRINT_GAME_NAME = "LARGE_BLUEPRINT";

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = abc::PREFLOP_SIZE; // Pluribus: 169
static const bckSize_t N_BCK_FLOP = 200; // Pluribus: 200
static const bckSize_t N_BCK_TURN = 200; // Pluribus: 200
static const bckSize_t N_BCK_RIVER = 200; // Pluribus: 200

static const uint8_t MAX_PLAYERS = 6; // Pluribus: 6

static const egn::chips ANTE = 0; // Pluribus: 0
static const egn::chips BIG_BLIND = 100; // Pluribus: 100
static const egn::chips INITIAL_STAKE = 10000; // Pluribus: 10e3

static const abc::betSizes_t BET_SIZES = {
	{
		{ 0.5, 0.75, 1, 1.5, 2, 4, 6, 8, 15, 25 },
		{ 0.5, 1, 1.5, 2, 4, 8, 15, 25 },
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

static const uint8_t maxNBetSizes = 10;


} // large


namespace medium {


static const std::string BLUEPRINT_GAME_NAME = "MEDIUM_BLUEPRINT";

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = 50; // Pluribus: 169
static const bckSize_t N_BCK_FLOP = 50; // Pluribus: 200
static const bckSize_t N_BCK_TURN = 50; // Pluribus: 200
static const bckSize_t N_BCK_RIVER = 50; // Pluribus: 200

static const uint8_t MAX_PLAYERS = 6; // Pluribus: 6

static const egn::chips ANTE = 0; // Pluribus: 0
static const egn::chips BIG_BLIND = 100; // Pluribus: 100
static const egn::chips INITIAL_STAKE = 10000; // Pluribus: 10e3

static const abc::betSizes_t BET_SIZES = {
	{
		{ 0.5, 1, 1.5, 2, 4, 8, 15, 25 },
		{ 0.5, 1, 2, 4, 8, 15, 25 },
		{ 0.5, 1, 2 },
		{ 1 }
	},
	{
		{ 0.5, 1, 2 },
		{ 0.5, 1 },
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

static const uint8_t maxNBetSizes = 8;


} // medium


namespace simple {


static const std::string BLUEPRINT_GAME_NAME = "SIMPLE_BLUEPRINT";

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = 5; // Pluribus: 169
static const bckSize_t N_BCK_FLOP = 5; // Pluribus: 200
static const bckSize_t N_BCK_TURN = 5; // Pluribus: 200
static const bckSize_t N_BCK_RIVER = 5; // Pluribus: 200

static const uint8_t MAX_PLAYERS = 6; // Pluribus: 6

static const egn::chips ANTE = 0; // Pluribus: 0
static const egn::chips BIG_BLIND = 100; // Pluribus: 100
static const egn::chips INITIAL_STAKE = 10000; // Pluribus: 10e3

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

static const uint8_t maxNBetSizes = 2;


} // simple


namespace test {


static const std::string BLUEPRINT_GAME_NAME = "TEST_BLUEPRINT";

typedef uint8_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = abc::PREFLOP_SIZE; // Pluribus: 169
static const bckSize_t N_BCK_FLOP = 200; // Pluribus: 200
static const bckSize_t N_BCK_TURN = 200; // Pluribus: 200
static const bckSize_t N_BCK_RIVER = 200; // Pluribus: 200

static const uint8_t MAX_PLAYERS = 2; // Pluribus: 6

static const egn::chips ANTE = 0; // Pluribus: 0
static const egn::chips BIG_BLIND = 100; // Pluribus: 100
static const egn::chips INITIAL_STAKE = 10000; // Pluribus: 10e3

static const abc::betSizes_t BET_SIZES = {
	{
		{ 1, 2, 3 }
	},
	{
		{ 1, 2, 3 }
	},
	{
		{ 1, 2, 3 }
	},
	{
		{ 1, 2, 3 }
	}
};

static const uint8_t maxNBetSizes = 3;


} // test


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace original {


static const std::string BLUEPRINT_BUILD_NAME = "ORIGINAL_BUILD";

static const uint64_t nSnapshots = 54; // Pluribus: 54

static const uint64_t snapshotBeginIter = 3.2e9; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 800e6; // Pluribus: 800e6 (200 min)
static const uint64_t avgSnapshotsPeriod = 1; // Pluribus: 54

static const uint64_t discountEndIter = 1.6e9; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 40e6; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 800e6; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95
static const int32_t pruneThreshold = -300e6; // Pluribus: -300e6
static const int32_t minRegret = -310e6; // Pluribus: -310e6
static const int32_t maxRegret = 100e6; // 100e6

static const uint64_t checkpointPeriod = 240e6; // 240e6 (60 min)
static const uint64_t printPeriod = 1e6; // 1e6 (15 s)

static const double evalStratDuration = 10; // in seconds

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

// This is needed because the snapshots are not discounted.
static_assert(discountEndIter <= snapshotBeginIter);


} // original


namespace large {


static const std::string BLUEPRINT_BUILD_NAME = "LARGE_BUILD";

static const uint64_t nSnapshots = 54; // Pluribus: 54

static const uint64_t snapshotBeginIter = 3.2e9; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 800e6; // Pluribus: 800e6 (200 min)
static const uint64_t avgSnapshotsPeriod = 1; // Pluribus: 54

static const uint64_t discountEndIter = 1.6e9; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 40e6; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 800e6; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95
static const int32_t pruneThreshold = -300e6; // Pluribus: -300e6
static const int32_t minRegret = -310e6; // Pluribus: -310e6
static const int32_t maxRegret = 100e6; // 100e6

static const uint64_t checkpointPeriod = 240e6; // 240e6 (60 min)
static const uint64_t printPeriod = 1e6; // 1e6 (15 s)

static const double evalStratDuration = 10; // in seconds

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

// This is needed because the snapshots are not discounted.
static_assert(discountEndIter <= snapshotBeginIter);


} // large


namespace medium {


static const std::string BLUEPRINT_BUILD_NAME = "MEDIUM_BUILD";

static const uint64_t nSnapshots = 10; // Pluribus: 54

static const uint64_t snapshotBeginIter = 260e3; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 388e3; // Pluribus: 800e6 (200 min)
static const uint64_t avgSnapshotsPeriod = 1; // Pluribus: 54

static const uint64_t discountEndIter = 130e3; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 3.3e3; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 65e3; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95
static const int32_t pruneThreshold = -300e6; // Pluribus: -300e6
static const int32_t minRegret = -310e6; // Pluribus: -310e6
static const int32_t maxRegret = 100e6; // 100e6

static const uint64_t checkpointPeriod = 375e3; // 240e6 (60 min)
static const uint64_t printPeriod = 10e3; // 1e6 (15 s)

static const double evalStratDuration = 3; // in seconds

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

// This is needed because the snapshots are not discounted.
static_assert(discountEndIter <= snapshotBeginIter);


} // medium


namespace simple {


static const std::string BLUEPRINT_BUILD_NAME = "SIMPLE_BUILD";

static const uint64_t nSnapshots = 3; // Pluribus: 54

static const uint64_t snapshotBeginIter = 100; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 25; // Pluribus: 800e6 (200 min)
static const uint64_t avgSnapshotsPeriod = 1; // Pluribus: 54

static const uint64_t discountEndIter = 50; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 2; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 25; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95
static const int32_t pruneThreshold = -300e6; // Pluribus: -300e6
static const int32_t minRegret = -310e6; // Pluribus: -310e6
static const int32_t maxRegret = 100e6; // 100e6

static const uint64_t checkpointPeriod = 50; // 1e8 (25 min)
static const uint64_t printPeriod = 5; // 1e6 (15 s)

static const double evalStratDuration = 2; // in seconds

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

// This is needed because the snapshots are not discounted.
static_assert(discountEndIter <= snapshotBeginIter);


} // simple


namespace test {


static const std::string BLUEPRINT_BUILD_NAME = "TEST_BUILD";

static const uint64_t nSnapshots = 55; // 2.9e6 it -> 55 | 82e6 it -> 1637 // Pluribus: 54

static const uint64_t snapshotBeginIter = 200e3; // Pluribus: 3.2e9 (800 min)
static const uint64_t snapshotPeriod = 50e3; // Pluribus: 800e6 (200 min)
static const uint64_t avgSnapshotsPeriod = 1; // Pluribus: 54

static const uint64_t discountEndIter = 100e3; // Pluribus: 1.6e9 (400 min)
static const uint64_t discountPeriod = 2500; // Pluribus: 40e6 (10 min)

static const uint64_t pruneBeginIter = 50e3; // Pluribus: 800e6 (200 min)
static const uint8_t pruneProbaPerc = 95; // Pluribus: 95
static const int32_t pruneThreshold = -300e6; // Pluribus: -300e6
static const int32_t minRegret = -310e6; // Pluribus: -310e6
static const int32_t maxRegret = 100e6; // 100e6

static const uint64_t checkpointPeriod = 1e6; // 1e8 (25 min)
static const uint64_t printPeriod = 100e3; // 1e6 (15 s)

static const double evalStratDuration = 3; // in seconds

static const uint64_t endIter = snapshotBeginIter + (nSnapshots - 1) * snapshotPeriod;

// This is needed because the snapshots are not discounted.
static_assert(discountEndIter <= snapshotBeginIter);


} // test


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static const std::string BLUEPRINT_GAME_NAME = BP_GAME_NAMESPACE::BLUEPRINT_GAME_NAME;

typedef BP_GAME_NAMESPACE::bckSize_t bckSize_t;
static const bckSize_t N_BCK_PREFLOP = BP_GAME_NAMESPACE::N_BCK_PREFLOP;
static const bckSize_t N_BCK_FLOP = BP_GAME_NAMESPACE::N_BCK_FLOP;
static const bckSize_t N_BCK_TURN = BP_GAME_NAMESPACE::N_BCK_TURN;
static const bckSize_t N_BCK_RIVER = BP_GAME_NAMESPACE::N_BCK_RIVER;

static const uint8_t MAX_PLAYERS = BP_GAME_NAMESPACE::MAX_PLAYERS;

static const egn::chips ANTE = BP_GAME_NAMESPACE::ANTE;
static const egn::chips BIG_BLIND = BP_GAME_NAMESPACE::BIG_BLIND;
static const egn::chips INITIAL_STAKE = BP_GAME_NAMESPACE::INITIAL_STAKE;

static const abc::betSizes_t BET_SIZES = BP_GAME_NAMESPACE::BET_SIZES;

static const uint8_t maxNBetSizes = BP_GAME_NAMESPACE::maxNBetSizes;


static const std::string BLUEPRINT_BUILD_NAME = BP_BUILD_NAMESPACE::BLUEPRINT_BUILD_NAME;

static const uint64_t nSnapshots = BP_BUILD_NAMESPACE::nSnapshots;

static const uint64_t snapshotBeginIter = BP_BUILD_NAMESPACE::snapshotBeginIter;
static const uint64_t snapshotPeriod = BP_BUILD_NAMESPACE::snapshotPeriod;
static const uint64_t avgSnapshotsPeriod = BP_BUILD_NAMESPACE::avgSnapshotsPeriod;

static const uint64_t discountEndIter = BP_BUILD_NAMESPACE::discountEndIter;
static const uint64_t discountPeriod = BP_BUILD_NAMESPACE::discountPeriod;

static const uint64_t pruneBeginIter = BP_BUILD_NAMESPACE::pruneBeginIter;
static const uint8_t pruneProbaPerc = BP_BUILD_NAMESPACE::pruneProbaPerc;
static const int32_t pruneThreshold = BP_BUILD_NAMESPACE::pruneThreshold;
static const int32_t minRegret = BP_BUILD_NAMESPACE::minRegret;
static const int32_t maxRegret = BP_BUILD_NAMESPACE::maxRegret;

static const uint64_t checkpointPeriod = BP_BUILD_NAMESPACE::checkpointPeriod;
static const uint64_t printPeriod = BP_BUILD_NAMESPACE::printPeriod;

static const double evalStratDuration = BP_BUILD_NAMESPACE::evalStratDuration;

static const uint64_t endIter = BP_BUILD_NAMESPACE::endIter;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static std::string blueprintName(const std::string& blueprintGameName, const std::string& blueprintBuildName)
{
	return blueprintGameName + "_" + blueprintBuildName;
}

static std::string blueprintName()
{
	return blueprintName(BLUEPRINT_GAME_NAME, BLUEPRINT_BUILD_NAME);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Blueprint's files paths's names

static std::string blueprintDir(const std::string& blueprintName)
{
	return opt::dataDir + "Blueprint/" + blueprintName + "/";
}

static std::string blueprintTmpDir(const std::string& blueprintName)
{
	return blueprintDir(blueprintName) + "tmp/";
}

static std::string snapshotPath(
	const std::string& blueprintName, unsigned snapshotId, uint8_t roundId)
{
	return blueprintTmpDir(blueprintName) + "SNAPSHOT"
		+ "_" + std::to_string(snapshotId) + "_" + opt::toUpper(egn::roundToString(roundId)) + ".bin";
}

static std::string checkpointPath(const std::string& blueprintName)
{
	return blueprintTmpDir(blueprintName) + "CHECKPOINT.bin";
}

static std::string constantPath(const std::string& blueprintName)
{
	return blueprintDir(blueprintName) + "CONSTANTS.txt";
}

static std::string stratPath(const std::string& blueprintName, uint8_t roundId)
{
	return blueprintDir(blueprintName) + "STRATEGY"
		+ "_" + opt::toUpper(egn::roundToString(roundId)) + ".bin";
}

static std::string blueprintDir()
{
	return blueprintDir(blueprintName());
}

static std::string blueprintTmpDir()
{
	return blueprintTmpDir(blueprintName());
}

static std::string snapshotPath(unsigned snapshotId, uint8_t roundId)
{
	return snapshotPath(blueprintName(), snapshotId, roundId);
}

static std::string checkpointPath()
{
	return checkpointPath(blueprintName());
}

static std::string constantPath()
{
	return constantPath(blueprintName());
}

static std::string stratPath(uint8_t roundId)
{
	return stratPath(blueprintName(), roundId);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef int32_t regret_t;
typedef std::vector<std::vector<std::vector<regret_t>>> regrets_t;
typedef uint8_t strat_t;
typedef uint32_t sumStrat_t;

static const strat_t sumStrat = (std::numeric_limits<strat_t>::max)();

static const uint8_t maxNAbcActions = 3 + maxNBetSizes;


} // bp


#pragma warning(pop)

#endif // BP_CONSTANTS_H