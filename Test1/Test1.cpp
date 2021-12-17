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

namespace fs = std::filesystem;

int main()
{
    auto t1 = std::chrono::steady_clock::now();
    std::ifstream file(hdt::compressedHandDataFile);
    while (true) {
        hdt::HandHistory hist;
        if (!(hdt::readCompressedData(file, hist))) break;
    }
    auto t2 = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
    std::cout << duration << " s" << "\n";
}