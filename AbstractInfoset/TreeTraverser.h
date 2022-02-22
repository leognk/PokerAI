#ifndef ABC_TREETRAVERSER_H
#define ABC_TREETRAVERSER_H

#include "SimpleAbstractInfoset.h"
#include "ActionSeq.h"
#include "../Utils/Time.h"
#include <unordered_set>

namespace abc {

class TreeTraverser
{
public:
	typedef StdActionSeq seq_t;
	typedef ActionSeq<4, 50> longSeq_t;

	typedef std::unordered_set<seq_t, ActionSeqHash> seqs_t;
	typedef std::unordered_set<longSeq_t, ActionSeqHash> longSeqs_t;

	TreeTraverser(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const betSizes_t& betSizes,
		bool verbose = false);

	// Return the sets of all action sequences of each round.
	std::vector<std::vector<seq_t>> traverseTree();

private:
	void traverseRoundTree(
		egn::Round round,
		const longSeqs_t& seqsToCurrentRound,
		longSeqs_t& seqsToNextRound,
		std::vector<seq_t>& actionSeqs);

	void initAbcInfo(const longSeq_t& seqToCurrentRound);

	void addSubActionSeqs(seqs_t& actionSeqs);

	void printProgress(
		uint64_t nNodes,
		uint64_t nFinishedSeq,
		uint64_t nContinuingSeq,
		uint64_t height,
		opt::time_t startTime);

	SimpleAbstractInfoset abcInfo;

	bool verbose;

}; // TreeTraverser

} // abc

#endif // ABC_TREETRAVERSER_H