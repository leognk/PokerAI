#include "BlueprintAIAdvisor.h"

namespace bp {

BlueprintAIAdvisor::BlueprintAIAdvisor(unsigned rngSeed) :
	blueprint(
		bp::BP_GAME_NAMESPACE::BLUEPRINT_GAME_NAME,
		bp::BP_BUILD_NAMESPACE::BLUEPRINT_BUILD_NAME,
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
	uint8_t myPosition,
	const char* myHand)
{
	state.setAnte(ante);
	state.setBigBlind(bb);

	for (uint8_t i = 0; i < bp::MAX_PLAYERS; ++i)
		state.stakes[i] = stakes[i];

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
	// Set state's action.
	state.action = egn::Action(action);
	if (state.action == egn::RAISE)
		state.bet = bet;

	// Update states.
	blueprintAI.update(state);
	state.nextState();
}

void BlueprintAIAdvisor::getAdvices()
{
	blueprintAI.act(state, aiAction, aiBet);

	nActions = blueprintAI.abcInfo.nActions();

	const std::vector<uint8_t> probasVect =
		blueprint.calculateProbasPerc(blueprintAI.abcInfo);
	std::copy(probasVect.begin(), probasVect.end(), probas);

	const std::vector<uint8_t> actionsVect =
		blueprintAI.abcInfo.actionAbc.legalActions;
	std::copy(actionsVect.begin(), actionsVect.end(), actions);

	for (uint8_t i = 0; i < nActions; ++i) {
		switch (actions[i]) {
		case abc::FOLD:
			bets[i] = 0;
			break;
		case abc::CALL:
			bets[i] = state.call;
			break;
		case abc::ALLIN:
			bets[i] = state.allin;
			break;
			// RAISE
		default:
			blueprintAI.abcInfo.setStateAction(actions[i]);
			const egn::chips bet = blueprintAI.abcInfo.state.bet;
			bets[i] = blueprintAI.abcToRealChips(bet);
		}
	}
}

} // bp