#include "BlueprintCalculator.h"
#include "../Utils/StringManip.h"

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

	gpSeqs(
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

	preflopFinalStrat(abc::PREFLOP_SIZE, std::vector<sumRegret_t>(abcInfo.preflopNActionSeqs())),

	currIter(0),
	nextSnapshotId(1)
{
	gpSeqs.load();
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
	for (auto& handStrat : preflopFinalStrat) {
		for (sumRegret_t& strat : handStrat)
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
			uint8_t a = stack.back();
			stack.pop_back();
			abcInfo.nextState(a);
			hist.push_back(abcInfo);
			lastChild.push_back(a == 0);
		}

		// Current node has children.
		else {
			if (abcInfo.state.actingPlayer == traverser) {
				// Sample an action with the current strategy.
				calculateCumRegrets();
#pragma warning(suppress: 4244)
				uint8_t a = actionRandChoice(cumRegrets, rng);
				// Increment the final strategy.
				++preflopFinalStrat[abcInfo.handIdx()][abcInfo.actionSeqIds[a]];
				// Go to the next node.
				abcInfo.nextState(a);
				hist.push_back(abcInfo);
				lastChild.push_back(true);
			}
			else {
				// Add all actions.
				for (uint8_t a = 0; a < nActions() - 1; ++a)
					stack.push_back(a);
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
					if (getRegret(a) += expVals.back() - v < minRegret)
						getRegret(a) = minRegret;
					expVals.pop_back();
				}
				// All leafs visited and traverser's regrets updated: end of MCCFR traversal.
				if (stack.empty()) return;
				expVals.push_back(v);

				// Go back to the previous parent.
				hist.pop_back();
				wasLastChild = lastChild.back();
				lastChild.pop_back();
				abcInfo = hist.back();
			}

			// Go to the next node.
			uint8_t a = stack.back();
			stack.pop_back();
			abcInfo.nextState(a);
			lastChild.push_back(a == 0);
		}

		// Current node has children.
		else {
			if (abcInfo.state.actingPlayer == traverser) {
				// Add all actions.
				for (uint8_t a = 0; a < nActions() - 1; ++a)
					stack.push_back(a);
				// Go to the next node.
				abcInfo.nextState(nActions() - 1);
				// There will always be at least two legal actions, so this is never the last.
				lastChild.push_back(false);
			}
			else {
				// Sample an action with the current strategy.
				calculateCumRegrets();
#pragma warning(suppress: 4244)
				uint8_t a = actionRandChoice(cumRegrets, rng);
				// Go to the next node.
				abcInfo.nextState(a);
				if (abcInfo.state.actingPlayer == traverser && !abcInfo.state.finished)
					hist.push_back(abcInfo);
			}
		}
	}
}

void BlueprintCalculator::traverseMCCFRP(uint8_t traverser)
{
	abcInfo.startNewHand();
	stack.clear();
	// hist will only contain no-leaf nodes where traverser plays.
	hist.clear();
	// lastChild will only deal with children of nodes where traverser plays.
	lastChild.clear();
	expVals.clear();
	// visited will only deal with children of nodes where traverser plays.
	visited.clear();

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
					if (visited.rbegin()[nActions() - 1 + a]) {
						getRegret(a) += expVals.back() - v;
						expVals.pop_back();
					}
				}
				// All leafs visited and traverser's regrets updated: end of MCCFR traversal.
				if (stack.empty()) return;
				// Remove the last nActions elements.
				visited.resize(visited.size() - nActions());
				expVals.push_back(v);

				// Go back to the previous parent.
				hist.pop_back();
				wasLastChild = lastChild.back();
				lastChild.pop_back();
				abcInfo = hist.back();
			}

			// Go to the next node.
			uint8_t a = stack.back();
			stack.pop_back();
			abcInfo.nextState(a);
			lastChild.push_back(a == 0);
		}

		// Current node has children.
		else {
			if (abcInfo.state.actingPlayer == traverser) {
				// Add all actions.
				for (uint8_t a = 0; a < nActions(); ++a) {
					const auto action = abcInfo.actionAbc.legalActions[a];
					// Prune only if the action is not on the last betting
					// round or does not lead to a terminal node.
					visited.push_back(action == abc::FOLD || action == abc::ALLIN
						|| abcInfo.state.round == egn::RIVER
						|| getRegret(a) > pruneThreshold
						|| (action == abc::CALL && abcInfo.state.call == abcInfo.state.stakes[traverser]));
					if (visited.back())
						stack.push_back(a);
				}
				// Go to the next node.
				uint8_t a = stack.back();
				stack.pop_back();
				abcInfo.nextState(a);
				lastChild.push_back(a == 0);
			}
			else {
				// Sample an action with the current strategy.
				calculateCumRegrets();
#pragma warning(suppress: 4244)
				uint8_t a = actionRandChoice(cumRegrets, rng);
				// Go to the next node.
				abcInfo.nextState(a);
				if (abcInfo.state.actingPlayer == traverser && !abcInfo.state.finished)
					hist.push_back(abcInfo);
			}
		}
	}
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

// Save the current strategy of the rounds after the preflop on the disk.
void BlueprintCalculator::takeSnapshot()
{
	for (uint8_t r = 1; r < egn::N_ROUNDS; ++r) {

		// Open the file.
		const std::string path = snapshotPath
			+ std::to_string(nextSnapshotId)
			+ "_" + opt::toUpper(egn::roundToString(egn::Round(r))) + ".bin";
		auto file = std::fstream(path, std::ios::out | std::ios::binary);

		// Write in the file.
		for (bckSize_t handIdx = 0;  handIdx < regrets[r].size(); ++handIdx) {

			abc::GroupedActionSeqs::seqIdx_t currSeq = 0;

			for (const uint8_t nLegalActions : gpSeqs.lens[r]) {

				// Calculate the sum of the positive regrets of the legal actions.
				sumRegret_t sum = 0;
				auto tmpCurrSeq = currSeq;
				for (uint8_t i = 0; i < nLegalActions; ++i) {
					auto seqIdx = gpSeqs.seqs[r][tmpCurrSeq];
					regret_t regret = regrets[r][handIdx][seqIdx];
					if (regret > 0) sum += regret;
					++tmpCurrSeq;
				}

				// Normalize the regrets of the legal actions and write them in the file.
				for (uint8_t i = 0; i < nLegalActions; ++i) {
					auto seqIdx = gpSeqs.seqs[r][tmpCurrSeq];
					regret_t strat = regrets[r][handIdx][seqIdx];
					if (strat > 0) strat /= sum;
					else strat = 0;
					file.write((char*)&strat, sizeof(strat));
					++currSeq;
				}
			}
		}

		file.close();
	}

	++nextSnapshotId;
}

void BlueprintCalculator::averageSnapshots()
{

}

void BlueprintCalculator::updateCheckpoint()
{

}

} // bp