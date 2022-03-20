#include "pch.h"
#include "BlueprintAILib.h"
	
int FOLD() { return egn::FOLD; };
int CALL() { return egn::CALL; };
int RAISE() { return egn::RAISE; };

int ABC_FOLD() { return abc::FOLD; };
int ABC_CALL() { return abc::CALL; };
int ABC_ALLIN() { return abc::ALLIN; };
int ABC_RAISE() { return abc::RAISE; };

int MAX_PLAYERS() { return bp::MAX_PLAYERS; };

bp::BlueprintAIAdvisor* newBlueprintAIAdvisor(unsigned rngSeed)
{
	return new bp::BlueprintAIAdvisor(rngSeed);
}

void delBlueprintAIAdvisor(bp::BlueprintAIAdvisor* advisor)
{
	delete advisor;
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

void adv_getAdvices(bp::BlueprintAIAdvisor* advisor)
{
	advisor->getAdvices();
}

int adv_aiAction(bp::BlueprintAIAdvisor* advisor) { return advisor->aiAction; };
egn::chips adv_aiBet(bp::BlueprintAIAdvisor* advisor) { return advisor->aiBet; };

uint8_t adv_nActions(bp::BlueprintAIAdvisor* advisor) { return advisor->nActions; };
uint8_t* adv_probas(bp::BlueprintAIAdvisor* advisor) { return advisor->probas; };
uint8_t* adv_actions(bp::BlueprintAIAdvisor* advisor) { return advisor->actions; };
egn::chips* adv_bets(bp::BlueprintAIAdvisor* advisor) { return advisor->bets; };