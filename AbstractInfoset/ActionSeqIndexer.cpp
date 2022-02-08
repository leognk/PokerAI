#include "ActionSeqIndexer.h"
#include <fstream>

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
	preflopMPHFPath(phfDir + indexerName + "_PREFLOP_MPHF.bin"),
	flopMPHFPath(phfDir + indexerName + "_FLOP_MPHF.bin"),
	turnMPHFPath(phfDir + indexerName + "_TURN_MPHF.bin"),
	riverMPHFPath(phfDir + indexerName + "_RIVER_MPHF.bin")
{
}

void ActionSeqIndexer::buildMPHF()
{
	std::vector<std::vector<seq_t>> actionSeqs = traverser.traverseTree();
	preflopMPHF = mphf_t(actionSeqs[egn::PREFLOP].size(), actionSeqs[egn::PREFLOP], nThreads, gamma, 0);
	flopMPHF = mphf_t(actionSeqs[egn::FLOP].size(), actionSeqs[egn::FLOP], nThreads, gamma, 0);
	turnMPHF = mphf_t(actionSeqs[egn::TURN].size(), actionSeqs[egn::TURN], nThreads, gamma, 0);
	riverMPHF = mphf_t(actionSeqs[egn::RIVER].size(), actionSeqs[egn::RIVER], nThreads, gamma, 0);
}

void ActionSeqIndexer::saveMPHF()
{
	savePreflopMPHF();
	saveFlopMPHF();
	saveTurnMPHF();
	saveRiverMPHF();
}

void ActionSeqIndexer::loadMPHF()
{
	loadPreflopMPHF();
	loadFlopMPHF();
	loadTurnMPHF();
	loadRiverMPHF();
}

void ActionSeqIndexer::savePreflopMPHF()
{
	auto file = std::fstream(preflopMPHFPath, std::ios::out | std::ios::binary);
	preflopMPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveFlopMPHF()
{
	auto file = std::fstream(flopMPHFPath, std::ios::out | std::ios::binary);
	flopMPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveTurnMPHF()
{
	auto file = std::fstream(turnMPHFPath, std::ios::out | std::ios::binary);
	turnMPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveRiverMPHF()
{
	auto file = std::fstream(riverMPHFPath, std::ios::out | std::ios::binary);
	riverMPHF.save(file);
	file.close();
}

void ActionSeqIndexer::loadPreflopMPHF()
{
	auto file = std::fstream(preflopMPHFPath, std::ios::in | std::ios::binary);
	preflopMPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadFlopMPHF()
{
	auto file = std::fstream(flopMPHFPath, std::ios::in | std::ios::binary);
	flopMPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadTurnMPHF()
{
	auto file = std::fstream(turnMPHFPath, std::ios::in | std::ios::binary);
	turnMPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadRiverMPHF()
{
	auto file = std::fstream(riverMPHFPath, std::ios::in | std::ios::binary);
	riverMPHF.load(file);
	file.close();
}

uint64_t ActionSeqIndexer::index(egn::Round round, const seq_t& actionSeq)
{
	switch (round) {

	case egn::PREFLOP:
		return preflopMPHF.lookup(actionSeq);

	case egn::FLOP:
		return flopMPHF.lookup(actionSeq);

	case egn::TURN:
		return turnMPHF.lookup(actionSeq);

	case egn::RIVER:
		return riverMPHF.lookup(actionSeq.data);

	default:
		throw std::runtime_error("Unknown round.");
	}
}

} // abc