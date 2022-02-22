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

#include "../Utils/StringManip.h"

int main()
{
	std::cout << opt::prettySmallNum(5786.255e-3, 2, true) << "\n";
}