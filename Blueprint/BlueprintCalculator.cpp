#include "BlueprintCalculator.h"

namespace bp {

BlueprintCalculator::BlueprintCalculator(unsigned rngSeed) :

	rng{ (!rngSeed) ? std::random_device{}() : rngSeed },
	pruneCumWeights(buildPruneCumWeights()),

	abcInfo(
		bp::MAX_PLAYERS,
		bp::ANTE,
		bp::BIG_BLIND,
		bp::INITIAL_STAKE,
		bp::BET_SIZES,
		bp::BLUEPRINT_NAME),

	regrets({
		std::vector<std::vector<regret_t>>(abc::PREFLOP_SIZE, std::vector<regret_t>(abcInfo.preflopNActionSeqs())),
		std::vector<std::vector<regret_t>>(bp::N_BCK, std::vector<regret_t>(abcInfo.flopNActionSeqs())),
		std::vector<std::vector<regret_t>>(bp::N_BCK, std::vector<regret_t>(abcInfo.turnNActionSeqs())),
		std::vector<std::vector<regret_t>>(bp::N_BCK, std::vector<regret_t>(abcInfo.riverNActionSeqs()))
	}),

	currIter(0)
{
}

std::array<uint8_t, 2> BlueprintCalculator::buildPruneCumWeights()
{
	std::array<uint8_t, 2> res = { pruneProbaPerc, 100 - pruneProbaPerc };
	randChoice.rescaleCumWeights(res);
	return res;
}

void BlueprintCalculator::buildStrategy()
{
	while (currIter < endIter) {

		bool mustPrune = currIter >= pruneBeginIter || randChoice(pruneCumWeights, rng) == 1;
		bool mustUpdatePreflopStrat = currIter && currIter % preflopStratUpdatePeriod == 0;
		for (uint8_t currPlayer = 0; currPlayer < MAX_PLAYERS; ++currPlayer) {
			if (mustPrune) traverseMCCFR();
			else traverseMCCFRP();
			if (mustUpdatePreflopStrat) updatePreflopStrat();
		}

		if (currIter && currIter < discountEndIter && currIter % discountPeriod == 0)
			applyDiscounting();

		++currIter;

		if (currIter >= snapshotBeginIter && (currIter - snapshotBeginIter) % snapshotPeriod == 0)
			takeSnapshot();
		if (currIter % checkpointPeriod == 0)
			updateCheckpoint();
	}

	// Free memory allocated for regrets.
	std::vector<std::vector<std::vector<regret_t>>>().swap(regrets);
	averageSnapshots();
}

void BlueprintCalculator::saveStrategy()
{
}

void BlueprintCalculator::loadStrategy()
{
}

void BlueprintCalculator::updatePreflopStrat()
{

}

void BlueprintCalculator::applyDiscounting()
{
}

void BlueprintCalculator::takeSnapshot()
{
}

void BlueprintCalculator::averageSnapshots()
{
}

void BlueprintCalculator::updateCheckpoint()
{
}

void BlueprintCalculator::traverseMCCFR()
{
}

void BlueprintCalculator::traverseMCCFRP()
{
}

} // bp