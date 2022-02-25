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

#include "../Utils/ioVar.h"

int main()
{
	std::vector<std::vector<std::vector<float>>> x = {
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		}
		};
	WRITE_VAR(std::cout, x);
}