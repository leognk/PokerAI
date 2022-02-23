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
	std::cout << opt::prettyNumDg(57.85e-20, 5) << "\n";
}