#include "../AbstractInfoset/GroupedActionSeqsInv.h"
#include "../Blueprint/Constants.h"
#include "../Utils/Time.h"

int main()
{
	opt::time_t startTime = opt::getTime();

	abc::GroupedActionSeqs gpSeqs(bp::BLUEPRINT_GAME_NAME);
	gpSeqs.build(
		bp::MAX_PLAYERS,
		bp::ANTE,
		bp::BIG_BLIND,
		bp::INITIAL_STAKE,
		bp::BET_SIZES);
	gpSeqs.save();

	abc::GroupedActionSeqsInv gpSeqsInv(bp::BLUEPRINT_GAME_NAME);
	gpSeqsInv.build();
	gpSeqsInv.save();

	std::cout << "Duration: " << opt::prettyDuration(startTime) << "\n";
}