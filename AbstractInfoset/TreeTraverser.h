#ifndef ABC_TREETRAVERSER_H
#define ABC_TREETRAVERSER_H

#include "SimpleAbstractInfoset.h"
#include "ActionSeq.h"
#include <chrono>
#include <unordered_set>
#include <set>

namespace abc {

class TreeTraverser
{
public:
	typedef StdActionSeq seq_t;
	typedef ActionSeq<4, 50> longSeq_t;

	typedef std::unordered_set<seq_t, seq_t::hasher_t> seqs_t;
	typedef std::unordered_set<longSeq_t, longSeq_t::hasher_t> longSeqs_t;

	TreeTraverser(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const std::vector<std::vector<std::vector<float>>>& betSizes,
		bool verbose = false);

	// Return the sets of all action sequences of each round.
	std::vector<std::vector<seq_t>> traverseTree();

private:
	// Return the number of nodes.
	void traverseRoundTree(
		egn::Round round,
		const longSeqs_t& seqsToCurrentRound,
		longSeqs_t& seqsToNextRound,
		std::vector<seq_t>& actionSeqs);

	void addSubActionSeqs(seqs_t& actionSeqs);

	void printProgress(
		uint64_t nNodes,
		uint64_t nFinishedSeq,
		uint64_t nContinuingSeq,
		uint64_t height,
		std::chrono::high_resolution_clock::time_point startTime);

	uint8_t maxPlayers;

	SimpleAbstractInfoset abcInfo;

	bool verbose;

}; // TreeTraverser

} // abc

#endif // ABC_TREETRAVERSER_H