#ifndef ABC_BLUEPRINTAI_H
#define ABC_BLUEPRINTAI_H

#include "Blueprint.h"
#include "../GameEngine/Player.h"

namespace bp {

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
class BlueprintAI : public egn::Player
{
public:

	// Set rngSeed to 0 to set a random seed.
	BlueprintAI(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const abc::betSizes_t& betSizes,
		const std::string& blueprintGameName,
		const Blueprint* blueprint,
		unsigned rngSeed = 0) :

		abcInfo(
			maxPlayers,
			ante,
			bigBlind,
			initialStake,
			betSizes,
			blueprintGameName,
			rngSeed),

		blueprint(blueprint),

		rng{ (!rngSeed) ? std::random_device{}() : rngSeed },
	{
	}

	void reset(const egn::GameState& state) override
	{

	}

	void act(egn::GameState& state) override
	{
		// Choose action.
		abcInfo.updateStateIds();
		abcInfo.nextState(blueprint->chooseAction(abcInfo), false);
		
		// Set state's action.
		state.action = abcInfo.state.action;
		if (state.action == egn::RAISE)
			state.bet = abcInfo.state.bet;
	}

	void update(const egn::GameState& state) override
	{
		const uint8_t actionId = abcInfo.actionAbc.mapActionToAbcAction(state, rng);
		abcInfo.nextStateWithAction(actionId, false);
	}

private:

	abc::AbstractInfoset<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver> abcInfo;
	Blueprint* blueprint;
	abc::ActionAbstraction::Rng rng;

}; // BlueprintAI

} // bp

#endif // ABC_BLUEPRINTAI_H