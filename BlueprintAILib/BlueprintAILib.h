#pragma once

#include "../Blueprint/BlueprintAIAdvisor.h"

#ifdef BLUEPRINTAILIB_EXPORTS
#define BLUEPRINTAILIB_API __declspec(dllexport)
#else
#define BLUEPRINTAILIB_API __declspec(dllimport)
#endif

extern "C" {
	
BLUEPRINTAILIB_API int FOLD() { return egn::FOLD; };
BLUEPRINTAILIB_API int CALL() { return egn::CALL; };
BLUEPRINTAILIB_API int RAISE() { return egn::RAISE; };

BLUEPRINTAILIB_API int MAX_PLAYERS() { return bp::MAX_PLAYERS; };

} // extern "C"