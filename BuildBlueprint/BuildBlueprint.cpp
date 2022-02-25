#include "../Blueprint/BlueprintCalculator.h"

int main()
{
	const bool resumeFromCheckpoint = false;
	const unsigned rngSeed = 1;

	bp::BlueprintCalculator calculator(resumeFromCheckpoint, rngSeed);
	calculator.buildStrategy();
}