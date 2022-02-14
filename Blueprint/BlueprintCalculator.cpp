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

void BlueprintCalculator::buildStrategy()
{
	while (currIter < endIter) {

		bool mustPrune = currIter >= pruneBeginIter || pruneRandChoice(pruneCumWeights, rng) == 1;
		bool mustUpdatePreflopStrat = currIter && currIter % preflopStratUpdatePeriod == 0;

		for (uint8_t traverser = 0; traverser < MAX_PLAYERS; ++traverser) {
			if (mustPrune) traverseMCCFR(traverser);
			else traverseMCCFRP(traverser);
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

std::array<uint8_t, 2> BlueprintCalculator::buildPruneCumWeights()
{
	std::array<uint8_t, 2> res = { pruneProbaPerc, 100 - pruneProbaPerc };
	pruneRandChoice.rescaleCumWeights(res);
	return res;
}

uint8_t BlueprintCalculator::nActions() const
{
	return abcInfo.nActions();
}

regret_t& BlueprintCalculator::getRegret(uint8_t actionId)
{
	return regrets[abcInfo.roundIdx()][abcInfo.handIdx()][abcInfo.actionSeqIds[actionId]];
}

const regret_t BlueprintCalculator::getRegret(uint8_t actionId) const
{
	return regrets[abcInfo.roundIdx()][abcInfo.handIdx()][abcInfo.actionSeqIds[actionId]];
}

void BlueprintCalculator::calculateCumRegrets()
{
	cumRegrets.resize(nActions());
	cumRegrets[0] = (getRegret(0) > 0) ? getRegret(0) : 0;
	for (uint8_t a = 1; a < cumRegrets.size(); a++) {
		if (getRegret(a) > 0)
			cumRegrets[a] = cumRegrets[a - 1] + getRegret(a);
		else
			cumRegrets[a] = cumRegrets[a - 1];
	}
	// If no regret is positive, the random choice will be uniformly distributed.
	if (cumRegrets.back() == 0) {
		for (uint8_t i = 0; i < cumRegrets.size(); ++i)
			cumRegrets[i] = i + 1;
	}
}

sumRegret_t BlueprintCalculator::calculateSumRegrets() const
{
	sumRegret_t sum = 0;
	for (uint8_t a = 0; a < nActions(); ++a) {
		if (getRegret(a) > 0) sum += getRegret(a);
	}
	return sum;
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
			|| !abcInfo.state.isActing(traverser)) {

			// All leafs visited: end of DFS.
			if (stack.empty()) break;

			// Go back to the latest node having children not visited yet.
			bool wasLastChild;
			do {
				hist.pop_back();
				wasLastChild = lastChild.back();
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
				for (uint8_t actionId = 0; actionId < nActions() - 1; ++actionId)
					stack.push_back(actionId);
				// Go to the next node.
				abcInfo.nextState(nActions() - 1);
				hist.push_back(abcInfo);
				// There will always be at least two legal actions, so this is never the last.
				lastChild.push_back(false);
			}
		}
	}
}

void BlueprintCalculator::traverseMCCFR(uint8_t traverser)
{
	abcInfo.startNewHand();
	stack.clear();
	// hist will only contain no-leaf nodes where traverser plays.
	hist.clear();
	// lastChild will only deal with children of nodes where traverser plays.
	lastChild.clear();
	expVals.clear();

	// Do a DFS.
	while (true) {

		// Reached leaf node.
		if (abcInfo.state.finished || !abcInfo.state.isAlive(traverser)) {

			// Add leaf's expected value on stack.
			expVals.push_back(abcInfo.state.reward(traverser));

			// Go back to the latest node having children not visited yet while
			// backpropagating the expected value and updating the regrets.
			bool wasLastChild = lastChild.back();
			lastChild.pop_back();
			abcInfo = hist.back();
			while (wasLastChild) {

				// The expected values of all children have been calculated and
				// we can average them into the parent node's expected value.
				egn::dchips v = calculateExpectedValue();
				// Update the regrets.
				for (uint8_t a = 0; a < nActions(); ++a) {
					getRegret(a) += expVals.back() - v;
					expVals.pop_back();
				}
				// All leafs visited and traverser's regrets updated: end of MCCFR traversal.
				if (stack.empty()) return;
				expVals.push_back(v);

				// Go back to the next parent.
				hist.pop_back();
				wasLastChild = lastChild.back();
				lastChild.pop_back();
				abcInfo = hist.back();
			}

			// Go to the next node.
			uint8_t actionId = stack.back();
			stack.pop_back();
			abcInfo.nextState(actionId);
			lastChild.push_back(actionId == 0);
		}

		// Current node has children.
		else {
			if (abcInfo.state.actingPlayer == traverser) {
				// Add all actions.
				for (uint8_t actionId = 0; actionId < nActions() - 1; ++actionId)
					stack.push_back(actionId);
				// Go to the next node.
				abcInfo.nextState(nActions() - 1);
				// There will always be at least two legal actions, so this is never the last.
				lastChild.push_back(false);
			}
			else {
				// Sample an action with the current strategy.
				calculateCumRegrets();
#pragma warning(suppress: 4244)
				uint8_t actionId = actionRandChoice(cumRegrets, rng);
				// Go to the next node.
				abcInfo.nextState(actionId);
				if (abcInfo.state.actingPlayer == traverser && !abcInfo.state.finished)
					hist.push_back(abcInfo);
			}
		}
	}
}

void BlueprintCalculator::traverseMCCFRP(uint8_t traverser)
{

}

egn::dchips BlueprintCalculator::calculateExpectedValue() const
{
	egn::dchips v = 0;
	sumRegret_t s = calculateSumRegrets();

	// If no regret is positive, all actions have the same proba,
	// so we take the arithmetic mean of the expected values.
	if (s == 0) {
		for (uint8_t a = 0; a < nActions(); ++a)
			v += expVals.rbegin()[a];
		v /= nActions();
	}

	else {
		for (uint8_t a = 0; a < nActions(); ++a) {
			if (getRegret(a) > 0)
				v += getRegret(a) * expVals.rbegin()[a];
		}
		v /= s;
	}

	return v;
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

} // bp