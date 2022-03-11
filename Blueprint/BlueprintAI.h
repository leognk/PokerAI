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
		const std::string& blueprintBuildName,
		unsigned rngSeed = 0) :

		abcInfo(
			maxPlayers,
			ante,
			bigBlind,
			initialStake,
			betSizes,
			blueprintGameName,
			rngSeed)
	{
	}

	void act(egn::GameState& state) override
	{

	}

	void update(const egn::GameState& state) override
	{

	}

private:

	abc::AbstractInfoset<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver> abcInfo;

}; // BlueprintAI

} // bp

#endif // ABC_BLUEPRINTAI_H