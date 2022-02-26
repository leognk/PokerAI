#ifndef ABC_ACTIONSEQSIZE_H
#define ABC_ACTIONSEQSIZE_H

#include "ActionSeqIndexer.h"

namespace abc {

class ActionSeqSize
{
public:
	ActionSeqSize(const std::string& indexerName);

	uint64_t preflopSize;
	uint64_t flopSize;
	uint64_t turnSize;
	uint64_t riverSize;

}; // ActionSeqSize

} // abc

#endif // ABC_ACTIONSEQSIZE_H