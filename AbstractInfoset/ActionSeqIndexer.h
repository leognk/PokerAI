#ifndef ABC_ACTIONSEQINDEXER_H
#define ABC_ACTIONSEQINDEXER_H

#include "TreeTraverser.h"
#include "../Utils/Hash.h"
#include "../Utils/Hash.h"
#include "../Utils/io.h"
#include "../Utils/Constants.h"
#include "../BBHash/BooPHF.h"

namespace abc {

static const std::string mphfDir = opt::dataDir + "ActionSequences/MPHF/";

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
	void saveMPHF() const;
	void loadMPHF();

	static std::string getSizesPath(const std::string& indexerName);

	uint64_t index(egn::Round round, const seq_t& actionSeq);

	mphf_t preflopMPHF, flopMPHF, turnMPHF, riverMPHF;
	TreeTraverser traverser;

private:
	void savePreflopMPHF() const;
	void saveFlopMPHF() const;
	void saveTurnMPHF() const;
	void saveRiverMPHF() const;
	void saveSizes() const;

	void loadPreflopMPHF();
	void loadFlopMPHF();
	void loadTurnMPHF();
	void loadRiverMPHF();

	const int nThreads;
	const double gamma;

	const std::string preflopMPHFPath;
	const std::string flopMPHFPath;
	const std::string turnMPHFPath;
	const std::string riverMPHFPath;
	const std::string sizesPath;

}; // ActionSeqIndexer

} // abc

#endif // ABC_ACTIONSEQINDEXER_H