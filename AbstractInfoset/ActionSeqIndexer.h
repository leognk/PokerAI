#ifndef ABC_ACTIONSEQINDEXER_H
#define ABC_ACTIONSEQINDEXER_H

#include "../AbstractInfoset/TreeTraverser.h"
#include "xxhash.h"
#include "../BBHash/BooPHF.h"

namespace abc {

static const std::string phfDir = "../data/ActionSequenceMPHF/";

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

	// indexerName is used in the MPHF files names.
	ActionSeqIndexer(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const std::vector<std::vector<std::vector<float>>>& betSizes,
		const std::string& indexerName,
		int nThreads = 1, double gamma = 2.0);

	void buildPHF();
	void savePHF();
	void loadPHF();

	uint64_t index(egn::Round round, const seq_t& actionSeq);

private:
	void buildPreflopPHF();
	void buildFlopPHF();
	void buildTurnPHF();
	void buildRiverPHF();

	void savePreflopPHF();
	void saveFlopPHF();
	void saveTurnPHF();
	void saveRiverPHF();

	void loadPreflopPHF();
	void loadFlopPHF();
	void loadTurnPHF();
	void loadRiverPHF();

	TreeTraverser traverser;
	phf_t preflopPHF, flopPHF, turnPHF, riverPHF;

	const int nThreads;
	const double gamma;

	const std::string preflopPHFPath;
	const std::string flopPHFPath;
	const std::string turnPHFPath;
	const std::string riverPHFPath;

}; // ActionSeqIndexer

} // abc

#endif // ABC_ACTIONSEQINDEXER_H