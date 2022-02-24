#include "../Blueprint/BlueprintCalculator.h"

int main()
{
	const unsigned rngSeed = 1;

	bp::BlueprintCalculator calculator(rngSeed, false);
	try {
		calculator.buildStrategy();
	}
	catch (...) {
		abc::printAbcInfoHist(calculator.abcInfo.hist);
	}
}