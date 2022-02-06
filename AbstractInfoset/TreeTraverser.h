#ifndef ABC_TREETRAVERSER_H
#define ABC_TREETRAVERSER_H

#include "SimpleAbstractInfoset.h"
#include <chrono>
#include <unordered_set>

namespace abc {

class TreeTraverser
{
public:
	typedef std::vector<uint8_t> seq_t;
	typedef std::vector<seq_t> seqs_t;

	TreeTraverser(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const std::vector<std::vector<std::vector<float>>>& betSizes,
		bool verbose = false);

	// Generate the sets of all action sequences for each round.
	void traverseTree(std::vector<seqs_t>& actionSeqs);

private:
	// Return the number of nodes.
	void traverseRoundTree(
		egn::Round round,
		const seqs_t& seqsToCurrentRound,
		seqs_t& seqsToNextRound,
		seqs_t& actionSeqs);

	void printProgress(
		uint64_t nNodes,
		uint64_t nFinishedSeq,
		uint64_t nContinuingSeq,
		uint32_t height,
		std::chrono::high_resolution_clock::time_point startTime);

	uint8_t maxPlayers;

	SimpleAbstractInfoset abcInfo;

	bool verbose;

}; // TreeTraverser

} // abc

#endif // ABC_TREETRAVERSER_H