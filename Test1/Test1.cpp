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

#include "../GameEngine/GameState.h"

int main()
{
	egn::GameState state(0, 100, {});
	for (auto& x : state.stakes)
		std::cout << x << " ";
	std::cout << "\n";
}