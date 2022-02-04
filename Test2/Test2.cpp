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

int main()
{
	std::vector<int> a = { 7, 8, 2 };
	a = { 1, 2 };
	for (auto x : a)
		std::cout << x << " ";
	std::cout << "\n";
}