#include "BlueprintCalculator.h"

namespace bp {

BlueprintCalculator::BlueprintCalculator(unsigned rngSeed, bool verbose) :

	rng{ (!rngSeed) ? std::random_device{}() : rngSeed },
	verbose(verbose),
	pruneCumWeights(buildPruneCumWeights()),

	abcInfo(
		MAX_PLAYERS,
		ANTE,
		BIG_BLIND,
		INITIAL_STAKE,
		BET_SIZES,
		BLUEPRINT_NAME,
		rngSeed),

	gpSeqs(
		MAX_PLAYERS,
		ANTE,
		BIG_BLIND,
		INITIAL_STAKE,
		BET_SIZES,
		BLUEPRINT_NAME),

	currIter(0),
	nextSnapshotId(1)
{
	// Allocate memory for the regrets.
	regrets = {
		std::vector<std::vector<regret_t>>(N_BCK_PREFLOP, std::vector<regret_t>(nActionSeqIds(egn::PREFLOP))),
		std::vector<std::vector<regret_t>>(N_BCK_FLOP, std::vector<regret_t>(nActionSeqIds(egn::FLOP))),
		std::vector<std::vector<regret_t>>(N_BCK_TURN, std::vector<regret_t>(nActionSeqIds(egn::TURN))),
		std::vector<std::vector<regret_t>>(N_BCK_RIVER, std::vector<regret_t>(nActionSeqIds(egn::RIVER)))
	};

	// Allocate memory for the preflop strategy.
	preflopStrat = std::vector<std::vector<sumRegret_t>>(
		N_BCK_PREFLOP, std::vector<sumRegret_t>(nActionSeqIds(egn::PREFLOP)));

	gpSeqs.load();
}

void BlueprintCalculator::buildStrategy()
{
	startTime = opt::getTime();

	while (currIter < endIter) oneIter();

	// Free memory allocated for regrets.
	std::vector<std::vector<std::vector<regret_t>>>().swap(regrets);
	// Perform final calculations for the final strategy and save it to the disk.
	normalizePreflopStrat();
	averageSnapshots();

	printFinalStats();
}

void BlueprintCalculator::oneIter()
{
	bool mustPrune = currIter >= pruneBeginIter && pruneRandChoice(pruneCumWeights, rng) == 0;
	bool mustUpdatePreflopStrat = currIter && (currIter % preflopStratUpdatePeriod == 0);

	for (uint8_t traverser = 0; traverser < MAX_PLAYERS; ++traverser) {
		if (mustPrune) traverseMCCFRP(traverser);
		else traverseMCCFR(traverser);
		if (mustUpdatePreflopStrat) updatePreflopStrat(traverser);
	}

	if (currIter && currIter < discountEndIter && currIter % discountPeriod == 0)
		applyDiscounting();

	++currIter;

	if (currIter >= snapshotBeginIter && (currIter - snapshotBeginIter) % snapshotPeriod == 0)
		takeSnapshot();
	if (currIter % checkpointPeriod == 0)
		updateCheckpoint();
	if (verbose && currIter && (currIter % printPeriod == 0 || currIter == endIter))
		printProgress();
}

std::array<uint8_t, 2> BlueprintCalculator::buildPruneCumWeights()
{
	std::array<uint8_t, 2> res = { pruneProbaPerc, 100 };
	pruneRandChoice.rescaleCumWeights(res);
	return res;
}

size_t BlueprintCalculator::nHandIds(egn::Round round) const
{
	switch (round) {
	case egn::PREFLOP: return N_BCK_PREFLOP;
	case egn::FLOP: return N_BCK_FLOP;
	case egn::TURN: return N_BCK_TURN;
	case egn::RIVER: return N_BCK_RIVER;
	default: throw std::runtime_error("Unknown round.");
	}
}

size_t BlueprintCalculator::nActionSeqIds(egn::Round round) const
{
	switch (round) {
	case egn::PREFLOP: return abcInfo.preflopNActionSeqs();
	case egn::FLOP: return abcInfo.flopNActionSeqs();
	case egn::TURN: return abcInfo.turnNActionSeqs();
	case egn::RIVER: return abcInfo.riverNActionSeqs();
	default: throw std::runtime_error("Unknown round.");
	}
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
#pragma warning(suppress: 4244)
				regret = std::round(regret * d);
		}
	}

	// Discount final strategy on preflop.
	// (discount is done before taking the snapshots for the other rounds' strategies)
	for (auto& handStrat : preflopStrat) {
		for (sumRegret_t& strat : handStrat)
#pragma warning(suppress: 4244)
			strat = std::round(strat * d);
	}
}

