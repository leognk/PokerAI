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

#include "../Utils/Memory.h"

int main()
{
	std::vector<std::vector<std::vector<int>>> regrets;
	regrets = {
		std::vector<std::vector<int>>(169, std::vector<int>(4311483)),
		std::vector<std::vector<int>>(50, std::vector<int>(1601014)),
		std::vector<std::vector<int>>(50, std::vector<int>(1483018)),
		std::vector<std::vector<int>>(50, std::vector<int>(1483018))
	};

	std::cout << std::left << std::setw(20) << "Total virtual: " << opt::totalVirtualMem() * 1e-9 << "\n";
	std::cout << std::left << std::setw(20) << "Virtual used: " << opt::virtualMemUsed() * 1e-9 << "\n";
	std::cout << std::left << std::setw(20) << "Virtual used by me: " << opt::virtualMemUsedByMe() * 1e-9 << "\n";
	std::cout << std::left << std::setw(20) << "Total RAM: " << opt::totalPhysMem() * 1e-9 << "\n";
	std::cout << std::left << std::setw(20) << "RAM used: " << opt::physMemUsed() * 1e-9 << "\n";
	std::cout << std::left << std::setw(20) << "RAM used by me: " << opt::physMemUsedByMe() * 1e-9 << "\n";
}