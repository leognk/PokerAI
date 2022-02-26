#include "Blueprint.h"

namespace bp {

const abc::ActionSeqSize Blueprint::seqSizes(bp::BLUEPRINT_GAME_NAME);
const strats_t Blueprint::strat(loadStrat());

strats_t Blueprint::loadStrat()
{
	// Allocate memory for the strategy.
	strats_t res = {
		std::vector<std::vector<strat_t>>(N_BCK_PREFLOP, std::vector<strat_t>(seqSizes.preflopSize)),
		std::vector<std::vector<strat_t>>(N_BCK_FLOP, std::vector<strat_t>(seqSizes.flopSize)),
		std::vector<std::vector<strat_t>>(N_BCK_TURN, std::vector<strat_t>(seqSizes.turnSize)),
		std::vector<std::vector<strat_t>>(N_BCK_RIVER, std::vector<strat_t>(seqSizes.riverSize))
	};

	// Load the strategy.
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r)
		opt::load2DVector(strat[r], BlueprintCalculator::getStratPath(r));

	return res;
}

} // bp