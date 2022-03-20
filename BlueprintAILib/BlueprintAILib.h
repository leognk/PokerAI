#pragma once

#include "../Blueprint/BlueprintAIAdvisor.h"

#ifdef BLUEPRINTAILIB_EXPORTS
#define BLUEPRINTAILIB_API __declspec(dllexport)
#else
#define BLUEPRINTAILIB_API __declspec(dllimport)
#endif

extern "C" {
	
BLUEPRINTAILIB_API int FOLD();
BLUEPRINTAILIB_API int CALL();
BLUEPRINTAILIB_API int RAISE();

BLUEPRINTAILIB_API int ABC_FOLD();
BLUEPRINTAILIB_API int ABC_CALL();
BLUEPRINTAILIB_API int ABC_ALLIN();
BLUEPRINTAILIB_API int ABC_RAISE();

BLUEPRINTAILIB_API int MAX_PLAYERS();

BLUEPRINTAILIB_API bp::BlueprintAIAdvisor* newBlueprintAIAdvisor(unsigned rngSeed = 0);
BLUEPRINTAILIB_API void delBlueprintAIAdvisor(bp::BlueprintAIAdvisor* advisor);
BLUEPRINTAILIB_API void adv_startNewHand(
	bp::BlueprintAIAdvisor* advisor,
	egn::chips ante, egn::chips bb,
	const egn::chips stakes[],
	uint8_t dealer,
	uint8_t myPosition,
	const char* myHand);
BLUEPRINTAILIB_API void adv_updateBoardCards(bp::BlueprintAIAdvisor* advisor, const char* newCards);
BLUEPRINTAILIB_API void adv_update(bp::BlueprintAIAdvisor* advisor, int action, egn::chips bet);
BLUEPRINTAILIB_API void adv_getAdvices(bp::BlueprintAIAdvisor* advisor);

BLUEPRINTAILIB_API int adv_aiAction(bp::BlueprintAIAdvisor* advisor);
BLUEPRINTAILIB_API egn::chips adv_aiBet(bp::BlueprintAIAdvisor* advisor);

BLUEPRINTAILIB_API uint8_t adv_nActions(bp::BlueprintAIAdvisor* advisor);
BLUEPRINTAILIB_API uint8_t* adv_probas(bp::BlueprintAIAdvisor* advisor);
BLUEPRINTAILIB_API uint8_t* adv_actions(bp::BlueprintAIAdvisor* advisor);
BLUEPRINTAILIB_API egn::chips* adv_bets(bp::BlueprintAIAdvisor* advisor);

} // extern "C"