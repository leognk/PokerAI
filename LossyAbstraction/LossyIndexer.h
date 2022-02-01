#ifndef ABC_LOSSYINDEXER_H
#define ABC_LOSSYINDEXER_H

#include "DKEM.h"
#include "KOC.h"
#include "../GameEngine/GameState.h"

namespace abc {

template<typename bckSize_t, bckSize_t nBck>
class LossyIndexer
{
public:
	static void loadLUT()
	{
		dkem.loadFlopBckLUT();
		dkem.loadTurnBckLUT();
		koc.loadRivBckLUT();
	}

	static bckSize_t handIndex(
		const egn::Round round, const uint8_t hand[], const uint8_t board[])
	{
		switch (round) {

		case egn::PREFLOP:
			return abc::EquityCalculator::preflopIndexer.hand_index_last(hand);

		case egn::FLOP: {
			uint8_t cards[omp::FLOP_HAND] = {
				hand[0], hand[1], board[0], board[1], board[2] };
			return dkem.FLOP_BCK_LUT[
				abc::EquityCalculator::flopIndexer.hand_index_last(cards)];
		}

		case egn::TURN: {
			uint8_t cards[omp::TURN_HAND] = {
				hand[0], hand[1], board[0], board[1], board[2], board[3] };
			return dkem.TURN_BCK_LUT[
				abc::EquityCalculator::cmbTurnIndexer.hand_index_last(cards)];
		}

		case egn::RIVER: {
			uint8_t cards[omp::RIVER_HAND] = {
				hand[0], hand[1], board[0], board[1], board[2], board[3], board[4] };
			return koc.RIV_BCK_LUT[
				abc::EquityCalculator::cmbRivIndexer.hand_index_last(cards)];
		}

		default:
			throw std::runtime_error("Unknown round.");
		}
	}

private:
	static abc::DKEM<bckSize_t, nBck> dkem;
	static abc::KOC<bckSize_t, nBck> koc;

}; // LossyIndexer

template<typename bckSize_t, bckSize_t nBck>
abc::DKEM<bckSize_t, nBck> LossyIndexer<bckSize_t, nBck>::dkem;
template<typename bckSize_t, bckSize_t nBck>
abc::KOC<bckSize_t, nBck> LossyIndexer<bckSize_t, nBck>::koc;

} // abc

#endif // ABC_LOSSYINDEXER_H