#include "GroupedActionSeqsInv.h"

namespace abc {

GroupedActionSeqsInv::GroupedActionSeqsInv(const std::string& groupedActionSeqsName) :

	groupedActionSeqsName(groupedActionSeqsName),
	filePath(groupedActionSeqsDir + groupedActionSeqsName + "_GROUPED_ACTION_SEQS_INV.bin")
{
}

void GroupedActionSeqsInv::build()
{
	GroupedActionSeqs gpSeqs(groupedActionSeqsName);
	gpSeqs.load();

	// Allocate memory for invSeqs.
	invSeqs = {
		std::vector<seqIdx_t>(gpSeqs.seqs[egn::PREFLOP].size()),
		std::vector<seqIdx_t>(gpSeqs.seqs[egn::FLOP].size()),
		std::vector<seqIdx_t>(gpSeqs.seqs[egn::TURN].size()),
		std::vector<seqIdx_t>(gpSeqs.seqs[egn::RIVER].size())
	};

	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {
		for (seqIdx_t i = 0; i < gpSeqs.seqs[r].size(); ++i)
			invSeqs[r][gpSeqs.seqs[r][i]] = i;
	}
}

void GroupedActionSeqsInv::save()
{
	opt::save2DVector(invSeqs, filePath);
}

void GroupedActionSeqsInv::load()
{
	const abc::ActionSeqSize seqSizes(groupedActionSeqsName);

	// Allocate memory for invSeqs.
	invSeqs = {
		std::vector<seqIdx_t>(seqSizes.preflopSize),
		std::vector<seqIdx_t>(seqSizes.flopSize),
		std::vector<seqIdx_t>(seqSizes.turnSize),
		std::vector<seqIdx_t>(seqSizes.riverSize)
	};

	opt::load2DVector(invSeqs, filePath);
}

} // abc