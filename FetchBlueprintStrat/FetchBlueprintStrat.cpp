#include "../Blueprint/Blueprint.h"

int main()
{
	bp::Blueprint blueprint(bp::BLUEPRINT_GAME_NAME, bp::BLUEPRINT_BUILD_NAME);
	blueprint.loadStrat();

	bp::abcInfo_t abcInfo(
		bp::MAX_PLAYERS,
		bp::ANTE,
		bp::BIG_BLIND,
		bp::INITIAL_STAKE,
		bp::BET_SIZES,
		bp::BLUEPRINT_GAME_NAME,
		0);

	for ()
}