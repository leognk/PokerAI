#pragma once

#include "../Blueprint/BlueprintAI.h"

#ifdef BLUEPRINTAILIB_EXPORTS
#define BLUEPRINTAILIB_API __declspec(dllexport)
#else
#define BLUEPRINTAILIB_API __declspec(dllimport)
#endif

typedef bp::BlueprintAI<bp::bckSize_t,
	bp::N_BCK_PREFLOP, bp::N_BCK_FLOP,
	bp::N_BCK_TURN, bp::N_BCK_RIVER> blueprintAI_t;

extern "C" {

BLUEPRINTAILIB_API bp::Blueprint* newBlueprint(int rngSeed = 0);
BLUEPRINTAILIB_API void delBlueprint(bp::Blueprint* blueprint);

BLUEPRINTAILIB_API blueprintAI_t* newBlueprintAI(bp::Blueprint* blueprint, int rngSeed = 0);
BLUEPRINTAILIB_API void delBlueprintAI(blueprintAI_t* blueprint);

} // extern "C"