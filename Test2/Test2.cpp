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

#include "../Utils/Histogram.h"

int main()
{
	const std::vector<int> v = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
	std::vector<int> xticks(4);
	const std::vector<uint64_t> hist = opt::buildHist(v, xticks);

	for (const auto& x : xticks)
		std::cout << x << " | ";
	std::cout << "\n";
	for (const auto& h : hist)
		std::cout << h << " | ";
	std::cout << "\n";
}