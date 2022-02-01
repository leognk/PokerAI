#ifndef ABC_TREETRAVERSER_H
#define ABC_TREETRAVERSER_H

#include "AbstractInfoset.h"

namespace abc {

// Number of buckets has no influence on the actions
// tree's traversal.
typedef uint8_t bckSize_t;
static const uint8_t nBck = 5;
typedef AbstractInfoset<bckSize_t, nBck> abcInfo_t;

class TreeTraverser
{
public:
	TreeTraverser(
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const std::vector<std::vector<std::vector<float>>>& betSizes,
		bool saveActionSeqs = false,
		bool verbose = false);

	// Return the number of nodes.
	uint64_t traverseRoundTree(
		egn::Round round,
		std::vector<std::vector<uint8_t>>& actionSeqs);

	uint64_t traverseRoundTree(egn::Round round);

private:

	void traverseRoundTreeFixedPlayers(
		egn::Round round, uint8_t nPlayers,
		uint64_t& totNodes, uint64_t& totFinishedSeq, uint64_t& totContinuingSeq,
		uint32_t& totHeight,
		std::vector<std::vector<uint8_t>>& actionSeqs);

	void prepareAbcInfoset(egn::Round round, uint8_t nPlayers);

	void printProgress(
		uint64_t nNodes,
		uint64_t nFinishedSeq,
		uint64_t nContinuingSeq,
		uint32_t height,
		std::chrono::high_resolution_clock::time_point startTime);

	abcInfo_t abcInfo;

	bool saveActionSeqs;
	bool verbose;

}; // TreeTraverser

} // abc

#endif // ABC_TREETRAVERSER_H