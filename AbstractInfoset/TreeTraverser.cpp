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

void TreeTraverser::traverseTree(std::vector<seqs_t>& actionSeqs)
{
	actionSeqs.resize(egn::N_ROUNDS);
	seqs_t seqsToCurrentRound(1);

	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {
		seqs_t seqsToNextRound;

		traverseRoundTree(egn::Round(r), seqsToCurrentRound, seqsToNextRound, actionSeqs[r]);

		seqsToCurrentRound = seqsToNextRound;
		if (verbose) std::cout << "\n";
	}
}

void TreeTraverser::traverseRoundTree(
	egn::Round round,
	const seqs_t& seqsToCurrentRound,
	seqs_t& seqsToNextRound,
	seqs_t& actionSeqs)
{
	auto startTime = std::chrono::high_resolution_clock::now();
	uint64_t nNodes = 0, nFinishedSeq = 0, nContinuingSeq = 0;
	uint32_t height = 0;

	std::unordered_set<uint64_t> actionSeqsHashes;

	// Set of pairs (nPlayers, pot).
	std::unordered_set<std::array<egn::chips, 2>, opt::ContainerHash> nextRoundStates;

	seq_t roundActions;

	for (size_t seqIdx = 0; seqIdx < seqsToCurrentRound.size(); ++seqIdx) {

		const size_t initialActionSeqsSize = actionSeqs.size();

		abcInfo.startNewHand();
		for (uint8_t a : seqsToCurrentRound[seqIdx])
			abcInfo.nextState(a);

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

			// Save action sequence.
			bool isNewSeq;

			if (round == egn::PREFLOP) {
				actionSeqs.push_back(roundActions);
				isNewSeq = true;
			}

			// For rounds other than preflop, include the number of players.
			else {
				roundActions.push_back(nPlayers);

				// Search whether the action sequence has already been generated before.
				if (actionSeqsHashes.insert(opt::ContainerHash()(roundActions)).second)
					isNewSeq = true;
				else {
					auto it = std::find(actionSeqs.begin(), actionSeqs.begin() + initialActionSeqsSize, roundActions);
					isNewSeq = it != actionSeqs.begin() + initialActionSeqsSize;
				}

				// Add the action sequence only if it is a new one.
				if (isNewSeq) actionSeqs.push_back(roundActions);

				roundActions.pop_back();
			}

			abcInfo.nextState(roundActions.back());

			// Update variables.
			hist.push_back(abcInfo);
			lastChild.push_back(a == 0);
			if (isNewSeq) ++nNodes;
#pragma warning(suppress: 4267)
			if (hist.size() - 1 > height) height = hist.size() - 1;

			// Reached end of the round.
			if (abcInfo.state.round != round || abcInfo.state.finished) {

				if (abcInfo.state.finished) {
					if (isNewSeq) ++nFinishedSeq;
				}

				else {
					if (isNewSeq) ++nContinuingSeq;

					if (nextRoundStates.insert({ abcInfo.nPlayers, abcInfo.state.pot }).second) {
						roundActions.insert(
							roundActions.begin(),
							seqsToCurrentRound[seqIdx].begin(),
							seqsToCurrentRound[seqIdx].end());
						seqsToNextRound.push_back(roundActions);
					}
				}

				if (stack.empty()) {

					if (verbose && ((seqIdx + 1) % size_t(1e3) == 0
						|| seqIdx == seqsToCurrentRound.size() - 1)) {

						std::cout
							<< "ROUND: " << round
							<< " - STATE: " << std::setw(5) << seqIdx + 1 << "/" << seqsToCurrentRound.size() << "\n";
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
	}
}

void TreeTraverser::printProgress(
	uint64_t nNodes,
	uint64_t nFinishedSeq,
	uint64_t nContinuingSeq,
	uint32_t height,
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