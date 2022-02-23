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
		<< opt::prettyNumDg(calculator.currIter, 3, true) << "it\n"
		<< opt::prettyNumDg(calculator.currIter / duration, 3, true) << "it/s\n"
		<< opt::prettyNumDg(duration / calculator.currIter, 3, true) << "sec/it\n";
}