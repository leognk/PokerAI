#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>
#include <locale>
#include <bitset>

#include "../LossyAbstraction/Metrics.h"

int main()
{
	const uint32_t nBins = 5;
	std::array<std::array<uint32_t, nBins>, 1> data = { 3, 0, 1, 4, 2 };
	std::vector<uint32_t> labels(nBins, 0);
	uint32_t label = 0;
	uint32_t weight = 1;
	std::vector<uint32_t> center(nBins);
	abc::emdCenter(data, labels, label, weight, center);

	for (uint32_t k = 0; k < nBins; ++k)
		std::cout << center[k] << "  ";
	std::cout << "\n";
}