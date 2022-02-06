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
	int x = 3;
	int y = 3;
	bool b = true;
	(b ? x : y) = 5;
	std::cout << x << " " << y << "\n";
}