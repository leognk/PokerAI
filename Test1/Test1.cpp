#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <random>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <bitset>

#include "../AbstractInfoset/GroupedActionSeqs.h"
#include "../Blueprint/Constants.h"

int main()
{
	abc::GroupedActionSeqs gpSeqs(bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES, bp::BLUEPRINT_NAME);
	gpSeqs.load();

	std::cout << gpSeqs.seqs.size() << "\n";
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r)
		std::cout << gpSeqs.seqs[r].size() << " | " << gpSeqs.lens[r].size() << "\n";
	std::cout << "\n";

	abc::TreeTraverser traverser(bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES);
	auto actionSeqs = traverser.traverseTree();

	abc::ActionSeqIndexer indexer(bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES, bp::BLUEPRINT_NAME);
	indexer.loadMPHF();

	std::vector<std::unordered_map<size_t, abc::StdActionSeq>> idxToSeq(egn::N_ROUNDS);
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {
		for (const auto& seq : actionSeqs[r]) {
			idxToSeq[r][indexer.index(egn::Round(r), seq)] = seq;
		}
	}

	const uint8_t r = 1;
	const size_t nSeqs = 15;
	const size_t nLens = 5;

	for (size_t i = 0; i < nSeqs; ++i) {
		auto v = abc::seqToVect(idxToSeq[r][gpSeqs.seqs[r][i]]);
		for (auto x : v)
			std::cout << std::setw(2) << std::to_string(x) << " ";
		std::cout << "\n";
	}
	std::cout << "\n";

	for (size_t i = 0; i < nLens; ++i)
		std::cout << std::to_string(gpSeqs.lens[r][i]) << "\n";
}