#include "TreeTraverser.h"
#include "../Utils/FastVector.h"
#include "../Utils/Hash.h"

namespace abc {

TreeTraverser::TreeTraverser(
	uint8_t maxPlayers,
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake,
	const betSizes_t& betSizes,
	bool verbose) :

	abcInfo(maxPlayers, ante, bigBlind, initialStake, betSizes),
	verbose(verbose)
{
}

std::vector<std::vector<TreeTraverser::seq_t>> TreeTraverser::traverseTree()
{
	std::vector<std::vector<seq_t>> actionSeqs(egn::N_ROUNDS);
	longSeqs_t seqsToCurrentRound;
	seqsToCurrentRound.insert(longSeq_t());

	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {

		if (verbose)
			std::cout
				<< opt::toUpper(egn::roundToString(egn::Round(r))) << "\n"
				<< "states: " << opt::prettyNumber(seqsToCurrentRound.size(), 1) << "\n";

		longSeqs_t seqsToNextRound;
		traverseRoundTree(egn::Round(r), seqsToCurrentRound, seqsToNextRound, actionSeqs[r]);
		seqsToCurrentRound = seqsToNextRound;

		if (verbose) std::cout << "\n";
	}

	return actionSeqs;
}

void TreeTraverser::traverseRoundTree(
	egn::Round round,
	const longSeqs_t& seqsToCurrentRound,
	longSeqs_t& seqsToNextRound,
	std::vector<seq_t>& actionSeqs)
{
	opt::time_t startTime = opt::getTime();
	uint64_t nFinishedSeq = 0, nContinuingSeq = 0, height = 0;

	// Set of pairs (nPlayers, pot).
	std::unordered_set<std::array<egn::chips, 2>, opt::ContainerHash> nextRoundStates;

	// Set of all action sequences of this round.
	seqs_t setActionSeqs;

	// Loop over each sequence leading to a state (nPlayers, pot) of the target round.
	// The initial state of the infoset will be generated with these sequences.
	size_t seqToCurrentRoundIdx = 0;
	for (const longSeq_t& seqToCurrentRound : seqsToCurrentRound) {

		initAbcInfo(seqToCurrentRound);

		// Variables for doing a DFS.

		opt::FastVector<SimpleAbstractInfoset> hist;
		hist.push_back(abcInfo);

		// lastChild[i] indicates whether hist[i] is the last child of its parent node.
		// We use uint8_t instead of bool because vector<bool> behaves weirdly.
		opt::FastVector<uint8_t> lastChild;
		lastChild.push_back(true);

		opt::FastVector<uint8_t> stack(abcInfo.nActions());
		for (uint8_t i = 0; i < stack.size(); ++i)
			stack[i] = i;

		// Do a DFS.
		while (true) {

			// Process the next node.
			uint8_t actionId = stack.back();
			stack.pop_back();

			// Keep action sequence in memory here before going to the next state
			// because when the round changes, roundActions is emptied.
			seq_t roundActions = abcInfo.roundActions;
			roundActions.push_back(abcInfo.actionAbc.legalActions[actionId]);
			uint8_t nPlayers = abcInfo.nPlayers;

			// For preflop, we are sure that the action sequence is unique,
			// so no need to use a set which costs a lot.
			if (round == egn::PREFLOP)
				actionSeqs.push_back(roundActions);

			abcInfo.nextState(roundActions.back());

			// Update DFS variables.
			hist.push_back(abcInfo);
			lastChild.push_back(actionId == 0);
			if (hist.size() - 1 > height)
				height = hist.size() - 1;

			// Reached the end of the round.
			if (abcInfo.state.round != round || abcInfo.state.finished) {

				// Save the complete action sequence for rounds other than preflop.
				// Sub-sequences will be generated at the end to minimize the use of sets
				// which cost a lot.
				bool isNewSeq = true;
				// For rounds other than preflop, we need to use a set
				// and to include the number of players.
				if (round != egn::PREFLOP) {
					roundActions.push_back(nPlayers);
					isNewSeq = setActionSeqs.insert(roundActions).second;
					roundActions.pop_back();
				}

				// Update some variables.
				if (abcInfo.state.finished) {
					if (isNewSeq) ++nFinishedSeq;
				}
				else {
					if (isNewSeq) ++nContinuingSeq;
					if (nextRoundStates.insert({ abcInfo.nPlayers, abcInfo.state.pot }).second)
						seqsToNextRound.insert(concatActionSeqs<longSeq_t>(seqToCurrentRound, roundActions));
				}

				// End of DFS.
				if (stack.empty()) {

					// End of tree traversals for all states of the round.
					if (seqToCurrentRoundIdx == seqsToCurrentRound.size() - 1) {

						// Generate the missing sub-sequences for rounds other than preflop.
						if (round != egn::PREFLOP) {
							addSubActionSeqs(setActionSeqs);
							// Copy the set of action sequences into the vector.
							actionSeqs.insert(actionSeqs.end(), setActionSeqs.begin(), setActionSeqs.end());
						}

						if (verbose)
							printProgress(actionSeqs.size(), nFinishedSeq, nContinuingSeq, height, startTime);
					}

					// Go out from the current DFS.
					break;
				}

				// Go back to the latest node having children not visited yet.
				bool wasLastChild;
				do {
					wasLastChild = lastChild.back();
					hist.pop_back();
					lastChild.pop_back();
				} while (wasLastChild);
				abcInfo = hist.back();
			}

			// Current action sequence continues.
			else {
				// Add node's children to the stack.
				for (uint8_t a = 0; a < abcInfo.nActions(); ++a)
					stack.push_back(a);
			}
		}

		++seqToCurrentRoundIdx;
	}
}

void TreeTraverser::initAbcInfo(const longSeq_t& seqToCurrentRound)
{
	abcInfo.startNewHand();
	ActionSeqIterator iter(seqToCurrentRound);
	while (!iter.end())
		abcInfo.nextState(iter.next());
}

// Loop over all the complete action sequences and insert their sub-sequences.
void TreeTraverser::addSubActionSeqs(seqs_t& actionSeqs)
{
	const seqs_t completeSeqs = actionSeqs;
	for (const auto& seq : completeSeqs) {
		uint8_t nPlayers = seq.back();
		// We stop length before seq.size() - 1 because the last entry is nPlayers.
		for (uint8_t length = 1; length < seq.size() - 1; ++length) {
			seq_t subSeq = seq.extractSubSeq(length);
			subSeq.push_back(nPlayers);
			actionSeqs.insert(subSeq);
		}
	}
}

void TreeTraverser::printProgress(
	uint64_t nNodes,
	uint64_t nFinishedSeq,
	uint64_t nContinuingSeq,
	uint64_t height,
	opt::time_t startTime)
{
	uint64_t nActionSeq = nFinishedSeq + nContinuingSeq;
	const std::string duration = opt::prettyDuration(startTime);

	std::cout
		<< "nodes: " << std::setw(6) << opt::prettyNumber(nNodes, 1)
		<< " | seq: " << std::setw(6) << opt::prettyNumber(nActionSeq, 1)
		<< " | finishedSeq: " << std::setw(6) << opt::prettyNumber(nFinishedSeq, 1)
		<< " | continuingSeq: " << std::setw(6) << opt::prettyNumber(nContinuingSeq, 1)
		<< " | height: " << std::setw(2) << height
		<< " | " << duration << "\n";
}

} // abc