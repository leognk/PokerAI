#ifndef BP_BLUEPRINTCALCULATOR_H
#define BP_BLUEPRINTCALCULATOR_H

#include "AbstractInfoset.h"

namespace bp {

class BlueprintCalculator
{
public:
	BlueprintCalculator(
		egn::chips ante = 0,
		egn::chips bigBlind = 100,
		egn::chips initialStake = 20000);

	void computeStrategy();

private:
	AbstractInfoset abcInfo;

}; // BlueprintCalculator

} // bp

#endif // BP_BLUEPRINTCALCULATOR_H