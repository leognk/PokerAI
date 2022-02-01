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

#include "xxhash.h"
#include "../BBHash/BooPHF.h"
#include "../AbstractInfoset/ActionSeqIndexer.h"
#include "../Blueprint/Constants.h"

typedef std::vector<uint8_t> T;

class ActionSeqHasher
{
public:
	template<typename T>
	uint64_t operator()(std::vector<T> v, uint64_t seed = 0) const
	{
		return XXH3_64bits_withSeed(v.data(), v.size() * sizeof(T), seed);
	}
};

int main()
{
	//typedef boomphf::mphf<T, ActionSeqHasher> phf_t;

	//std::vector<T> keys = {
	//	{ 5 },
	//	{ 5, 2 },
	//	{ 5, 2, 3 },
	//	{ 5, 2, 77 },
	//	{ 52, 2, 77 },
	//	{ 52, 8, 77 },
	//	{ 8, 77 },
	//	{ 52, 8, 77, 100, 3, 5, 7, 65, 88, 88, 0, 0, 0 },
	//	{ 52, 8, 77, 100, 3, 5, 7, 65, 88, 88, 0, 0 }
	//};

	//phf_t phf(keys.size(), keys, 1, 2.0);

	//std::cout << "\n";
	//for (const T& key : keys)
	//	std::cout << phf.lookup(key) << "\n";

	abc::ActionSeqIndexer indexer(bp::ANTE, bp::BIG_BLIND, bp::INITIAL_STAKE, bp::BET_SIZES, 1, 2.0);

	std::cout << indexer.index(egn::PREFLOP, { 1, 0, 0 }) << "\n";
}