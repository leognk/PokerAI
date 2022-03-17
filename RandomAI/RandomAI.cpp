#include "RandomAI.h"
#include <random>
#include <numeric>

namespace opt {

RandomAI::RandomAI(
	double foldProba, double callProba, unsigned rngSeed) :

	mRng{ (!rngSeed) ? std::random_device{}() : rngSeed }
{
	setProbas(foldProba, callProba);
}

void RandomAI::setProbas(double foldProba, double callProba)
{
	mFoldWeight = (unsigned)std::round(foldProba * mRandChoice.RANGE);
	mCallWeight = (unsigned)std::round(callProba * mRandChoice.RANGE);
	mRaiseWeight = (unsigned)(mRandChoice.RANGE - mFoldWeight - mCallWeight);
	actionCumWeights = buildActionCumWeights();

	assert(0.0 <= foldProba && foldProba < 1.0);
	assert(0.0 <= callProba && callProba <= 1.0);
	assert(!(foldProba == 0.0 && callProba == 0.0));
}

std::array<std::array<unsigned, egn::GameState::nLegalCases>,
	egn::N_ACTIONS> RandomAI::buildActionCumWeights()
{
	std::array<std::array<unsigned, egn::GameState::nLegalCases>,
		egn::N_ACTIONS> res{};

	for (uint8_t i = 0; i < egn::GameState::nLegalCases; ++i) {

		// Put the weight depending on the mask (default: 0).
		if (egn::GameState::actionMasks[i][egn::FOLD])
			res[i][egn::FOLD] = mFoldWeight;
		if (egn::GameState::actionMasks[i][egn::CALL])
			res[i][egn::CALL] = mCallWeight;
		if (egn::GameState::actionMasks[i][egn::RAISE])
			res[i][egn::RAISE] = mRaiseWeight;

		// Accumulate
		for (uint8_t j = 1; j < egn::N_ACTIONS; ++j)
			res[i][j] += res[i][j - 1];

		// Rescale
		mRandChoice.rescaleCumWeights(res[i]);
	}
	return res;
}

void RandomAI::act(egn::GameState& state)
{
	//ZoneScoped;
	// Pick a random action using the distribution.
	state.action = egn::Action(
		mRandChoice(actionCumWeights[state.legalCase], mRng));

	if (state.action == egn::RAISE) {
		if (state.minRaise < state.allin) {
			mRaiseDist.init(state.minRaise, state.allin);
			state.bet = mRaiseDist(mRng);
		}
		else
			state.bet = state.allin;
	}
}

} // opt