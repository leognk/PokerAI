#ifndef ABC_ACTIONSEQINDEXER_H
#define ABC_ACTIONSEQINDEXER_H

#include "xxhash.h"
#include "../BBHash/BooPHF.h"

namespace abc {

class ActionSeqHasher
{
public:
	template<typename key_t>
	uint64_t operator()(key_t key, uint64_t seed = 0) const
	{
		return XXH3_64bits_withSeed(&key, sizeof(key), seed);
	}
};

template<typename actionSeqIdx_t>
class ActionSeqIndexer
{
public:
	typedef boomphf::mphf<T, ActionSeqHasher> phf_t;

	static void init()
	{
		phf_t phf(keys.size(), keys);
	}

	static actionSeqIdx_t actionSeqIndex(const std::vector<uint8_t>& actionSeq)
	{
		return phf.lookup(actionSeq);
	}

private:


}; // ActionSeqIndexer

} // abc

#endif // ABC_ACTIONSEQINDEXER_H