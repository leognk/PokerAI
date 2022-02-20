#include "../Blueprint/BlueprintCalculator.h"

int main()
{
	const unsigned rngSeed = 1;

	bp::BlueprintCalculator calculator(rngSeed);
	calculator.buildStrategy();
}