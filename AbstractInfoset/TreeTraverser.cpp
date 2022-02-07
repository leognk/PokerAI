#include "TreeTraverser.h"
#include "../Utils/FastVector.h"
#include "../Utils/Hash.h"

namespace abc {

TreeTraverser::TreeTraverser(
	uint8_t maxPlayers,
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake,
	const std::vector<std::vector<std::vector<float>>>& betSizes,
	bool verbose) :

	maxPlayers(maxPlayers),
	abcInfo(maxPlayers, ante, bigBlind, initialStake, betSizes),
	verbose(verbose)
{
}

std::vector<TreeTraverser::seqs_t> TreeTraverser::traverseTree()
{
	std::vector<seqs_t> actionSeqs(egn::N_ROUNDS);

	longSeqs_t seqsToCurrentRound;
	seqsToCurrentRound.insert(longSeq_t());

	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {
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
	seqs_t& actionSeqs)
{
	auto startTime = std::chrono::high_resolution_clock::now();
	uint64_t nNodes = 0, nFinishedSeq = 0, nContinuingSeq = 0, height = 0;

	// Set of pairs (nPlayers, pot).
	std::unordered_set<std::array<egn::chips, 2>, opt::ContainerHash> nextRoundStates;

	seq_t roundActions;

	size_t seqToCurrentRoundIdx = 0;

	for (const longSeq_t& seqToCurrentRound : seqsToCurrentRound) {

		abcInfo.startNewHand();
		ActionSeqIterator iter(seqToCurrentRound);
		while (!iter.end())
			abcInfo.nextState(iter.next());

		opt::FastVector<SimpleAbstractInfoset> hist;
		hist.push_back(abcInfo);
		// lastChild[i] indicates whether hist[i] is the last child of its parent node.
		// We use uint8_t instead of bool because vector<bool> behaves weirdly.
		opt::FastVector<uint8_t> lastChild;
		lastChild.push_back(true);

		opt::FastVector<uint8_t> stack(abcInfo.nActions());
		for (uint8_t i = 0; i < stack.size(); ++i)
			stack[i] = i;

		while (true) {

			// Process the next node.
			uint8_t a = stack.back();
			stack.pop_back();

			roundActions = abcInfo.roundActions;
			roundActions.push_back(abcInfo.actionAbc.legalActions[a]);
			uint8_t nPlayers = abcInfo.nPlayers;

			// Save action sequence before going to the next state because
			// if the round changes, roundActions is emptied.
			bool isNewSeq;
			if (round == egn::PREFLOP)
				isNewSeq = actionSeqs.insert(roundActions).second;
			// For rounds other than preflop, include the number of players.
			else {
				roundActions.push_back(nPlayers);
				isNewSeq = actionSeqs.insert(roundActions).second;
				roundActions.pop_back();
			}

			abcInfo.nextState(roundActions.back());

			// Update variables.
			hist.push_back(abcInfo);
			lastChild.push_back(a == 0);
			if (isNewSeq) ++nNodes;
			if (hist.size() - 1 > height) height = hist.size() - 1;

			// Reached end of the round.
			if (abcInfo.state.round != round || abcInfo.state.finished) {

				if (abcInfo.state.finished) {
					if (isNewSeq) ++nFinishedSeq;
				}

				else {
					if (isNewSeq) ++nContinuingSeq;

					if (nextRoundStates.insert({ abcInfo.nPlayers, abcInfo.state.pot }).second) {
						seqsToNextRound.insert(concatActionSeqs
							<longSeq_t::nBitsPerAction, longSeq_t::maxSizeActionSeq>(seqToCurrentRound, roundActions));
					}
				}

				if (stack.empty()) {

					if (verbose && ((seqToCurrentRoundIdx + 1) % size_t(1e3) == 0
						|| seqToCurrentRoundIdx == seqsToCurrentRound.size() - 1)) {

						std::cout
							<< "ROUND: " << round
							<< " - STATE: " << std::setw(5) << seqToCurrentRoundIdx + 1
							<< "/" << seqsToCurrentRound.size() << "\n";
						printProgress(nNodes, nFinishedSeq, nContinuingSeq, height, startTime);
					}

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

			else {
				// Add node's children to stack.
				for (uint8_t a = 0; a < abcInfo.nActions(); ++a)
					stack.push_back(a);
			}
		}

		++seqToCurrentRoundIdx;
	}
}

void TreeTraverser::printProgress(
	uint64_t nNodes,
	uint64_t nFinishedSeq,
	uint64_t nContinuingSeq,
	uint64_t height,
	std::chrono::high_resolution_clock::time_point startTime)
{
	uint64_t nActionSeq = nFinishedSeq + nContinuingSeq;
	auto t = std::chrono::high_resolution_clock::now();
	auto duration = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(t - startTime).count();

	std::cout

		<< "nodes: " << std::setw(8) << nNodes
		<< std::setw(6) << (" (" + std::to_string((uint32_t)std::round(nNodes / 1e6)) + "M)")

		<< " | seq: " << std::setw(8) << nActionSeq
		<< std::setw(6) << (" (" + std::to_string((uint32_t)std::round(nActionSeq / 1e6)) + "M)")

		<< " | finishedSeq: " << std::setw(8) << nFinishedSeq
		<< std::setw(6) << (" (" + std::to_string((uint32_t)std::round(nFinishedSeq / 1e6)) + "M)")

		<< " | continuingSeq: " << std::setw(8) << nContinuingSeq
		<< std::setw(6) << (" (" + std::to_string((uint32_t)std::round(nContinuingSeq / 1e6)) + "M)")

		<< " | height: " << std::setw(2) << height

		<< " | " << std::setw(2) << std::round(duration) << "s" << "\n";
}

} // abc