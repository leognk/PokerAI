#ifndef ABC_ACTIONSEQINDEXER_H
#define ABC_ACTIONSEQINDEXER_H

#include "../AbstractInfoset/TreeTraverser.h"
#include "../Utils/Hash.h"
#include "../BBHash/BooPHF.h"

namespace abc {

static const std::string mphfDir = "../data/ActionSequences/MPHF/";

class ActionSeqIndexer
{
public:
	typedef StdActionSeq seq_t;
	typedef boomphf::mphf<seq_t::data_t, opt::ArrayHash> mphf_t;

	// indexerName is used in the MPHF files names.
	ActionSeqIndexer(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const betSizes_t& betSizes,
		const std::string& indexerName,
		int nThreads = 1, double gamma = 2.0);

	void buildMPHF();
	void saveMPHF();
	void loadMPHF();

	uint64_t index(egn::Round round, const seq_t& actionSeq);

	mphf_t preflopMPHF, flopMPHF, turnMPHF, riverMPHF;

private:
	void savePreflopMPHF();
	void saveFlopMPHF();
	void saveTurnMPHF();
	void saveRiverMPHF();

	void loadPreflopMPHF();
	void loadFlopMPHF();
	void loadTurnMPHF();
	void loadRiverMPHF();

	TreeTraverser traverser;

	const int nThreads;
	const double gamma;

	const std::string preflopMPHFPath;
	const std::string flopMPHFPath;
	const std::string turnMPHFPath;
	const std::string riverMPHFPath;

}; // ActionSeqIndexer

} // abc

#endif // ABC_ACTIONSEQINDEXER_H