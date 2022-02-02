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
		bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES, "BLUEPRINT");
	indexer.loadPHF();

	std::cout << indexer.index(egn::FLOP, { 0, 6 }) << "\n";
}