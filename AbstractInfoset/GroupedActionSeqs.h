#ifndef ABC_GROUPEDACTIONSEQS_H
#define ABC_GROUPEDACTIONSEQS_H

#include "ActionSeqIndexer.h"

namespace abc {

static const std::string groupedActionSeqsDir = "../data/ActionSequences/GroupedActionSeqs/";

class GroupedActionSeqs
{
public:
	typedef uint32_t seqIdx_t;

	// groupedActionSeqsName is used in files names.
	GroupedActionSeqs(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const betSizes_t& betSizes,
		const std::string& groupedActionSeqsName);

	void build();
	void save();
	void load();

	std::vector<std::vector<seqIdx_t>> seqs;
	std::vector<std::vector<uint8_t>> lens;

private:
	ActionSeqIndexer indexer;

	const std::string filePath;

};

} // abc

#endif // ABC_GROUPEDACTIONSEQS_H