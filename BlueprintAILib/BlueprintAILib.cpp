#include "pch.h"
#include "BlueprintAILib.h"

bp::BlueprintAIAdvisor* newBlueprintAIAdvisor(unsigned rngSeed)
{
	return new bp::BlueprintAIAdvisor(rngSeed);
}

void adv_startNewHand(
	bp::BlueprintAIAdvisor* advisor,
	egn::chips ante, egn::chips bb,
	const egn::chips stakes[],
	uint8_t dealer,
	uint8_t myPosition,
	const char* myHand)
{
	advisor->startNewHand(ante, bb, stakes, dealer, myPosition, myHand);
}

void adv_updateBoardCards(bp::BlueprintAIAdvisor* advisor, const char* newCards)
{
	advisor->updateBoardCards(newCards);
}

void adv_update(bp::BlueprintAIAdvisor* advisor, int action, egn::chips bet)
{
	advisor->update(action, bet);
}