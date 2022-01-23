#include "BlueprintCalculator.h"

namespace bp {

BlueprintCalculator::BlueprintCalculator(
	egn::chips ante,
	egn::chips bigBlind,
	egn::chips initialStake) :

	abcInfo(ante, bigBlind, initialStake)
{
}

void BlueprintCalculator::computeStrategy()
{

}

} // bp