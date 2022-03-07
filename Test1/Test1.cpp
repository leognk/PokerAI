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

int f(std::vector<int> v)
{
	std::sort(v.begin(), v.end());
	return v[0];
}

int main()
{
	std::vector<int> v = { 3, 2, 1 };
	int x = f(v);
	std::cout << v[0] << "\n" << x << "\n";
}