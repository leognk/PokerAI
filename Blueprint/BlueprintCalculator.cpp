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
	pruneRandChoice.rescaleCumWeights(res);
	return res;
}

void BlueprintCalculator::buildStrategy()
{
	while (currIter < endIter) {

		bool mustPrune = currIter >= pruneBeginIter || pruneRandChoice(pruneCumWeights, rng) == 1;
		bool mustUpdatePreflopStrat = currIter && currIter % preflopStratUpdatePeriod == 0;

		for (uint8_t traverser = 0; traverser < MAX_PLAYERS; ++traverser) {
			if (mustPrune) traverseMCCFR();
			else traverseMCCFRP();
			if (mustUpdatePreflopStrat) updatePreflopStrat(traverser);
		}

		if (currIter && currIter < discountEndIter && currIter % discountPeriod == 0)
			applyDiscounting();

		++currIter;

		if (currIter >= snapshotBeginIter && (currIter - snapshotBeginIter) % snapshotPeriod == 0)
			takeSnapshot();
		if (currIter % checkpointPeriod == 0)
			updateCheckpoint();
	}

	// Free allocated memory for regrets.
	std::vector<std::vector<std::vector<regret_t>>>().swap(regrets);
	averageSnapshots();
}

void BlueprintCalculator::saveStrategy()
{

}

void BlueprintCalculator::loadStrategy()
{

}

void BlueprintCalculator::updateActionRegrets()
{
	actionRegrets.resize(abcInfo.nActions());
	for (uint8_t i = 0; i < actionRegrets.size(); ++i)
		actionRegrets[i] = regrets[abcInfo.roundIdx()][abcInfo.handIdx()][abcInfo.actionSeqIds[i]];
}

void BlueprintCalculator::calculateCumRegrets()
{
	cumRegrets.resize(abcInfo.nActions());
	cumRegrets[0] = (actionRegrets[0] > 0) ? actionRegrets[0] : 0;
	for (uint8_t actionId = 1; actionId < cumRegrets.size(); actionId++) {
		if (actionRegrets[actionId] > 0)
			cumRegrets[actionId] = cumRegrets[actionId - 1] + actionRegrets[actionId];
		else
			cumRegrets[actionId] = cumRegrets[actionId - 1];
	}
	// If no regret is positive, the random choice will be uniformly distributed.
	if (cumRegrets.back() == 0) {
		for (uint8_t i = 0; i < cumRegrets.size(); ++i)
			cumRegrets[i] = i + 1;
	}
}

sumRegret_t BlueprintCalculator::calculateSumRegrets()
{
	sumRegret_t sum = 0;
	for (const regret_t& regret : actionRegrets) {
		if (regret > 0) sum += regret;
	}
	return sum;
}

void BlueprintCalculator::updatePreflopStrat(uint8_t traverser)
{
	abcInfo.startNewHand();
	stack.clear();
	hist.clear();
	lastChild.clear();

	// Do a DFS.
	while (true) {

		// Reached leaf node.
		if (abcInfo.state.finished || abcInfo.state.round != egn::PREFLOP
			|| abcInfo.state.isActing(traverser)) {

			// All leaf nodes visited: end of DFS.
			if (stack.empty()) break;

			// Go back to the latest node having children not visited yet.
			bool wasLastChild;
			do {
				wasLastChild = lastChild.back();
				hist.pop_back();
				lastChild.pop_back();
			} while (wasLastChild);
			abcInfo = hist.back();

			// Go to the next node.
			uint8_t actionId = stack.back();
			stack.pop_back();
			abcInfo.nextState(actionId);
			hist.push_back(abcInfo);
			lastChild.push_back(actionId == 0);
		}

		// Current node has children.
		else {

			if (abcInfo.state.actingPlayer == traverser) {

				// Sample an action with the current strategy.
				updateActionRegrets();
				calculateCumRegrets();
#pragma warning(suppress: 4244)
				uint8_t actionId = actionRandChoice(cumRegrets, rng);

				// Increment the final strategy.
				++finalStrat[egn::PREFLOP][abcInfo.handIdx()][abcInfo.actionSeqIds[actionId]];

				// Go to the next node.
				abcInfo.nextState(actionId);
				hist.push_back(abcInfo);
				lastChild.push_back(true);
			}

			else {

				// Add all actions.
				if (abcInfo.nActions() > 1) {
					for (uint8_t actionId = 0; actionId < abcInfo.nActions() - 1; ++actionId)
						stack.push_back(actionId);
				}

				// Go to the next node.
				uint8_t actionId = abcInfo.nActions() - 1;
				abcInfo.nextState(actionId);
				hist.push_back(abcInfo);
				lastChild.push_back(actionId == 0);
			}
		}
	}
}

void BlueprintCalculator::applyDiscounting()
{
	// currIter is divisible by discountPeriod.
	float d = 1 - 1 / (float)(currIter / discountPeriod + 1);

	// Discount regrets.
	for (auto& roundRegrets : regrets) {
		for (auto& handRegrets : roundRegrets) {
			for (regret_t& regret : handRegrets)
				regret = std::round(regret * d);
		}
	}

	// Discount final strategy on preflop.
	// (discount is done before taking the snapshots for the other rounds' strategies)
	for (auto& handStrat : finalStrat[egn::PREFLOP]) {
		for (strat_t& strat : handStrat)
			strat = std::round(strat * d);
	}
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