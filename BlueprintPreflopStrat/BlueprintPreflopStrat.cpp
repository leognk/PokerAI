#include "../Blueprint/Blueprint.h"

int main()
{
	static const uint8_t dealer = 0;

	static const std::vector<std::vector<uint8_t>> actionSeqs = {
		{ abc::FOLD },
		{ abc::CALL },
		{ abc::ALLIN },
		{ abc::RAISE + 1 },
		{ abc::RAISE + 2 },

		{ abc::CALL, abc::CALL },
		{ abc::CALL, abc::RAISE + 0 },
		{ abc::CALL, abc::RAISE + 1 },
		{ abc::CALL, abc::RAISE + 2 },
		{ abc::RAISE + 1, abc::FOLD },
		{ abc::RAISE + 1, abc::RAISE + 0 },
		{ abc::RAISE + 1, abc::RAISE + 1 },
		{ abc::RAISE + 1, abc::RAISE + 2 }
	};

	static const std::string dir = "../data/Blueprint/Tests/PreflopStrat/";

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

	for (const auto& actionSeq : actionSeqs) {

		std::vector<std::vector<uint8_t>> actionProbas(omp::RANK_COUNT, std::vector<uint8_t>(omp::RANK_COUNT));

		for (uint8_t handIdx = 0; handIdx < abc::PREFLOP_SIZE; ++handIdx) {

			// Prepare the infoset.
			abcInfo.setHoleCards();
			abcInfo.startNewHand(dealer, false);
			for (uint8_t i = 0; i < actionSeq.size() - 1; ++i)
				abcInfo.nextStateWithAction(actionSeq[i]);

			blueprint.calculateProbas(abcInfo);
		}
	}
}