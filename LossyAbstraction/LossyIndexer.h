#ifndef ABC_LOSSYINDEXER_H
#define ABC_LOSSYINDEXER_H

#include "DKEM.h"
#include "KOC.h"
#include "../GameEngine/GameState.h"

namespace abc {

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
class LossyIndexer
{
public:
	static void loadLUT()
	{
		if constexpr (nBckPreflop < abc::PREFLOP_SIZE)
			dkem.loadPreflopBckLUT();
		dkem.loadFlopBckLUT();
		dkem.loadTurnBckLUT();
		koc.loadRivBckLUT();
	}

	static bckSize_t handIndex(
		const egn::Round round, const uint8_t hand[], const uint8_t board[])
	{
		switch (round) {

		case egn::PREFLOP: {
			if constexpr (nBckPreflop < abc::PREFLOP_SIZE)
				return dkem.PREFLOP_BCK_LUT[
					abc::EquityCalculator::preflopIndexer.hand_index_last(hand)];
			else
				return abc::EquityCalculator::preflopIndexer.hand_index_last(hand);
		}

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
	static abc::DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn> dkem;
	static abc::KOC<bckSize_t, nBckRiver> koc;

}; // LossyIndexer

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
abc::DKEM<bckSize_t, nBckPreflop, nBckFlop, nBckTurn> LossyIndexer<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver>::dkem;
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
abc::KOC<bckSize_t, nBckRiver> LossyIndexer<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver>::koc;

} // abc

#endif // ABC_LOSSYINDEXER_H