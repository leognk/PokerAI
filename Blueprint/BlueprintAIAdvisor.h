#ifndef BP_BLUEPRINTAIADVISOR_H
#define BP_BLUEPRINTAIADVISOR_H

#include "BlueprintAI.h"
#include "../UserPlayer/UserPlayer.h"

namespace bp {

typedef bp::BlueprintAI<bp::bckSize_t,
	bp::N_BCK_PREFLOP, bp::N_BCK_FLOP,
	bp::N_BCK_TURN, bp::N_BCK_RIVER> blueprintAI_t;

class BlueprintAIAdvisor
{
public:
	BlueprintAIAdvisor(unsigned rngSeed = 0);
	void startNewHand(
		egn::chips ante, egn::chips bb,
		const egn::chips stakes[],
		uint8_t dealer,
		uint8_t myPosition,
		const char* myHand);
	void updateBoardCards(const char* newCards);
	void update(int action, egn::chips bet);
	void getAdvices();

	egn::GameState state;

	egn::Action aiAction;
	egn::chips aiBet;

	uint8_t nActions;
	uint8_t probas[maxNAbcActions];
	uint8_t actions[maxNAbcActions];
	egn::chips bets[maxNAbcActions];

private:
	std::string currBoardCards;

	Blueprint blueprint;
	blueprintAI_t blueprintAI;

}; // BlueprintAIAdvisor

} // bp

#endif // BP_BLUEPRINTAIADVISOR_H