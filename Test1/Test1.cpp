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
#include <fstream>
//#include <bitset>

#include "../GameEngine/GameState.h"
#include "../LosslessAbstraction/hand_index.h"
#include "../LossyAbstraction/EquityCalculator.h"
#include "../GameEngine/Hand.h"

//namespace fs = std::filesystem;

int main()
{
	const uint32_t nSamples = 6;
	uint8_t nEmpty = 2;
	std::array<uint64_t, nSamples> dists = { 1, 5, 50, 2, 99, 100 };
	std::vector<uint32_t> farFromCenters(nSamples);
	std::iota(farFromCenters.begin(), farFromCenters.end(), uint32_t(0));
	std::nth_element(
		farFromCenters.begin(),
		farFromCenters.begin() + nEmpty,
		farFromCenters.end(),
		[&dists](uint32_t i, uint32_t j) { return dists[i] > dists[j]; });
	farFromCenters.erase(farFromCenters.begin() + nEmpty, farFromCenters.end());
	for (uint32_t i : farFromCenters)
		std::cout << i << "\n";
}