#include "GroupedActionSeqs.h"
#include "../Utils/ioContainer.h"

namespace abc {

GroupedActionSeqs::GroupedActionSeqs(
	uint8_t maxPlayers,
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake,
	const betSizes_t& betSizes,
	const std::string& groupedActionSeqsName) :

	indexer(maxPlayers, ante, bigBlind, initialStake, betSizes, groupedActionSeqsName),
	filePath(groupedActionSeqsDir + groupedActionSeqsName + "_GROUPED_ACTION_SEQS.bin")
{
}

void GroupedActionSeqs::build()
{
	indexer.loadMPHF();

	// Allocate memory for seqs and lens.
	seqs = {
		std::vector<seqIdx_t>(indexer.preflopMPHF.nbKeys()),
		std::vector<seqIdx_t>(indexer.flopMPHF.nbKeys()),
		std::vector<seqIdx_t>(indexer.turnMPHF.nbKeys()),
		std::vector<seqIdx_t>(indexer.riverMPHF.nbKeys())
	};
	lens = std::vector<std::vector<uint8_t>>(egn::N_ROUNDS);

	// Generate all action sequences.
	auto actionSeqs = indexer.traverser.traverseTree();

	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {

		// Sort the sequences.
		// Use a comparator taking into account the number of players
		// at the end of a sequence for rounds other than preflop.
		if (r == egn::PREFLOP)
			std::sort(actionSeqs[r].begin(), actionSeqs[r].end());
		else
			std::sort(actionSeqs[r].begin(), actionSeqs[r].end(),
				abc::compareSeqsWithNPlayers<ActionSeqIndexer::seq_t>);

		// Put the sequences' indices in seqs and the lengths of each group
		// of sequences in lens, which is the number of legal actions
		// available from a sequence's parent node.
		for (seqIdx_t i = 0; i < actionSeqs[r].size(); ++i) {
#pragma warning(suppress: 4244)
			seqs[r][i] = indexer.index(egn::Round(r), actionSeqs[r][i]);
			if (i != 0 && abc::seqsHaveSameParent(actionSeqs[r][i], actionSeqs[r][i - 1]))
				++lens[r].back();
			else
				lens[r].push_back(1);
		}
	}
}

void GroupedActionSeqs::save()
{
	auto file = std::fstream(filePath, std::ios::out | std::ios::binary);

	// Save the sizes of the vectors in seqs.
	std::vector<size_t> seqsSizes;
	for (const auto& roundSeqs : seqs)
		seqsSizes.push_back(roundSeqs.size());
	file.write((char*)&seqsSizes[0], seqsSizes.size() * sizeof(size_t));

	// Save seqs.
	for (const auto& w : seqs)
		file.write((char*)&w[0], w.size() * sizeof(w[0]));

	// Save the sizes of the vectors in lens.
	std::vector<size_t> lensSizes;
	for (const auto& roundLens : lens)
		lensSizes.push_back(roundLens.size());
	file.write((char*)&lensSizes[0], lensSizes.size() * sizeof(size_t));

	// Save lens.
	for (const auto& w : lens)
		file.write((char*)&w[0], w.size() * sizeof(w[0]));

	file.close();
}

void GroupedActionSeqs::load()
{
	auto file = std::fstream(filePath, std::ios::in | std::ios::binary);

	// Allocate memory for seqs.
	std::vector<size_t> seqsSizes(egn::N_ROUNDS);
	file.read((char*)&seqsSizes[0], seqsSizes.size() * sizeof(size_t));
	seqs = {
		std::vector<seqIdx_t>(seqsSizes[egn::PREFLOP]),
		std::vector<seqIdx_t>(seqsSizes[egn::FLOP]),
		std::vector<seqIdx_t>(seqsSizes[egn::TURN]),
		std::vector<seqIdx_t>(seqsSizes[egn::RIVER])
	};

	// Load seqs.
	for (auto& w : seqs)
		file.read((char*)&w[0], w.size() * sizeof(w[0]));

	// Allocate memory for lens.
	std::vector<size_t> lensSizes(egn::N_ROUNDS);
	file.read((char*)&lensSizes[0], lensSizes.size() * sizeof(size_t));
	lens = {
		std::vector<uint8_t>(lensSizes[egn::PREFLOP]),
		std::vector<uint8_t>(lensSizes[egn::FLOP]),
		std::vector<uint8_t>(lensSizes[egn::TURN]),
		std::vector<uint8_t>(lensSizes[egn::RIVER])
	};

	// Load seqs.
	for (auto& w : lens)
		file.read((char*)&w[0], w.size() * sizeof(w[0]));

	file.close();
}

} // abc