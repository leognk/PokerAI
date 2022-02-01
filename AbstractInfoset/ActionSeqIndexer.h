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

class ActionSeqIndexer
{
public:
	//typedef boomphf::mphf<T, ActionSeqHasher> phf_t;

	ActionSeqIndexer();

	static void init()
	{
		//phf_t phf(keys.size(), keys);
	}

	static uint64_t actionSeqIndex(const std::vector<uint8_t>& actionSeq)
	{
		return 0;// phf.lookup(actionSeq);
	}

private:


}; // ActionSeqIndexer

} // abc

#endif // ABC_ACTIONSEQINDEXER_H