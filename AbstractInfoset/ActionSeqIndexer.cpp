#include "ActionSeqIndexer.h"

namespace abc {

ActionSeqIndexer::ActionSeqIndexer(
	uint8_t maxPlayers,
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake,
	const betSizes_t& betSizes,
	const std::string& indexerName,
	int nThreads, double gamma) :

	traverser(maxPlayers, ante, bigBlind, initialStake, betSizes, false),
	nThreads(nThreads),
	gamma(gamma),
	preflopMPHFPath(mphfDir + indexerName + "_PREFLOP_MPHF.bin"),
	flopMPHFPath(mphfDir + indexerName + "_FLOP_MPHF.bin"),
	turnMPHFPath(mphfDir + indexerName + "_TURN_MPHF.bin"),
	riverMPHFPath(mphfDir + indexerName + "_RIVER_MPHF.bin"),
	sizesPath(getSizesPath(indexerName))
{
}

std::string ActionSeqIndexer::getSizesPath(const std::string& indexerName)
{
	return mphfDir + indexerName + "_ACTION_SEQ_SIZES.bin";
}

void ActionSeqIndexer::buildMPHF()
{
	// Collect all action sequences for all rounds.
	std::vector<std::vector<seq_t>> actionSeqs = traverser.traverseTree();

	// Convert seq_t to seq_t::data_t.
	typedef std::vector<seq_t::data_t> keys_t;
	std::vector<keys_t> allKeys = {
		keys_t(actionSeqs[egn::PREFLOP].size()),
		keys_t(actionSeqs[egn::FLOP].size()),
		keys_t(actionSeqs[egn::TURN].size()),
		keys_t(actionSeqs[egn::RIVER].size())
	};
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {
		for (size_t i = 0; i < allKeys[r].size(); ++i)
			allKeys[r][i] = actionSeqs[r][i].data;
	}

	// Build MPHF.
	preflopMPHF = mphf_t(allKeys[egn::PREFLOP].size(), allKeys[egn::PREFLOP], nThreads, gamma, 0);
	flopMPHF = mphf_t(allKeys[egn::FLOP].size(), allKeys[egn::FLOP], nThreads, gamma, 0);
	turnMPHF = mphf_t(allKeys[egn::TURN].size(), allKeys[egn::TURN], nThreads, gamma, 0);
	riverMPHF = mphf_t(allKeys[egn::RIVER].size(), allKeys[egn::RIVER], nThreads, gamma, 0);
}

void ActionSeqIndexer::saveMPHF() const
{
	savePreflopMPHF();
	saveFlopMPHF();
	saveTurnMPHF();
	saveRiverMPHF();
	saveSizes();
}

void ActionSeqIndexer::loadMPHF()
{
	loadPreflopMPHF();
	loadFlopMPHF();
	loadTurnMPHF();
	loadRiverMPHF();
}

void ActionSeqIndexer::savePreflopMPHF() const
{
	auto file = opt::fstream(preflopMPHFPath, std::ios::out | std::ios::binary);
	preflopMPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveFlopMPHF() const
{
	auto file = opt::fstream(flopMPHFPath, std::ios::out | std::ios::binary);
	flopMPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveTurnMPHF() const
{
	auto file = opt::fstream(turnMPHFPath, std::ios::out | std::ios::binary);
	turnMPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveRiverMPHF() const
{
	auto file = opt::fstream(riverMPHFPath, std::ios::out | std::ios::binary);
	riverMPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveSizes() const
{
	auto file = opt::fstream(sizesPath, std::ios::out | std::ios::binary);
	opt::saveVar(preflopMPHF.nbKeys(), file);
	opt::saveVar(flopMPHF.nbKeys(), file);
	opt::saveVar(turnMPHF.nbKeys(), file);
	opt::saveVar(riverMPHF.nbKeys(), file);
	file.close();
}

void ActionSeqIndexer::loadPreflopMPHF()
{
	auto file = opt::fstream(preflopMPHFPath, std::ios::in | std::ios::binary);
	preflopMPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadFlopMPHF()
{
	auto file = opt::fstream(flopMPHFPath, std::ios::in | std::ios::binary);
	flopMPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadTurnMPHF()
{
	auto file = opt::fstream(turnMPHFPath, std::ios::in | std::ios::binary);
	turnMPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadRiverMPHF()
{
	auto file = opt::fstream(riverMPHFPath, std::ios::in | std::ios::binary);
	riverMPHF.load(file);
	file.close();
}

uint64_t ActionSeqIndexer::index(egn::Round round, const seq_t& actionSeq)
{
	switch (round) {

	case egn::PREFLOP:
		return preflopMPHF.lookup(actionSeq.data);

	case egn::FLOP:
		return flopMPHF.lookup(actionSeq.data);

	case egn::TURN:
		return turnMPHF.lookup(actionSeq.data);

	case egn::RIVER:
		return riverMPHF.lookup(actionSeq.data);

	default:
		throw std::runtime_error("Unknown round.");
	}
}

} // abc