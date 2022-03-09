#include "BlueprintCalculator.h"

namespace bp {

const std::string BlueprintCalculator::printSep(20, '_');

opt::FastRandomChoice<7> BlueprintCalculator::pruneRandChoice;
opt::FastRandomChoiceRNGRescale<16> BlueprintCalculator::actionRandChoice;
const opt::FastRandomChoice<> BlueprintCalculator::cumWeightsRescaler;

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
		BLUEPRINT_GAME_NAME,
		rngSeed),

	gpSeqs(BLUEPRINT_GAME_NAME),
	gpSeqsInv(BLUEPRINT_GAME_NAME),

	currIter(0),
	extraDuration(0),
	nextSnapshotId(1),
	lastCheckpointIter(0),

	nodesCount(0),
	nodesUniqueCount(0),
	totUniqueNodes(getNUniqueNodes())
{
	// Allocate memory for the regrets.
	regrets = {
		std::vector<std::vector<regret_t>>(N_BCK_PREFLOP, std::vector<regret_t>(abcInfo.nActionSeqs(egn::PREFLOP))),
		std::vector<std::vector<regret_t>>(N_BCK_FLOP, std::vector<regret_t>(abcInfo.nActionSeqs(egn::FLOP))),
		std::vector<std::vector<regret_t>>(N_BCK_TURN, std::vector<regret_t>(abcInfo.nActionSeqs(egn::TURN))),
		std::vector<std::vector<regret_t>>(N_BCK_RIVER, std::vector<regret_t>(abcInfo.nActionSeqs(egn::RIVER)))
	};

	gpSeqs.load();
	gpSeqsInv.load();

	// Create save folders.
	std::filesystem::create_directory(blueprintDir());
	std::filesystem::create_directory(blueprintTmpDir());

	// If a checkpoint file is found, resume from it.
	auto checkpointFile = std::fstream(checkpointPath(), std::ios::in | std::ios::binary);
	if (checkpointFile) {
		// Verify that the constants are the same as the ones used in the checkpoint.
		verifyConstants();
		loadCheckpoint(checkpointFile);
	}
	checkpointFile.close();
	writeConstants();
}

void BlueprintCalculator::buildStrategy()
{
	startTime = opt::getTime();

	while (currIter < endIter) oneIter();

	// Free memory allocated for regrets.
	opt::freeVectMem(regrets);
	// Perform final calculations for the final strategy and save it to the disk.
	averageSnapshots();

	printFinalStats();
}

void BlueprintCalculator::oneIter()
{
	bool mustPrune = currIter >= pruneBeginIter && pruneRandChoice(pruneCumWeights, rng) == 0;

	for (uint8_t traverser = 0; traverser < MAX_PLAYERS; ++traverser) {
		if (mustPrune) traverseMCCFRP(traverser);
		else traverseMCCFR(traverser);
	}

	if (currIter && currIter < discountEndIter && currIter % discountPeriod == 0)
		applyDiscounting();

	++currIter;

	if (currIter >= snapshotBeginIter && (currIter - snapshotBeginIter) % snapshotPeriod == 0)
		takeSnapshot();
	if (currIter % checkpointPeriod == 0 || currIter == endIter)
		updateCheckpoint();
	if (verbose && (currIter % printPeriod == 0 || currIter == endIter))
		printProgress();
}

std::array<uint8_t, 2> BlueprintCalculator::buildPruneCumWeights()
{
	std::array<uint8_t, 2> res = { pruneProbaPerc, 100 };
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
	for (uint8_t a = 1; a < cumRegrets.size(); ++a) {
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

int64_t BlueprintCalculator::calculateSumRegrets() const
{
	int64_t sum = 0;
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
				regret = (regret_t)std::round(regret * d);
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
					getRegret(a) += expVals.back() - v;
					if (getRegret(a) < minRegret)
						getRegret(a) = minRegret;
					else if (getRegret(a) > maxRegret)
						throw std::runtime_error("Regret overflow");
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
			const uint8_t a = stack.back();
			incrNodesCount(a);
			abcInfo.nextState(a);
			stack.pop_back();
			lastChild.push_back(a == 0);
		}

		// Current node has children.
		else {
			if (abcInfo.state.actingPlayer == traverser) {
				// Add all actions.
				for (uint8_t a = 0; a < nActions() - 1; ++a)
					stack.push_back(a);
				// Go to the next node.
				const uint8_t a = nActions() - 1;
				incrNodesCount(a);
				abcInfo.nextState(a);
				// There will always be at least two legal actions, so this is never the last.
				lastChild.push_back(false);
			}
			else {
				// Sample an action with the current strategy.
				calculateCumRegrets();
#pragma warning(suppress: 4244)
				// Go to the next node.
				abcInfo.nextState(actionRandChoice(cumRegrets, rng));
			}
		}
	}
}

