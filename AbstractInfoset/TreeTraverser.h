#ifndef ABC_TREETRAVERSER_H
#define ABC_TREETRAVERSER_H

#include "SimpleAbstractInfoset.h"
#include <chrono>

namespace abc {

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
	// Accumulate roundActions in actionSeqs if
	// saveActionSeqs is set to true.
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

	SimpleAbstractInfoset abcInfo;

	bool saveActionSeqs;
	bool verbose;

}; // TreeTraverser

} // abc

#endif // ABC_TREETRAVERSER_H