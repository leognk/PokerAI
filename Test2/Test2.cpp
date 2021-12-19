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
    unsigned id = 1;
    unsigned id0 = 89135;
    for (const auto& folder : fs::directory_iterator(hdt::handDataRoot)) {
        for (const auto& filePath : fs::directory_iterator(folder)) {
            std::ifstream is(filePath);
            while (true) {
                hdt::HandHistory hist;
                if (is >> hist) {
                    if (id == id0) {
                        std::cout << filePath << "\n\n";
                        return 0;
                    }
                    ++id;
                }
                else if (is.bad())
                    throw std::runtime_error("Error while reading file.");
                else if (is.eof())
                    break;
                else if (is.fail())
                    is.clear();
            }
        }
    }
}