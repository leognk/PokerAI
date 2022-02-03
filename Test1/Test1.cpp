#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
//#include <random>
//#include <chrono>
//#include <filesystem>
//#include <fstream>
//#include <bitset>

#include "../AbstractInfoset/ActionSeqIndexer.h"
#include "../Blueprint/Constants.h"

int main()
{
	abc::ActionSeqIndexer indexer(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, bp::ACTION_SEQ_INDEXER_NAME);
	indexer.loadPHF();

	abc::TreeTraverser traverser(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, true, false);

	for (uint8_t i = 0; i < egn::N_ROUNDS; ++i) {

		std::vector<std::vector<uint8_t>> actionSeqs;
		traverser.traverseRoundTree(egn::Round(i), actionSeqs);

		std::vector<uint64_t> indices(actionSeqs.size());
		for (size_t j = 0; j < actionSeqs.size(); ++j)
			indices[j] = indexer.index(egn::Round(i), actionSeqs[j]);

		std::vector<bool> flags(actionSeqs.size(), false);
		std::vector<bool> printed(actionSeqs.size(), false);
		for (size_t j = 0; j < actionSeqs.size(); ++j) {

			if (indices[j] >= actionSeqs.size()) {

				std::cout << "Round: " << egn::Round(i) << "\n";
				std::cout << "Hash: " << indices[j] << "\n";
				std::cout << "Index: " << j << "\n";
				std::cout << "Seq: ";
				for (uint8_t a : actionSeqs[j])
					std::cout << std::setw(2) << std::to_string(a) << " ";
				std::cout << "\n\n";

				continue;
			}

			if (printed[indices[j]]) continue;

			if (flags[indices[j]]) {

				std::cout << "Round: " << egn::Round(i) << "\n";
				std::cout << "Hash: " << indices[j] << "\n";
				std::cout << "Index: " << j << "\n";
				std::cout << "Seq: ";
				for (uint8_t a : actionSeqs[j])
					std::cout << std::setw(2) << std::to_string(a) << " ";
				std::cout << "\n";

				for (size_t k = 0; k < actionSeqs.size(); ++k) {

					if (k != j && indices[k] == indices[j]) {

						std::cout << "Index: " << k << "\n";
						std::cout << "Seq: ";
						for (uint8_t a : actionSeqs[k])
							std::cout << std::setw(2) << std::to_string(a) << " ";
						std::cout << "\n";
					}
				}
				std::cout << "\n";
				printed[indices[j]] = true;
			}
			flags[indices[j]] = true;
		}
	}
}