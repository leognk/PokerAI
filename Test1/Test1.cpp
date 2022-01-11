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

#include "../LossyAbstraction/Metrics.h"

//namespace fs = std::filesystem;

int main()
{
	//std::array<uint8_t, 5> u1 = { 3, 0, 0, 0, 0 };
	//std::array<uint8_t, 5> u2 = { 0, 0, 3, 0, 0 };
	//std::array<uint8_t, 5> u3 = { 0, 0, 0, 0, 3 };
	//std::array<uint8_t, 5> m1 = { 1, 0, 1, 0, 1 };
	//std::array<uint8_t, 5> m2 = { 0, 0, 3, 0, 0 };
	//std::cout << abc::emd(u1, m1) << ", " << abc::emd(u2, m1) << ", " << abc::emd(u3, m1) << "\n";
	//std::cout << abc::emd(u1, m2) << ", " << abc::emd(u2, m2) << ", " << abc::emd(u3, m2) << "\n";

	std::array<uint8_t, 3> u1 = { 1, 2, 3 };
	std::array<uint8_t, 3> u2 = { 1, 2, 3 };
	std::array<uint8_t, 3> u3 = { 0, 6, 0 };
	std::array<std::array<uint8_t, 3>, 3> data = { u1, u2, u3 };
	std::array<uint8_t, 3> center;
	abc::emdCenter(data, center);
	for (uint8_t h : center)
		std::cout << std::to_string(h) << "  ";
	std::cout << "\n";
}