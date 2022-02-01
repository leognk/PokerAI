#include "ActionSeqIndexer.h"

namespace abc {

ActionSeqIndexer::ActionSeqIndexer(
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake,
	const std::vector<std::vector<std::vector<float>>>& betSizes,
	int nThreads, double gamma) :

	traverser(ante, bigBlind, initialStake, betSizes, true, false),
	nThreads(nThreads),
	gamma(gamma)
{
	init();
}

void ActionSeqIndexer::init()
{
	std::cout << "begin preflop PHF" << "\n";
	initPreflopPHF();
	std::cout << "begin flop PHF" << "\n";
	initFlopPHF();
	std::cout << "begin turn PHF" << "\n";
	initTurnPHF();
	std::cout << "begin river PHF" << "\n";
	initRiverPHF();
	std::cout << "finished PHF" << "\n";
}

void ActionSeqIndexer::initPreflopPHF()
{
	std::vector<seq_t> actionSeqs;
	traverser.traverseRoundTree(egn::PREFLOP, actionSeqs);
	preflopPHF = phf_t(actionSeqs.size(), actionSeqs, nThreads, gamma);
}

void ActionSeqIndexer::initFlopPHF()
{
	std::vector<seq_t> actionSeqs;
	traverser.traverseRoundTree(egn::FLOP, actionSeqs);
	flopPHF = phf_t(actionSeqs.size(), actionSeqs, nThreads, gamma);
}

void ActionSeqIndexer::initTurnPHF()
{
	std::vector<seq_t> actionSeqs;
	traverser.traverseRoundTree(egn::TURN, actionSeqs);
	turnPHF = phf_t(actionSeqs.size(), actionSeqs, nThreads, gamma);
}

void ActionSeqIndexer::initRiverPHF()
{
	std::vector<seq_t> actionSeqs;
	traverser.traverseRoundTree(egn::RIVER, actionSeqs);
	riverPHF = phf_t(actionSeqs.size(), actionSeqs, nThreads, gamma);
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