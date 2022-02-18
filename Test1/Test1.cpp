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

#include "../AbstractInfoset/TreeTraverser.h"
#include "../Blueprint/Constants.h"

int main()
{
	abc::TreeTraverser traverser(bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES, false);
	auto seqs = traverser.traverseTree();
	uint8_t r = 1;
	for (unsigned i = 0; i < 100; ++i) {
		auto v = abc::seqToVect(seqs[r][i]);
		for (auto a : v)
			std::cout << std::setw(2) << std::to_string(a) << " ";
		std::cout << "\n";
	}
}