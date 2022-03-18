#include <string>
#include <fstream>
#include <vector>
#include "../LosslessAbstraction/hand_index.h"
#include "../GameEngine/Hand.h"
#include "../Utils/Constants.h"

int main()
{
	abc::hand_indexer_t indexer(2, { 2, 5 });
	const uint8_t handSize = 7;
	const uint8_t roundToUnindex = 1;

	const std::vector<uint32_t> startIds = {
		0, 10000000, 20000000, 30000000, 40000000, 50000000, 60000000,
		70000000, 80000000, 90000000, 100000000, 110000000, 120000000 };
	const std::vector<uint32_t> lengths(startIds.size(), 3);

	const std::string dir = opt::dataDir + "AbstractionSaves/Tests/VisualizeOCHS/";
	const std::string fileName = "unindexed_hands.txt";

	std::ofstream file(dir + fileName);
	for (unsigned k = 0; k < startIds.size(); ++k) {
		for (uint32_t i = startIds[k]; i < startIds[k] + lengths[k]; ++i) {
			uint8_t hand[handSize];
			indexer.hand_unindex(roundToUnindex, i, hand);
			file << i << ": " << egn::Hand(hand, 2) << " "
				<< egn::Hand(&hand[2], 5) << "\n";
		}
	}
}