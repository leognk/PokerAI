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

typedef std::array<uint64_t, 3> T;

class ActionSeqHasher
{
public:
	template<typename key_t>
	uint64_t operator()(key_t key, uint64_t seed = 0) const
	{
		return XXH3_64bits_withSeed(&key, sizeof(key), seed);
	}
};

int main()
{
	typedef boomphf::mphf<T, ActionSeqHasher> phf_t;

	std::vector<T> keys = {
		{ 5, 8, 3 },
		{ 5, 8, 2 },
		{ 0, 8, 2 },
		{ 0, 77, 2 },
		{ 0, 0, 0 }
	};
	phf_t phf(keys.size(), keys);

	for (const T& key : keys)
		std::cout << phf.lookup(key) << "\n";
}