void BlueprintCalculator::traverseMCCFRP(uint8_t traverser)
{
	stack.clear();
	// firstAction[i] indicates whether the action stack[i] is the first one
	// in the list of legal actions.
	firstAction.clear();
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
				egn::dchips v = calculateExpectedValueP();
				// Update the regrets.
				for (uint8_t a = 0; a < nActions(); ++a) {
					if (visited.rbegin()[nActions() - 1 - a]) {
						getRegret(a) += expVals.back() - v;
						if (getRegret(a) < minRegret)
							getRegret(a) = minRegret;
						else if (getRegret(a) > maxRegret)
							throw std::runtime_error("Regret overflow");
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
			const uint8_t a = stack.back();
			incrNodesCount(a);
			abcInfo.nextState(a);
			stack.pop_back();
			lastChild.push_back(firstAction.back());
			firstAction.pop_back();
		}

		// Current node has children.
		else {
			if (abcInfo.state.actingPlayer == traverser) {
				// Add all actions.
				bool first = true;
				for (uint8_t a = 0; a < nActions(); ++a) {
					const auto action = abcInfo.actionAbc.legalActions[a];
					// Prune only if the action is not on the last betting
					// round or does not lead to a terminal node.
					visited.push_back(action == abc::FOLD || action == abc::ALLIN
						|| abcInfo.state.round == egn::RIVER
						|| getRegret(a) > pruneThreshold
						|| (action == abc::CALL && abcInfo.state.call == abcInfo.state.stakes[traverser]));
					if (visited.back()) {
						stack.push_back(a);
						firstAction.push_back(first);
						first = false;
					}
				}
				// Go to the next node.
				const uint8_t a = stack.back();
				incrNodesCount(a);
				abcInfo.nextState(a);
				stack.pop_back();
				lastChild.push_back(firstAction.back());
				firstAction.pop_back();
			}
			else {
				// Sample an action with the current strategy.
				calculateCumRegrets();
#pragma warning(suppress: 4244)
				// Go to the next node.
				abcInfo.nextState(actionRandChoice(cumRegrets, rng));
			}
		}
	}
}

egn::dchips BlueprintCalculator::calculateExpectedValue() const
{
	int64_t v = 0;
	int64_t s = calculateSumRegrets();

	// If no regret is positive, all actions have the same proba,
	// so we take the arithmetic mean of the expected values.
	if (s == 0) {
		while (s < nActions())
			v += expVals.rbegin()[s++];
	}

	else {
		for (uint8_t a = 0; a < nActions(); ++a) {
			if (getRegret(a) > 0)
				v += (int64_t)getRegret(a) * expVals.rbegin()[a];
		}
	}

	return (egn::dchips)(v / s);
}

egn::dchips BlueprintCalculator::calculateExpectedValueP() const
{
	int64_t v = 0;
	int64_t s = calculateSumRegrets();

	// If no regret is positive, all actions have the same proba,
	// so we take the arithmetic mean of the expected values.
	if (s == 0) {
		for (uint8_t a = 0; a < nActions(); ++a) {
			if (visited.rbegin()[nActions() - 1 - a])
				v += expVals.rbegin()[s++];
		}
	}

	else {
		uint8_t i = 0;
		for (uint8_t a = 0; a < nActions(); ++a) {
			if (visited.rbegin()[nActions() - 1 - a]) {
				if (getRegret(a) > 0)
					v += (int64_t)getRegret(a) * expVals.rbegin()[i];
				++i;
			}
		}
	}

	return (egn::dchips)(v / s);
}

