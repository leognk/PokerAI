#include "ActionSeqIndexer.h"
#include <fstream>

namespace abc {

ActionSeqIndexer::ActionSeqIndexer(
	uint8_t maxPlayers,
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake,
	const std::vector<std::vector<std::vector<float>>>& betSizes,
	const std::string& indexerName,
	int nThreads, double gamma) :

	traverser(maxPlayers, ante, bigBlind, initialStake, betSizes, true, false),
	nThreads(nThreads),
	gamma(gamma),
	preflopPHFPath(phfDir + indexerName + "_PREFLOP_MPHF.bin"),
	flopPHFPath(phfDir + indexerName + "_FLOP_MPHF.bin"),
	turnPHFPath(phfDir + indexerName + "_TURN_MPHF.bin"),
	riverPHFPath(phfDir + indexerName + "_RIVER_MPHF.bin")
{
}

void ActionSeqIndexer::buildPHF()
{
	buildPreflopPHF();
	buildFlopPHF();
	buildTurnPHF();
	buildRiverPHF();
}

void ActionSeqIndexer::savePHF()
{
	savePreflopPHF();
	saveFlopPHF();
	saveTurnPHF();
	saveRiverPHF();
}

void ActionSeqIndexer::loadPHF()
{
	loadPreflopPHF();
	loadFlopPHF();
	loadTurnPHF();
	loadRiverPHF();
}

void ActionSeqIndexer::buildPreflopPHF()
{
	std::vector<seq_t> actionSeqs;
	traverser.traverseRoundTree(egn::PREFLOP, actionSeqs);
	preflopPHF = phf_t(actionSeqs.size(), actionSeqs, nThreads, gamma);
}

void ActionSeqIndexer::buildFlopPHF()
{
	std::vector<seq_t> actionSeqs;
	traverser.traverseRoundTree(egn::FLOP, actionSeqs);
	flopPHF = phf_t(actionSeqs.size(), actionSeqs, nThreads, gamma);
}

void ActionSeqIndexer::buildTurnPHF()
{
	std::vector<seq_t> actionSeqs;
	traverser.traverseRoundTree(egn::TURN, actionSeqs);
	turnPHF = phf_t(actionSeqs.size(), actionSeqs, nThreads, gamma);
}

void ActionSeqIndexer::buildRiverPHF()
{
	std::vector<seq_t> actionSeqs;
	traverser.traverseRoundTree(egn::RIVER, actionSeqs);
	riverPHF = phf_t(actionSeqs.size(), actionSeqs, nThreads, gamma);
}

void ActionSeqIndexer::savePreflopPHF()
{
	auto file = std::fstream(preflopPHFPath, std::ios::out | std::ios::binary);
	preflopPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveFlopPHF()
{
	auto file = std::fstream(flopPHFPath, std::ios::out | std::ios::binary);
	flopPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveTurnPHF()
{
	auto file = std::fstream(turnPHFPath, std::ios::out | std::ios::binary);
	turnPHF.save(file);
	file.close();
}

void ActionSeqIndexer::saveRiverPHF()
{
	auto file = std::fstream(riverPHFPath, std::ios::out | std::ios::binary);
	riverPHF.save(file);
	file.close();
}

void ActionSeqIndexer::loadPreflopPHF()
{
	auto file = std::fstream(preflopPHFPath, std::ios::in | std::ios::binary);
	preflopPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadFlopPHF()
{
	auto file = std::fstream(flopPHFPath, std::ios::in | std::ios::binary);
	flopPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadTurnPHF()
{
	auto file = std::fstream(turnPHFPath, std::ios::in | std::ios::binary);
	turnPHF.load(file);
	file.close();
}

void ActionSeqIndexer::loadRiverPHF()
{
	auto file = std::fstream(riverPHFPath, std::ios::in | std::ios::binary);
	riverPHF.load(file);
	file.close();
}

uint64_t ActionSeqIndexer::index(egn::Round round, const seq_t& actionSeq)
{
	switch (round) {

	case egn::PREFLOP:
		return preflopPHF.lookup(actionSeq);

	case egn::FLOP:
		return flopPHF.lookup(actionSeq);

	case egn::TURN:
		return turnPHF.lookup(actionSeq);

	case egn::RIVER:
		return riverPHF.lookup(actionSeq);

	default:
		throw std::runtime_error("Unknown round.");
	}
}

} // abc