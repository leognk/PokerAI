#include "../Blueprint/BlueprintCalculator.h"

int main()
{
	const double maxDuration = 3; // in seconds
	const unsigned rngSeed = 1;
	const uint64_t updatePeriod = (uint64_t)1e3;

	bp::BlueprintCalculator calculator(rngSeed, false);
	double duration;

	// Calculate blueprint.
	opt::time_t startTime = opt::getTime();
	while (true) {
		calculator.oneIter();
		if (calculator.currIter % updatePeriod == 0) {
			duration = opt::getDuration(startTime);
			if (duration > maxDuration) break;
		}
	}

	// Print results.
	std::cout
		<< opt::prettyDuration(duration) << "\n"
		<< opt::prettyNumber(calculator.currIter, 2, true) << " iter\n"
		<< (unsigned)std::round(calculator.currIter / duration) << " iter/sec\n"
		<< opt::prettyNumber(duration / calculator.currIter, 2, true) << "sec/iter\n";
}