void BlueprintCalculator::updatePreflopStrat(uint8_t traverser)
{
	stack.clear();
	hist.clear();
	lastChild.clear();

	abcInfo.startNewHand();

	// Do a DFS.
	while (true) {

		hist.push_back(abcInfo);

		// Reached leaf node.
		if (abcInfo.state.finished || abcInfo.state.round != egn::PREFLOP
			|| !abcInfo.state.isActing(traverser)) {

			// All leafs visited: end of DFS.
			if (stack.empty()) return;

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
				++preflopStrat[abcInfo.handIdx()][abcInfo.actionSeqIds[a]];
				// Go to the next node.
				abcInfo.nextState(a);
				lastChild.push_back(true);
			}
			else {
				// Add all actions.
				for (uint8_t a = 0; a < nActions() - 1; ++a)
					stack.push_back(a);
				// Go to the next node.
				abcInfo.nextState(nActions() - 1);
				// There will always be at least two legal actions, so this is never the last.
				lastChild.push_back(false);
			}
		}
	}
}

void BlueprintCalculator::traverseMCCFR(uint8_t traverser)
{
	stack.clear();
	// hist will only contain no-leaf nodes where traverser plays.
	hist.clear();
	// lastChild will only deal with children of nodes where traverser plays.
	lastChild.clear();
	expVals.clear();

	abcInfo.startNewHand();

	// Do a DFS.
	while (true) {

		// Add no-leaf nodes where traverser plays to hist.
		if (abcInfo.state.actingPlayer == traverser && !abcInfo.state.finished)
			hist.push_back(abcInfo);

		// Reached leaf node.
		if (abcInfo.state.finished || !abcInfo.state.isAlive(traverser)) {

			// This can happen if everybody folds except the bb who is also the traverser.
			if (lastChild.empty()) return;

			// Add leaf's expected value on stack.
			expVals.push_back(abcInfo.state.reward(traverser));

			// Go back to the latest node having children not visited yet while
			// backpropagating the expected value and updating the regrets.
			abcInfo = hist.back();
			bool wasLastChild = lastChild.back();
			lastChild.pop_back();
			while (wasLastChild) {

				// The expected values of all children have been calculated and
				// we can average them into the parent node's expected value.
				egn::dchips v = calculateExpectedValue();
				// Update the regrets.
				for (uint8_t a = 0; a < nActions(); ++a) {
					if ((getRegret(a) += expVals.back() - v) < minRegret)
						getRegret(a) = minRegret;
					expVals.pop_back();
				}
				// All leafs visited and traverser's regrets updated: end of MCCFR traversal.
				if (lastChild.empty()) return;
				expVals.push_back(v);

				// Go back to the previous parent.
				hist.pop_back();
				abcInfo = hist.back();
				wasLastChild = lastChild.back();
				lastChild.pop_back();
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
			}
		}
	}
}

