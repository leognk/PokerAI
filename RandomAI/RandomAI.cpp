#include "RandomAI.h"
#include <random>

namespace opt {

RandomAI::RandomAI(
	double foldProba, double callProba,
	unsigned rngSeed) :

	mFoldProba(foldProba),
	mCallProba(callProba),
	mRaiseProba(1 - foldProba - callProba),
	mRng{ (!rngSeed) ? std::random_device{}() : rngSeed }
{
	assert(0 <= foldProba && foldProba <= 1);
	assert(0 <= callProba && callProba <= 1);
}

uint32_t RandomAI::act(const egn::GameState& state)
{
	// Build the distribution over legal actions.
	std::vector<double> proba(state.nActions);
	for (uint8_t i = 0; i < state.nActions; ++i) {
		switch (state.actions[i]) {
		case egn::Action::fold:
			proba[i] = mFoldProba;
			break;
		case egn::Action::call:
			proba[i] = mCallProba;
			break;
		case egn::Action::raise:
			proba[i] = mRaiseProba;
			break;
		case egn::Action::allin:
			proba[i] = mRaiseProba;
			break;
		}
	}
	// Renormalize the distribution if the number of actions is not 3.
	if (state.nActions == 2) {
		double sum = (proba[0] + proba[1]);
		proba[0] /= sum;
		proba[1] /= sum;
	}

	// Pick a random action using the distribution.
	egn::Action action = state.actions[mRandChoice(proba, mRng)];

	switch (action) {

	case egn::Action::fold:
		return state.fold;

	case egn::Action::call:
		return state.call;

	case egn::Action::allin:
		return state.allin;

	case egn::Action::raise:
		mRaiseDist.init(state.minRaise, state.allin);
		return mRaiseDist(mRng);

	default:
		return 0;
	}
}

} // opt