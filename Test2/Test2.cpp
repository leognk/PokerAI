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

int main()
{
    Json::Value v;
    v["a"] = uint8_t(5);
    uint8_t x = v["a"].asUInt();
    std::cout << int(x) << "\n";
}