void BlueprintCalculator::traverseMCCFRP(uint8_t traverser)
{
	stack.clear();
	// hist will only contain no-leaf nodes where traverser plays.
	hist.clear();
	// lastChild will only deal with children of nodes where traverser plays.
	lastChild.clear();
	expVals.clear();
	// visited will only deal with children of nodes where traverser plays.
	visited.clear();

	abcInfo.startNewHand();

	// Do a DFS.
	while (true) {

		// Add no-leaf nodes where traverser plays to hist.
		if (abcInfo.state.actingPlayer == traverser && !abcInfo.state.finished)
			hist.push_back(abcInfo);

		// Reached leaf node.
		if (abcInfo.state.finished || !abcInfo.state.isAlive(traverser)) {

			// This can happen if everybody folds but the bb who is also the traverser.
			if (lastChild.empty()) return;

			// Add leaf's expected value on stack.
			expVals.push_back(abcInfo.state.reward(traverser));

			// Go back to the latest node having children not visited yet while
			// backpropagating the expected value and updating the regrets.
			abcInfo = hist.back();
			bool wasLastChild = lastChild.back();
			lastChild.pop_back();
			while (wasLastChild) {

				// The expected values of all children have been calculated and
				// we can average them into the parent node's expected value.
				egn::dchips v = calculateExpectedValue();
				// Update the regrets.
				for (uint8_t a = 0; a < nActions(); ++a) {
					if (visited.rbegin()[nActions() - 1 - a]) {
						getRegret(a) += expVals.back() - v;
						expVals.pop_back();
					}
				}
				// All leafs visited and traverser's regrets updated: end of MCCFR traversal.
				if (lastChild.empty()) return;
				// Remove the last nActions elements.
				visited.resize(visited.size() - nActions());
				expVals.push_back(v);

				// Go back to the previous parent.
				hist.pop_back();
				abcInfo = hist.back();
				wasLastChild = lastChild.back();
				lastChild.pop_back();
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
		auto file = std::fstream(
			getSnapshotPath(nextSnapshotId, r), std::ios::out | std::ios::binary);

		// Write in the file.
		// Loop over all the regrets of the round.
		for (bckSize_t handIdx = 0;  handIdx < regrets[r].size(); ++handIdx) {

			abc::GroupedActionSeqs::seqIdx_t currSeq = 0;

			for (const uint8_t nLegalActions : gpSeqs.lens[r]) {

				// Calculate the cumulated sums of the positive regrets
				// of the legal actions.
				cumRegrets.resize(nLegalActions);
				auto seqIdx = gpSeqs.seqs[r][currSeq];
				regret_t regret = regrets[r][handIdx][seqIdx];
				cumRegrets[0] = (regret > 0) ? regret : 0;
				++currSeq;
				for (uint8_t a = 1; a < nLegalActions; ++a) {
					seqIdx = gpSeqs.seqs[r][currSeq];
					regret = regrets[r][handIdx][seqIdx];
					if (regret > 0)
						cumRegrets[a] = cumRegrets[a - 1] + regret;
					else
						cumRegrets[a] = cumRegrets[a - 1];
					++currSeq;
				}
				// If no regret is positive, the strategy is to choose
				// with a uniform distribution.
				if (cumRegrets.back() == 0) {
					for (uint8_t i = 0; i < cumRegrets.size(); ++i)
						cumRegrets[i] = i + 1;
				}

				// Normalize the regrets and write them in the file.
				cumWeightsRescaler.rescaleCumWeights(cumRegrets);
				strat_t strat = cumRegrets[0];
				file.write((char*)&strat, sizeof(strat));
				for (uint8_t i = 1; i < nLegalActions; ++i) {
					strat = cumRegrets[i] - cumRegrets[i - 1];
					file.write((char*)&strat, sizeof(strat));
				}
			}
		}

		file.close();
	}

	++nextSnapshotId;
}

// Average the snapshots into the final strategy for the rounds
// after the preflop and save it to the disk.
void BlueprintCalculator::averageSnapshots()
{
	for (uint8_t r = 1; r < egn::N_ROUNDS; ++r) {

		// Allocate memory for the sum of the snapshots' strategies.
		std::vector<std::vector<sumStrat_t>> sumStrats(
			nHandIds(egn::Round(r)), std::vector<sumStrat_t>(nActionSeqIds(egn::Round(r))));

		// Calculate the sum of the snapshots' strategies.
		for (unsigned snapshotId = 1; snapshotId <= nSnapshots; ++snapshotId) {

			// Open the snapshot.
			auto snapshotFile = std::fstream(
				getSnapshotPath(snapshotId, r), std::ios::in | std::ios::binary);

			// Add the snapshot's strategy to the total sum.
			for (bckSize_t handIdx = 0; handIdx < sumStrats.size(); ++handIdx) {
				for (size_t seqIdx = 0; seqIdx < sumStrats[0].size(); ++seqIdx) {
					strat_t strat;
					snapshotFile.read((char*)&strat, sizeof(strat));
					sumStrats[handIdx][seqIdx] += strat;
				}
			}

			snapshotFile.close();
		}

		// Open the file.
		auto file = std::fstream(getStratPath(r), std::ios::out | std::ios::binary);

		// Write the average of the snapshots' strategies.
		for (const auto& handSumStrats : sumStrats) {
			for (const sumStrat_t& sumStrat : handSumStrats) {
#pragma warning(suppress: 4244)
				strat_t strat = std::round((double)sumStrat / nSnapshots);
				file.write((char*)&strat, sizeof(strat));
			}
		}

		file.close();
	}
}

void BlueprintCalculator::normalizePreflopStrat()
{
	// Open the file.
	auto file = std::fstream(getStratPath(egn::PREFLOP), std::ios::out | std::ios::binary);

	// Normalize the final strategy and save it to the disk.
	// Loop over the strategy.
	for (bckSize_t handIdx = 0; handIdx < preflopStrat.size(); ++handIdx) {

		abc::GroupedActionSeqs::seqIdx_t currSeq = 0;

		for (const uint8_t nLegalActions : gpSeqs.lens[egn::PREFLOP]) {

			// Calculate the cumulated sums of the non-normalized probas
			// of the legal actions.
			cumProbas.resize(nLegalActions);
			auto seqIdx = gpSeqs.seqs[egn::PREFLOP][currSeq];
			sumRegret_t proba = preflopStrat[handIdx][seqIdx];
			++currSeq;
			for (uint8_t a = 1; a < nLegalActions; ++a) {
				seqIdx = gpSeqs.seqs[egn::PREFLOP][currSeq];
				proba = preflopStrat[handIdx][seqIdx];
				cumProbas[a] = cumProbas[a - 1] + proba;
				++currSeq;
			}
			// If all probas are null, the strategy is to choose
			// with a uniform distribution.
			if (cumProbas.back() == 0) {
				for (uint8_t i = 0; i < cumProbas.size(); ++i)
					cumProbas[i] = i + 1;
			}

			// Normalize the probas and write them in the file.
			cumWeightsRescaler.rescaleCumWeights(cumProbas);
			strat_t strat = cumProbas[0];
			file.write((char*)&strat, sizeof(strat));
			for (uint8_t i = 1; i < nLegalActions; ++i) {
				strat = cumProbas[i] - cumProbas[i - 1];
				file.write((char*)&strat, sizeof(strat));
			}
		}
	}

	file.close();
}

std::string BlueprintCalculator::getSnapshotPath(unsigned snapshotId, uint8_t roundId)
{
	return snapshotPath + "_" + std::to_string(snapshotId)
		+ "_" + opt::toUpper(egn::roundToString(egn::Round(roundId))) + ".bin";
}

std::string BlueprintCalculator::getStratPath(uint8_t roundId)
{
	return stratPath
		+ "_" + opt::toUpper(egn::roundToString(egn::Round(roundId))) + ".bin";
}

void BlueprintCalculator::updateCheckpoint()
{

}

void BlueprintCalculator::printProgress() const
{
	static const std::string endIterStr = opt::prettyBigNum(endIter, 1);
	const std::string currIterStr = opt::prettyBigNum(currIter, 1);
	const std::string duration = opt::prettyDuration(startTime);
	const std::string vm = opt::prettyBigNum(opt::virtualMemUsedByMe(), 1, true) + "o";
	const std::string ram = opt::prettyBigNum(opt::physMemUsedByMe(), 1, true) + "o";

	std::cout
		<< "iter: " << std::setw(6) << currIterStr << " / " << endIterStr
		<< " | " << duration
		<< " | VM: " << vm
		<< " | RAM: " << ram << "\n";
}

void BlueprintCalculator::printFinalStats() const
{
	const std::string duration = opt::prettyDuration(startTime);
	std::cout << "\nDuration: " << duration << "\n";
}

} // bp