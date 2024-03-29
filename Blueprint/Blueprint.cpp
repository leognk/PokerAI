#include "Blueprint.h"

namespace bp {

Blueprint::Blueprint(
	const std::string& blueprintGameName,
	const std::string& blueprintBuildName,
	unsigned rngSeed) :

	bpGameName(blueprintGameName),
	bpName(blueprintName(blueprintGameName, blueprintBuildName)),
	rng{ (!rngSeed) ? std::random_device{}() : rngSeed }
{
}

void Blueprint::loadStrat()
{
	const abc::ActionSeqSize seqSizes(bpGameName);

	// Allocate memory for the strategy.
	strat = {
		std::vector<std::vector<strat_t>>(N_BCK_PREFLOP, std::vector<strat_t>(seqSizes.preflopSize)),
		std::vector<std::vector<strat_t>>(N_BCK_FLOP, std::vector<strat_t>(seqSizes.flopSize)),
		std::vector<std::vector<strat_t>>(N_BCK_TURN, std::vector<strat_t>(seqSizes.turnSize)),
		std::vector<std::vector<strat_t>>(N_BCK_RIVER, std::vector<strat_t>(seqSizes.riverSize))
	};

	// Load the strategy.
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r)
		opt::load2DVector(strat[r], bp::stratPath(bpName, r));
}

void Blueprint::loadRegrets()
{
	const abc::ActionSeqSize seqSizes(bpGameName);

	// Allocate memory for the regrets.
	regrets = {
		std::vector<std::vector<regret_t>>(N_BCK_PREFLOP, std::vector<regret_t>(seqSizes.preflopSize)),
		std::vector<std::vector<regret_t>>(N_BCK_FLOP, std::vector<regret_t>(seqSizes.flopSize)),
		std::vector<std::vector<regret_t>>(N_BCK_TURN, std::vector<regret_t>(seqSizes.turnSize)),
		std::vector<std::vector<regret_t>>(N_BCK_RIVER, std::vector<regret_t>(seqSizes.riverSize))
	};

	// Load the regrets from the checkpoint file.
	auto file = opt::fstream(bp::checkpointPath(bpName), std::ios::in | std::ios::binary);
	opt::load3DVector(regrets, file);
	file.close();
}

} // bp