void BlueprintCalculator::incrNodesCount(uint8_t actionId)
{
	++nodesCount;
	if (getRegret(actionId) == 0) ++nodesUniqueCount;
}

uint64_t BlueprintCalculator::getNUniqueNodes() const
{
	return N_BCK_PREFLOP * abcInfo.nActionSeqs(egn::PREFLOP)
		+ N_BCK_FLOP * abcInfo.nActionSeqs(egn::FLOP)
		+ N_BCK_TURN * abcInfo.nActionSeqs(egn::TURN)
		+ N_BCK_RIVER * abcInfo.nActionSeqs(egn::RIVER);
}

// Save the current strategy of each round on the disk.
void BlueprintCalculator::takeSnapshot()
{
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {

		// Open the file.
		auto file = opt::fstream(
			snapshotPath(nextSnapshotId, r), std::ios::out | std::ios::binary);

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
				cumWeightsRescaler.rescaleCumWeights(cumRegrets, sumStrat);
				strat_t strat = (strat_t)cumRegrets[0];
				opt::saveVar(strat, file);
				for (uint8_t i = 1; i < nLegalActions; ++i) {
					strat = (strat_t)(cumRegrets[i] - cumRegrets[i - 1]);
					opt::saveVar(strat, file);
				}
			}
		}

		file.close();
	}

	++nextSnapshotId;
}

// Average the snapshots into the final strategy for each round
// and save it to the disk.
void BlueprintCalculator::averageSnapshots()
{
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {

		// Allocate memory for the snapshots' strategies.
		std::vector<std::vector<sumStrat_t>> strats(
			abcInfo.nBcks(egn::Round(r)), std::vector<sumStrat_t>(abcInfo.nActionSeqs(egn::Round(r))));

		// Calculate the sum of the snapshots' strategies.
		for (unsigned snapshotId = 1; snapshotId <= nSnapshots; ++snapshotId) {

			// Open the snapshot.
			auto snapshotFile = opt::fstream(
				snapshotPath(snapshotId, r), std::ios::in | std::ios::binary);

			// Add the snapshot's strategy to the total sum.
			for (bckSize_t handIdx = 0; handIdx < strats.size(); ++handIdx) {
				for (size_t seqIdx = 0; seqIdx < strats[0].size(); ++seqIdx) {
					strat_t strat;
					opt::loadVar(strat, snapshotFile);
					strats[handIdx][seqIdx] += strat;
				}
			}

			snapshotFile.close();
		}

		// Normalize the snapshots' strategies by dividing by nSnapshots.
		// We apply the following weird procedure so that the sum of
		// the strategy values will be exactly equal to sumStrat, getting around
		// rounding issues.
		for (auto& handStrats : strats) {
			abc::GroupedActionSeqs::seqIdx_t seqIdx = 0;
			for (const uint8_t nLegalActions : gpSeqs.lens[r]) {
				for (size_t i = seqIdx + 1; i < seqIdx + nLegalActions; ++i)
					handStrats[i] += handStrats[i - 1];
				for (size_t i = seqIdx; i < seqIdx + nLegalActions; ++i)
					handStrats[i] = (sumStrat_t)std::round((double)handStrats[i] / nSnapshots);
				for (size_t i = seqIdx + 1; i < seqIdx + nLegalActions; ++i)
					handStrats[i] -= handStrats[i - 1];
				seqIdx += nLegalActions;
			}
		}

		// Open the file.
		auto file = opt::fstream(stratPath(r), std::ios::out | std::ios::binary);

		// Write the average of the snapshots' strategies.
		for (bckSize_t handIdx = 0; handIdx < strats.size(); ++handIdx) {
			for (size_t seqIdx = 0; seqIdx < strats[0].size(); ++seqIdx) {
				strat_t strat = (strat_t)strats[handIdx][gpSeqsInv.invSeqs[r][seqIdx]];
				opt::saveVar(strat, file);
			}
		}

		file.close();
	}
}

