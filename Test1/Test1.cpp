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
//#include <fstream>
//#include <bitset>

#include "../AbstractInfoset/ActionSeqIdx.h"

int main()
{
	abc::ActionSeqIdx idx1;
	idx1.addAction(7);
	idx1.addAction(1);

	abc::ActionSeqIdx idx2;
	idx2.addAction(7);
	idx2.addAction(1);

	std::cout << (idx1 == idx2) << "\n";
}