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

#include "../LossyAbstraction/KMeans.h"

int main()
{
	std::array<std::array<uint8_t, 2>, 10> data = { {
		{ 1, 1 },
		{ 2, 2 },
		{ 3, 3 },
		{ 4, 4 },
		{ 5, 5 },
		{ 6, 6 },
		{ 7, 7 },
		{ 8, 8 },
		{ 9, 9 },
		{ 10, 10 }
		//{ 101, 101 },
		//{ 102, 102 },
		//{ 103, 103 },
		//{ 104, 104 },
		//{ 105, 105 }
	} };
	std::array<uint8_t, data.size()> labels;
	abc::KMeans<uint8_t, 2> kmeans(false, 100, 100000, 1);
	kmeans.buildClusters(data, labels);
}