void BlueprintCalculator::writeConstants() const
{
	auto file = std::ofstream(constantPath());

	WRITE_VAR(file, BLUEPRINT_GAME_NAME);
	file << "\n";
	WRITE_VAR(file, N_BCK_PREFLOP);
	WRITE_VAR(file, N_BCK_FLOP);
	WRITE_VAR(file, N_BCK_TURN);
	WRITE_VAR(file, N_BCK_RIVER);
	file << "\n";
	WRITE_VAR(file, MAX_PLAYERS);
	file << "\n";
	WRITE_VAR(file, ANTE);
	WRITE_VAR(file, BIG_BLIND);
	WRITE_VAR(file, INITIAL_STAKE);
	file << "\n";
	WRITE_VAR(file, BET_SIZES);

	file << printSep << "\n\n";

	WRITE_VAR(file, BLUEPRINT_BUILD_NAME);
	file << "\n";
	WRITE_VAR(file, nSnapshots);
	file << "\n";
	WRITE_VAR(file, snapshotBeginIter);
	WRITE_VAR(file, snapshotPeriod);
	file << "\n";
	WRITE_VAR(file, discountEndIter);
	WRITE_VAR(file, discountPeriod);
	file << "\n";
	WRITE_VAR(file, pruneBeginIter);
	WRITE_VAR(file, pruneProbaPerc);
	WRITE_VAR(file, pruneThreshold);
	WRITE_VAR(file, minRegret);
	file << "\n";
	WRITE_VAR(file, checkpointPeriod);
	WRITE_VAR(file, printPeriod);
	file << "\n";
	WRITE_VAR(file, endIter);

	file.close();
}

void BlueprintCalculator::verifyConstants() const
{
	std::ifstream file(constantPath());

	verifyOneConstant(file, BLUEPRINT_GAME_NAME);
	opt::skipLine(file);
	verifyOneConstant(file, N_BCK_PREFLOP);
	verifyOneConstant(file, N_BCK_FLOP);
	verifyOneConstant(file, N_BCK_TURN);
	verifyOneConstant(file, N_BCK_RIVER);
	opt::skipLine(file);
	verifyOneConstant(file, MAX_PLAYERS);
	opt::skipLine(file);
	verifyOneConstant(file, ANTE);
	verifyOneConstant(file, BIG_BLIND);
	verifyOneConstant(file, INITIAL_STAKE);
	opt::skipLine(file);
	verifyOneConstant(file, BET_SIZES);

	opt::skipLine(file);
	opt::skipLine(file);

	verifyOneConstant(file, BLUEPRINT_BUILD_NAME);
	opt::skipLine(file);
	opt::skipLine(file);
	opt::skipLine(file);
	verifyOneConstant(file, snapshotBeginIter);
	verifyOneConstant(file, snapshotPeriod);
	opt::skipLine(file);
	verifyOneConstant(file, discountEndIter);
	verifyOneConstant(file, discountPeriod);

	file.close();
}

template<typename T>
void BlueprintCalculator::verifyOneConstant(std::ifstream& file, const T& v) const
{
	std::string line;
	std::getline(file, line);

	std::ostringstream os;
	WRITE_VAR(os, v);

	if (opt::extractVarValue(line) != opt::extractVarValue(os.str()))
		throw std::runtime_error("A constant differs from the one used in the checkpoint.");
}

void BlueprintCalculator::updateCheckpoint()
{
	lastCheckpointIter = currIter;

	auto file = opt::fstream(checkpointPath(), std::ios::out | std::ios::binary);

	opt::save3DVector(regrets, file);

	rng.save(file);
	pruneRandChoice.save(file);
	actionRandChoice.save(file);
	abcInfo.state.saveRng(file);

	opt::saveVar(currIter, file);
	double duration = extraDuration + opt::getDuration(startTime);
	opt::saveVar(duration, file);
	opt::saveVar(nextSnapshotId, file);
	opt::saveVar(lastCheckpointIter, file);

	opt::saveVar(nodesCount, file);
	opt::saveVar(nodesUniqueCount, file);

	file.close();
}

