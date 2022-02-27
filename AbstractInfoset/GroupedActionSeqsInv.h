#ifndef ABC_GROUPEDACTIONSEQSINV_H
#define ABC_GROUPEDACTIONSEQSINV_H

#include "GroupedActionSeqs.h"
#include "ActionSeqSize.h"

namespace abc {

// Inverse permutation of GroupedActionSeqs.
class GroupedActionSeqsInv
{
public:
	typedef uint32_t seqIdx_t;

	GroupedActionSeqsInv(const std::string& groupedActionSeqsName);

	void build();
	void save();
	void load();

	std::vector<std::vector<seqIdx_t>> invSeqs;

private:
	const std::string groupedActionSeqsName;
	const std::string filePath;

};

} // abc

#endif // ABC_GROUPEDACTIONSEQSINV_H