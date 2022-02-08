#include "BlueprintCalculator.h"

namespace bp {

BlueprintCalculator::BlueprintCalculator() :
	abcInfo(
		bp::MAX_PLAYERS,
		bp::ANTE,
		bp::BIG_BLIND,
		bp::INITIAL_STAKE,
		bp::BET_SIZES,
		bp::ACTION_SEQ_INDEXER_NAME)
{
}

void BlueprintCalculator::computeStrategy()
{

}

} // bp