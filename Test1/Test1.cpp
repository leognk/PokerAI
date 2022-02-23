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
	std::cout << std::setw(6) << opt::prettyNumDg(1.65, 3, true) << "s/it\n";
}