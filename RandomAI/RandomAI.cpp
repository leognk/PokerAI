#include "RandomAI.h"
#include "../Utils/Random.h"
#include <random> 

namespace opt {

RandomAI::RandomAI(unsigned rngSeed = 0) :

	mRng{ (!rngSeed) ? std::random_device{}() : rngSeed }
{
}

uint32_t RandomAI::act(egn::GameState state)
{
	omp::FastUniformIntDistribution<uint32_t, 16> actionDist(0, state.nActions - 1);
	egn::Action action = state.actions[actionDist(mRng)];

	switch (action) {

	case egn::Action::fold:
		return state.fold;
		break;

	case egn::Action::call:
		return state.call;
		break;

	case egn::Action::allin:
		return state.allin;
		break;

	case egn::Action::raise:
		mRaiseDist.init(state.minRaise, state.allin);
		return mRaiseDist(mRng);
		break;
	}
}

} // opt