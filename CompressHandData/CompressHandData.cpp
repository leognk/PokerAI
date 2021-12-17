#include "../HandData/HandData.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

int main()
{
    // Count files.
    uint64_t nFiles = 0;
    for (const auto& folder : fs::directory_iterator(hdt::handDataRoot)) {
        for (const auto& filePath : fs::directory_iterator(folder))
            ++nFiles;
    }

    // Read files and save them in a compressed form.

    uint64_t fileCount = 0;
    uint64_t nValid = 0;
    uint64_t nInvalid = 0;
    std::ofstream compressedDataFile(hdt::compressedHandDataFile);
    auto t1 = std::chrono::steady_clock::now();

    // Loop over folders.
    for (const auto& folder : fs::directory_iterator(hdt::handDataRoot)) {

        // Loop over files.
        for (const auto& filePath : fs::directory_iterator(folder)) {
            std::ifstream is(filePath);

            while (true) {
                // Read a hand.
                hdt::HandHistory hist;
                if (is >> hist) {
                    // Save the hand in a compressed form.
                    hdt::writeCompressedData(compressedDataFile, hist);
                    ++nValid;
                }
                else if (is.bad())
                    throw std::runtime_error("Error while reading file.");
                else if (is.eof())
                    break;
                else if (is.fail()) {
                    is.clear();
                    ++nInvalid;
                }
            }
            ++fileCount;

            // Print progression.
            if (fileCount % 10 == 0 || fileCount == nFiles) {
                auto t2 = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
                std::cout << fileCount << "/" << nFiles << " files"
                    << " | " << nValid << "/" << nInvalid << " valid/invalid hands"
                    << " | " << duration << " s" << "\n";
            }
        }
    }
}