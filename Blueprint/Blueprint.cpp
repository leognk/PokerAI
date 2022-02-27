#include "Blueprint.h"

namespace bp {

Blueprint::Blueprint(
	const std::string& blueprintGameName,
	const std::string& blueprintBuildName,
	unsigned rngSeed) :

	strat(loadStrat(blueprintGameName, blueprintBuildName)),
	rng{ (!rngSeed) ? std::random_device{}() : rngSeed }
{
}

strats_t Blueprint::loadStrat(const std::string& blueprintGameName, const std::string& blueprintBuildName)
{
	const abc::ActionSeqSize seqSizes(blueprintGameName);

	// Allocate memory for the strategy.
	strats_t res = {
		std::vector<std::vector<strat_t>>(N_BCK_PREFLOP, std::vector<strat_t>(seqSizes.preflopSize)),
		std::vector<std::vector<strat_t>>(N_BCK_FLOP, std::vector<strat_t>(seqSizes.flopSize)),
		std::vector<std::vector<strat_t>>(N_BCK_TURN, std::vector<strat_t>(seqSizes.turnSize)),
		std::vector<std::vector<strat_t>>(N_BCK_RIVER, std::vector<strat_t>(seqSizes.riverSize))
	};

	// Load the strategy.
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r)
		opt::load2DVector(res[r], BlueprintCalculator::stratPath(
			blueprintName(blueprintGameName, blueprintBuildName), r));

	return res;
}

strat_t Blueprint::getProba(const abcInfo_t& abcInfo, uint8_t actionId)
{
	return strat[abcInfo.roundIdx()][abcInfo.handIdx()][abcInfo.actionSeqIds[actionId]];
}

void Blueprint::calculateCumProbas(const abcInfo_t& abcInfo)
{
	cumProbas.resize(abcInfo.nActions());
	cumProbas[0] = getProba(abcInfo, 0);
	for (uint8_t a = 1; a < cumProbas.size(); ++a)
		cumProbas[a] = cumProbas[a - 1] + getProba(abcInfo, a);

	// If all probas are null, the random choice will be uniformly distributed.
	if (cumProbas.back() == 0) {
		for (uint8_t i = 0; i < cumProbas.size(); ++i)
			cumProbas[i] = i + 1;
	}
}

uint8_t Blueprint::chooseAction(const abcInfo_t& abcInfo)
{
	calculateCumProbas(abcInfo);
	return (uint8_t)actionRandChoice(cumProbas, rng);
}

} // bp