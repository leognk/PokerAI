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

#include "../GameEngine/GameState.h"
#include "../HandData/HandData.h"
#include "../Test-GameEngine/CustomStates.h"

namespace fs = std::filesystem;

enum Round { preflop, flop, turn, river };

std::ostream& operator<<(std::ostream& os, const Round& r)
{
	switch (r) {
	case preflop:
		return os << "preflop";
	case flop:
		return os << "flop";
	case turn:
		return os << "turn";
	case river:
		return os << "river";
	default:
		return os;
	}
}

int main()
{
    Round r = Round(preflop + 1);
    std::cout << r << "\n";
}