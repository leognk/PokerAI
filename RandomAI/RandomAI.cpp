#include "RandomAI.h"
#include <random>
#include <numeric>

namespace opt {

RandomAI::RandomAI(
	double foldProba, double callProba,
	unsigned rngSeed) :

	mFoldProba(foldProba),
	mCallProba(callProba),
	mRaiseProba(1 - foldProba - callProba),
	actionProbas(makeActionProbas()),
	mRng{ (!rngSeed) ? std::random_device{}() : rngSeed }
{
	assert(0 <= foldProba && foldProba <= 1);
	assert(0 <= callProba && callProba <= 1);
	assert(!(foldProba == 1 && callProba == 1));
}

std::array<std::array<double, egn::GameState::nLegalCases>,
	egn::N_ACTIONS> RandomAI::makeActionProbas()
{
	std::array<std::array<double, egn::GameState::nLegalCases>,
		egn::N_ACTIONS> res{};
	for (uint8_t i = 0; i < egn::GameState::nLegalCases; ++i) {
		// Put the proba depending on the mask (default: 0).
		if (egn::GameState::actionMasks[i][egn::FOLD])
			res[i][egn::FOLD] = mFoldProba;
		if (egn::GameState::actionMasks[i][egn::CALL])
			res[i][egn::CALL] = mCallProba;
		if (egn::GameState::actionMasks[i][egn::RAISE])
			res[i][egn::RAISE] = mRaiseProba;
		// Normalize
		double sum = std::accumulate(res[i].begin(), res[i].end(), double(0));
		for (uint8_t j = 0; j < egn::N_ACTIONS; ++j)
			res[i][j] /= sum;
	}
	return res;
}

void RandomAI::act(egn::GameState& state)
{
	//ZoneScoped;
	// Pick a random action using the distribution.
	state.action = egn::Action(mRandChoice(actionProbas[state.legalCase], mRng));

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