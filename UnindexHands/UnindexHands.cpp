#include <string>
#include <fstream>
#include "../LosslessAbstraction/hand_index.h"
#include "../GameEngine/Hand.h"

int main()
{
	abc::hand_indexer_t indexer(2, { 2, 5 });
	const uint8_t handSize = 7;
	const uint8_t roundToUnindex = 1;

	const uint32_t startIdx = 0;
	const uint32_t endIdx = startIdx + 3;

	const std::string dir = "../data/AbstractionSaves/Tests/VisualizeOCHS/";
	const std::string fileName = "unindexed_hands.txt";

	std::ofstream file(dir + fileName, std::ios_base::app);
	for (uint32_t i = startIdx; i < endIdx; ++i) {
		uint8_t hand[handSize];
		indexer.hand_unindex(roundToUnindex, i, hand);
		file << i << ": " << egn::Hand(hand, handSize) << "\n";
	}
}