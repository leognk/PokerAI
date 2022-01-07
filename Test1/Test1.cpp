#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>
#include <array>
//#include <random>
//#include <chrono>
//#include <filesystem>
#include <fstream>
//#include <bitset>

#include "../GameEngine/GameState.h"
#include "../LosslessAbstraction/hand_index.h"
#include "../LossyAbstraction/EquityCalculator.h"
#include "../GameEngine/Hand.h"
#include "../Utils/Random.h"

//namespace fs = std::filesystem;

int main()
{
	opt::XoShiro256PlusPlus rng(0);
	for (unsigned i = 0; i < 100; ++i)
		std::cout << rng() << "\n";
}