void BlueprintCalculator::loadCheckpoint(std::fstream& file)
{
	opt::load3DVector(regrets, file);

	rng.load(file);
	pruneRandChoice.load(file);
	actionRandChoice.load(file);
	abcInfo.state.loadRng(file);

	opt::loadVar(currIter, file);
	opt::loadVar(extraDuration, file);
	opt::loadVar(nextSnapshotId, file);
	opt::loadVar(lastCheckpointIter, file);

	opt::loadVar(nodesCount, file);
	opt::loadVar(nodesUniqueCount, file);
}

void BlueprintCalculator::printProgress() const
{
	std::cout
		<< blueprintName() << "\n\n"

		<< opt::progressStr(currIter, endIter, startTime, extraDuration) << "\n\n"

		<< "next checkpoint: " << opt::prettyDuration(
			opt::remainingTime(currIter, lastCheckpointIter + checkpointPeriod, startTime, extraDuration))

		<< " | next snapshot: " << opt::prettyDuration(
			opt::remainingTime(currIter, snapshotBeginIter + (nextSnapshotId - 1) * snapshotPeriod, startTime, extraDuration))
		<< " (" << nextSnapshotId - 1 << "/" << nSnapshots << ")\n";

	if (currIter < pruneBeginIter)
		std::cout << "prune start: " << opt::prettyDuration(
			opt::remainingTime(currIter, pruneBeginIter, startTime, extraDuration));
	else std::cout << "pruning";

	if (currIter < discountEndIter)
		std::cout << " | discount end: " << opt::prettyDuration(
			opt::remainingTime(currIter, discountEndIter, startTime, extraDuration)) << "\n\n";
	else std::cout << " | no discount\n\n";

	std::cout
		<< "nodes: " << opt::prettyNumDg(nodesCount, 3)
		<< " | unique nodes: " << opt::prettyNumDg(nodesUniqueCount, 3)
		<< " / " << opt::prettyNumDg(totUniqueNodes, 3)
		<< " (" << opt::prettyPerc(nodesUniqueCount, totUniqueNodes) << ")\n\n";

	std::cout << "VM: " << opt::vmUsedByMeStr(1) << " | RAM: " << opt::ramUsedByMeStr(1) << "\n";

	std::cout << printSep << "\n\n";
}

void BlueprintCalculator::printFinalStats() const
{
	std::cout << "Duration: " << opt::prettyDuration(extraDuration + opt::getDuration(startTime)) << "\n";
}

std::string BlueprintCalculator::blueprintDir(const std::string& blueprintName)
{
	return "../data/Blueprint/" + blueprintName + "/";
}

std::string BlueprintCalculator::blueprintTmpDir(const std::string& blueprintName)
{
	return blueprintDir(blueprintName) + "tmp/";
}

std::string BlueprintCalculator::snapshotPath(
	const std::string& blueprintName, unsigned snapshotId, uint8_t roundId)
{
	return blueprintTmpDir(blueprintName) + "SNAPSHOT"
		+ "_" + std::to_string(snapshotId) + "_" + opt::toUpper(egn::roundToString(roundId)) + ".bin";
}

std::string BlueprintCalculator::checkpointPath(const std::string& blueprintName)
{
	return blueprintTmpDir(blueprintName) + "CHECKPOINT.bin";
}

std::string BlueprintCalculator::constantPath(const std::string& blueprintName)
{
	return blueprintDir(blueprintName) + "CONSTANTS.txt";
}

std::string BlueprintCalculator::stratPath(const std::string& blueprintName, uint8_t roundId)
{
	return blueprintDir(blueprintName) + "STRATEGY"
		+ "_" + opt::toUpper(egn::roundToString(roundId)) + ".bin";
}

std::string BlueprintCalculator::blueprintDir()
{
	return blueprintDir(blueprintName());
}

std::string BlueprintCalculator::blueprintTmpDir()
{
	return blueprintTmpDir(blueprintName());
}

std::string BlueprintCalculator::snapshotPath(unsigned snapshotId, uint8_t roundId)
{
	return snapshotPath(blueprintName(), snapshotId, roundId);
}

std::string BlueprintCalculator::checkpointPath()
{
	return checkpointPath(blueprintName());
}

std::string BlueprintCalculator::constantPath()
{
	return constantPath(blueprintName());
}

std::string BlueprintCalculator::stratPath(uint8_t roundId)
{
	return stratPath(blueprintName(), roundId);
}

} // bp