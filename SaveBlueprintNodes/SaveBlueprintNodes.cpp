#include "../AbstractInfoset/GroupedActionSeqs.h"
#include "../Blueprint/Constants.h"

int main()
{
	auto startTime = std::chrono::high_resolution_clock::now();

	abc::GroupedActionSeqs gpSeqs(
		bp::MAX_PLAYERS,
		bp::ANTE,
		bp::BIG_BLIND,
		bp::INITIAL_STAKE,
		bp::BET_SIZES,
		bp::BLUEPRINT_NAME);

	gpSeqs.build();
	gpSeqs.save();

	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
	std::cout << "Duration: " << std::round(duration) << " sec\n";
}