#include "../Blueprint/Blueprint.h"

// Return two vectors mapping a preflop canonical hand index to
// the corresponding row and column indices in the table view of
// the preflop canonical hands:
// 
//  22  23s 24s 25s ... 2As
//  32  33  34s 35s ... 3As
//  42  43  44  45s ... 4As
//  52  53  54  55  ... 5As
//   .   .   .   .   .   .
//   .   .   .   .   .   .
//   .   .   .   .   .   .
//  A2  A3  A4  A5  ... AA
//
std::pair<std::vector<uint8_t>, std::vector<uint8_t>> mapHandIdxToRowCol()
{
	std::vector<uint8_t> handIdxToRow(abc::PREFLOP_SIZE);
	std::vector<uint8_t> handIdxToCol(abc::PREFLOP_SIZE);
	uint8_t handIdx = 0;
	for (uint8_t i = 0; i < omp::RANK_COUNT; ++i) {
		for (uint8_t j = 0; j <= i; ++j) {
			handIdxToRow[handIdx] = i;
			handIdxToCol[handIdx] = j;
			++handIdx;
		}
	}
	for (uint8_t j = 1; j < omp::RANK_COUNT; ++j) {
		for (uint8_t i = 0; i < j; ++i) {
			handIdxToRow[handIdx] = i;
			handIdxToCol[handIdx] = j;
			++handIdx;
		}
	}
	return { handIdxToRow, handIdxToCol };
}

// Return the player who makes the last action in actionSeq.
uint8_t getActingPlayer(
	const std::vector<uint8_t>& actionSeq,
	bp::abcInfo_t abcInfo, uint8_t dealer)
{
	abcInfo.resetStakes();
	abcInfo.startNewHand(dealer, true);
	for (uint8_t i = 0; i < actionSeq.size() - 1; ++i)
		abcInfo.nextStateWithAction(actionSeq[i], true);
	return abcInfo.state.actingPlayer;
}

// Go to the state after doing the actions in actionSeq except the last
// and set the hole cards of the player acting on the last action to handIdx.
void prepareAbcInfo(
	bp::abcInfo_t& abcInfo, const uint8_t dealer,
	const std::vector<uint8_t>& actionSeq, uint8_t handIdx)
{
	abcInfo.state.resetUsedCards();

	// Set the hole cards corresponding to handIdx.
	const uint8_t player = getActingPlayer(actionSeq, abcInfo, dealer);
	uint8_t hand[omp::HOLE_CARDS];
	abc::EquityCalculator::preflopIndexer.hand_unindex(0, handIdx, hand);
	abcInfo.state.setHoleCards(player, hand);

	// Deal random cards for the other players.
	for (uint8_t i = 0; i < bp::MAX_PLAYERS; ++i) {
		if (i != player)
			abcInfo.state.setRandomHoleCards(i);
	}

	// Not necessary in theory as we stay in the preflop but just in case.
	abcInfo.state.setRandomBoardCards();

	// Go to the state after doing the actions in actionSeq except the last.
	abcInfo.resetStakes();
	abcInfo.startNewHand(dealer, true, false);
	for (uint8_t i = 0; i < actionSeq.size() - 1; ++i)
		abcInfo.nextStateWithAction(actionSeq[i], true);
}

std::string actionSeqToStr(const std::vector<uint8_t>& actionSeq)
{
	std::string res = "";
	for (uint8_t i = 0; i < actionSeq.size(); ++i) {
		if (i != 0) res += "_";
		res += abc::abcActionToStr(actionSeq[i]);
	}
	return res;
}

int main()
{
	static const uint8_t dealer = 0;

	static const std::vector<std::vector<uint8_t>> actionSeqs = {
		{ abc::FOLD },
		{ abc::CALL },
		{ abc::ALLIN },
		{ abc::RAISE + 0 },
		{ abc::RAISE + 1 },
		{ abc::RAISE + 2 },

		{ abc::CALL, abc::CALL },
		{ abc::CALL, abc::RAISE + 0 },
		{ abc::CALL, abc::RAISE + 1 },
		{ abc::CALL, abc::RAISE + 2 },
		{ abc::RAISE + 0, abc::FOLD },
		{ abc::RAISE + 0, abc::RAISE + 0 },
		{ abc::RAISE + 0, abc::RAISE + 1 },
		{ abc::RAISE + 0, abc::RAISE + 2 }
	};

	static const std::string dir = opt::dataDir + "Blueprint/Tests/PreflopStrat/" + bp::blueprintName() + "/";
	std::filesystem::create_directory(dir);

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

	const auto [handIdxToRow, handIdxToCol] = mapHandIdxToRowCol();

	for (const auto& actionSeq : actionSeqs) {

		std::vector<std::vector<uint8_t>> actionProbas(
			omp::RANK_COUNT, std::vector<uint8_t>(omp::RANK_COUNT));

		for (uint8_t handIdx = 0; handIdx < abc::PREFLOP_SIZE; ++handIdx) {
			prepareAbcInfo(abcInfo, dealer, actionSeq, handIdx);
			const auto i = handIdxToRow[handIdx];
			const auto j = handIdxToCol[handIdx];
			const auto probas = blueprint.calculateProbasPerc(abcInfo);
			actionProbas[i][j] = probas[abcInfo.getActionId(actionSeq.back())];
		}

		opt::save2DVector(actionProbas, dir + actionSeqToStr(actionSeq) + ".bin");
	}
}