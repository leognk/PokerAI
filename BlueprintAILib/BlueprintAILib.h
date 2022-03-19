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

BLUEPRINTAILIB_API bp::BlueprintAIAdvisor* newBlueprintAIAdvisor(unsigned rngSeed = 0);
BLUEPRINTAILIB_API void adv_startNewHand(
	bp::BlueprintAIAdvisor* advisor,
	egn::chips ante, egn::chips bb,
	const egn::chips stakes[],
	uint8_t dealer,
	uint8_t myPosition,
	const char* myHand);
BLUEPRINTAILIB_API void adv_updateBoardCards(bp::BlueprintAIAdvisor* advisor, const char* newCards);
BLUEPRINTAILIB_API void adv_update(bp::BlueprintAIAdvisor* advisor, int action, egn::chips bet);

BLUEPRINTAILIB_API int adv_aiAction(bp::BlueprintAIAdvisor* advisor) { return advisor->aiAction; };
BLUEPRINTAILIB_API egn::chips adv_aiBet(bp::BlueprintAIAdvisor* advisor) { return advisor->aiBet; };

BLUEPRINTAILIB_API uint8_t adv_nActions(bp::BlueprintAIAdvisor* advisor) { return advisor->nActions; };
BLUEPRINTAILIB_API uint8_t* adv_probas(bp::BlueprintAIAdvisor* advisor) { return advisor->probas; };
BLUEPRINTAILIB_API uint8_t* adv_actions(bp::BlueprintAIAdvisor* advisor) { return advisor->actions; };
BLUEPRINTAILIB_API egn::chips* adv_bets(bp::BlueprintAIAdvisor* advisor) { return advisor->bets; };

} // extern "C"