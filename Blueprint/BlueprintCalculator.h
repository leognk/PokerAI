#ifndef BP_BLUEPRINTCALCULATOR_H
#define BP_BLUEPRINTCALCULATOR_H

#include "Constants.h"
#include "../AbstractInfoset/AbstractInfoset.h"

namespace bp {

class BlueprintCalculator
{
public:
	BlueprintCalculator();

	void computeStrategy();

private:
	abc::AbstractInfoset<bckSize_t, N_BCK> abcInfo;

}; // BlueprintCalculator

} // bp

#endif // BP_BLUEPRINTCALCULATOR_H