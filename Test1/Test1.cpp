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

#include "../AbstractInfoset/ActionSeq.h"
#include "../BBHash/BooPHF.h"
#include "../Utils/Hash.h"

int main()
{
	typedef abc::StdActionSeq T;

	std::vector<T::data_t> keys(3);
	T seq;

	seq.push_back(5);
	seq.push_back(7);
	seq.push_back(0);
	keys[0] = seq.data;

	seq.clear();
	seq.push_back(1);
	keys[1] = seq.data;

	seq.clear();
	seq.push_back(15);
	seq.push_back(3);
	keys[2] = seq.data;

	typedef boomphf::mphf<T::data_t, opt::ContainerHash> mphf_t;
	mphf_t mphf(keys.size(), keys, 1, 2.0, 0);

	for (const auto& k : keys)
		std::cout << mphf.lookup(k) << "\n";
}