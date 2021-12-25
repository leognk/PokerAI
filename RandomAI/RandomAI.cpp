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

std::pair<egn::Action, egn::chips> RandomAI::act(const egn::GameState& state)
{
	ZoneScoped;
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
		default:
			throw std::runtime_error("Unknown action.");
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

	if (action == egn::Action::raise) {
		mRaiseDist.init(state.minRaise, state.allin);
		return std::make_pair(action, mRaiseDist(mRng));
	}
	else
		return std::make_pair(action, 0);
}

} // opt