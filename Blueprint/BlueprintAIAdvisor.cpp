#include "BlueprintAIAdvisor.h"

namespace bp {

BlueprintAIAdvisor::BlueprintAIAdvisor(unsigned rngSeed) :
	blueprint(
		bp::simple::BLUEPRINT_GAME_NAME,
		bp::medium::BLUEPRINT_BUILD_NAME,
		rngSeed),

	blueprintAI(BLUEPRINT_AI_BUILDER(
		bp::BP_GAME_NAMESPACE,
		bp::BIG_BLIND,
		&blueprint,
		rngSeed))
{
	blueprint.loadStrat();
}

void BlueprintAIAdvisor::startNewHand(
	egn::chips ante, egn::chips bb,
	const egn::chips stakes[],
	uint8_t dealer,
	uint8_t myPosition0,
	const char* myHand)
{
	state.setAnte(ante);
	state.setBigBlind(bb);

	for (uint8_t i = 0; i < bp::MAX_PLAYERS; ++i)
		state.stakes[i] = stakes[i];

	myPosition = myPosition0;

	currBoardCards = "";
	state.setHoleCards(myPosition, egn::Hand(std::string(myHand)));

	state.startNewHand(dealer, false);

	blueprintAI.reset(state);
}

void BlueprintAIAdvisor::updateBoardCards(const char* newCards)
{
	currBoardCards += std::string(newCards);
	state.setBoardCards(egn::Hand(currBoardCards));
}

void BlueprintAIAdvisor::update(int action, egn::chips bet)
{
	state.action = egn::Action(action);
	if (state.action == egn::RAISE)
		state.bet = bet;

	if (state.actingPlayer == myPosition) {
		blueprintAI.act(aiAction, aiBet);
		aiProbas = blueprint.calculateProbasPerc(blueprintAI.abcInfo);
		aiActions = blueprintAI.abcInfo.actionAbc.legalActions;
	}

	blueprintAI.update(state);
	state.nextState();
}

} // bp