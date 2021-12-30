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
#include <boost/regex/icu.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <json/json.h>

namespace fs = std::filesystem;

template<uint8_t size>
std::array<uint8_t, size> getArr()
{
	std::array<uint8_t, size> a;
	for (uint8_t i = 0; i < size; ++i)
		a[i] = i;
	return a;
}

int main()
{
	std::array<uint8_t, 6> a = getArr<6>();
	for (uint8_t x : a)
		std::cout << std::to_string(x) << "\n";
}