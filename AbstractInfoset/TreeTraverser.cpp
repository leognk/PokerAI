#include "TreeTraverser.h"
#include "../Utils/FastVector.h"

namespace abc {

TreeTraverser::TreeTraverser(
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake,
	const std::vector<std::vector<std::vector<float>>>& betSizes,
	bool saveActionSeqs,
	bool verbose) :

	abcInfo(ante, bigBlind, initialStake, betSizes),
	saveActionSeqs(saveActionSeqs),
	verbose(verbose)
{
}

uint64_t TreeTraverser::traverseRoundTree(
	egn::Round round,
	std::vector<std::vector<uint8_t>>& actionSeqs)
{
	auto startTime = std::chrono::high_resolution_clock::now();
	uint64_t nNodes = 0, nFinishedSeq = 0, nContinuingSeq = 0;
	uint32_t height = 0;

	for (uint8_t nPlayers = 2; nPlayers <= opt::MAX_PLAYERS; ++nPlayers) {
		traverseRoundTreeFixedPlayers(
			round, nPlayers,
			nNodes, nFinishedSeq, nContinuingSeq,
			height, actionSeqs);
	}

	if (round == egn::PREFLOP) {
		// We must add MAX_PLAYERS - 1 nodes and 1 finishedSeq which correspond to
		// the situation where all players fold at the beginning except the big blind
		// because this situation is not treated when we fix the number of players.
		nNodes += opt::MAX_PLAYERS - 1;
		++nFinishedSeq;
	}

	if (verbose) {
		std::cout
			<< "ROUND: " << round
			<< " - TOTAL (" << std::to_string(opt::MAX_PLAYERS) << " players)\n";
		printProgress(nNodes, nFinishedSeq, nContinuingSeq, height, startTime);
	}

	return nNodes;
}

uint64_t TreeTraverser::traverseRoundTree(egn::Round round)
{
	std::vector<std::vector<uint8_t>> dummyActionSeqs;
	return traverseRoundTree(round, dummyActionSeqs);
}

// Accumulate on totNodes, totFinishedSeq, totContinuingSeq,
// totHeight, actionSeqs in-place.
void TreeTraverser::traverseRoundTreeFixedPlayers(
	egn::Round round, uint8_t nPlayers,
	uint64_t& totNodes, uint64_t& totFinishedSeq, uint64_t& totContinuingSeq,
	uint32_t& totHeight,
	std::vector<std::vector<uint8_t>>& actionSeqs)
{
	auto startTime = std::chrono::high_resolution_clock::now();
	uint64_t nNodes = 0, nFinishedSeq = 0, nContinuingSeq = 0;
	uint32_t height = 0;

	prepareAbcInfoset(round, nPlayers);

	opt::FastVector<SimpleAbstractInfoset> hist;
	hist.push_back(abcInfo);
	// lastChild[i] indicates whether hist[i] is the last child of its parent node.
	// We use uint8_t instead of bool because vector<bool> behaves weirdly.
	opt::FastVector<uint8_t> lastChild;
	lastChild.push_back(true);

	// If the target round is the preflop, we don't make
	// the first player fold because it was taken care in prepareAbcInfoset.
	uint8_t firstIdx = (round == egn::PREFLOP) ? 1 : 0;
	opt::FastVector<uint8_t> stack(abcInfo.nActions - firstIdx);
	for (uint8_t i = 0; i < stack.size(); ++i)
		stack[i] = firstIdx + i;

	while (true) {

		// Process the next node.
		uint8_t a = stack.back();
		stack.pop_back();
		abcInfo.nextState(a);

		hist.push_back(abcInfo);
		lastChild.push_back(a == 0);
		++nNodes;
#pragma warning(suppress: 4267)
		if (hist.size() - 1 > height) height = hist.size() - 1;
		if (saveActionSeqs) actionSeqs.emplace_back(abcInfo.roundActions);

		if (abcInfo.state.round != round || abcInfo.state.finished) {

			if (abcInfo.state.finished) ++nFinishedSeq;
			else ++nContinuingSeq;

			if (stack.empty()) {
				if (verbose) {
					std::cout
						<< "ROUND: " << round
						<< " - N_PLAYERS: " << std::to_string(nPlayers) << "\n";
					printProgress(nNodes, nFinishedSeq, nContinuingSeq, height, startTime);
				}
				totNodes += nNodes;
				totFinishedSeq += nFinishedSeq;
				totContinuingSeq += nContinuingSeq;
				if (height > totHeight) totHeight = height;
				return;
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
			for (uint8_t a = 0; a < abcInfo.nActions; ++a)
				stack.push_back(a);
		}
	}
}

void TreeTraverser::prepareAbcInfoset(egn::Round round, uint8_t nPlayers)
{
	abcInfo.startNewHand();
	// Make MAX_PLAYERS - nPlayers fold at the beginning so that
	// there remains nPlayers in the game.
	for (uint8_t i = 0; i < opt::MAX_PLAYERS - nPlayers; ++i)
		abcInfo.nextState(0);
	if (round == egn::PREFLOP) return;
	// Go to the flop by making everyone call.
	while (abcInfo.state.round != egn::FLOP)
		abcInfo.nextState(1);
	// Make everyone check until reaching the target round.
	while (abcInfo.state.round != round)
		abcInfo.nextState(0);
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