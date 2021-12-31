#include "RandomAI.h"
#include <random>
#include <numeric>

namespace opt {

#pragma warning(push)
#pragma warning(disable: 4244)
RandomAI::RandomAI(
	double foldProba, double callProba,
	unsigned rngSeed) :

	mFoldWeight(foldProba * mRandChoice.RANGE),
	mCallWeight(callProba * mRandChoice.RANGE),
	mRaiseWeight(mRandChoice.RANGE - mFoldWeight - mCallWeight),
	actionCumWeights(buildActionCumWeights()),
	mRng{ (!rngSeed) ? std::random_device{}() : rngSeed }
{
	assert(0 <= foldProba && foldProba <= 1);
	assert(0 <= callProba && callProba <= 1);
	assert(!(foldProba == 1 && callProba == 1));
}
#pragma warning(pop)

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

		// Normalize if necessary
		unsigned delta = mRandChoice.RANGE
			- res[i][egn::FOLD]
			- res[i][egn::CALL]
			- res[i][egn::RAISE];
		if (delta) {
			uint8_t nNonNull = bool(res[i][egn::FOLD])
				+ bool(res[i][egn::CALL])
				+ bool(res[i][egn::RAISE]);
			unsigned divDelta = delta / nNonNull;
			unsigned remDelta = delta % nNonNull;
			for (uint8_t j = 0; j < egn::N_ACTIONS; ++j) {
				if (res[i][j]) {
					res[i][j] += divDelta;
					if (remDelta) {
						++res[i][j];
						--remDelta;
					}
				}
			}
		}

		// Accumulate
		for (uint8_t j = 1; j < egn::N_ACTIONS; ++j)
			res[i][j] += res[i][j - 1];
	}
	return res;
}

void RandomAI::act(egn::GameState& state)
{
	ZoneScoped;
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