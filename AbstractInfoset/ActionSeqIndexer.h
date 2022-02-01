#ifndef ABC_ACTIONSEQINDEXER_H
#define ABC_ACTIONSEQINDEXER_H

#include "../AbstractInfoset/TreeTraverser.h"
#include "xxhash.h"
#include "../BBHash/BooPHF.h"

namespace abc {

class VectorHasher
{
public:
	template<typename T>
	uint64_t operator()(std::vector<T> v, uint64_t seed = 0) const
	{
		return XXH3_64bits_withSeed(v.data(), v.size() * sizeof(T), seed);
	}
};

class ActionSeqIndexer
{
public:
	typedef std::vector<uint8_t> seq_t;
	typedef boomphf::mphf<seq_t, VectorHasher> phf_t;

	ActionSeqIndexer(
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const std::vector<std::vector<std::vector<float>>>& betSizes,
		int nThreads = 1, double gamma = 2.0);

	uint64_t index(egn::Round round, const seq_t& actionSeq);

private:
	void init();
	void initPreflopPHF();
	void initFlopPHF();
	void initTurnPHF();
	void initRiverPHF();

	TreeTraverser traverser;
	phf_t preflopPHF, flopPHF, turnPHF, riverPHF;

	const int nThreads;
	const double gamma;

}; // ActionSeqIndexer

} // abc

#endif // ABC_ACTIONSEQINDEXER_H