#ifndef ABC_ABSTRACTINFOSETDEBUG_H
#define ABC_ABSTRACTINFOSETDEBUG_H

#include "AbstractInfoset.h"

namespace abc {

// Same as AbstractInfoset but with additional variables for debugging.
template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
class AbstractInfosetDebug : public AbstractInfoset<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver>
{
public:

	typedef AbstractInfoset<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver> abcInfo_t;

	AbstractInfosetDebug(
		uint8_t maxPlayers,
		egn::chips ante,
		egn::chips bigBlind,
		egn::chips initialStake,
		const betSizes_t& betSizes,
		const std::string& actionSeqIndexerName,
		unsigned rngSeed) :

		abcInfo_t::AbstractInfoset(maxPlayers, ante, bigBlind, initialStake, betSizes, actionSeqIndexerName, rngSeed),
		print(false),
		id(0),
		traverser(0)
	{
	}

	AbstractInfosetDebug& operator=(const AbstractInfosetDebug& other)
	{
		if (this == &other) return *this;
		abcInfo_t::operator=(other);

		players = other.players;
		actions = other.actions;
		actionIds = other.actionIds;
		bets = other.bets;

		// id is different to count if other was pushed back into a FastVector.
		if (id == count && print) {
			std::cout << *this;
			++count;
			id = count;
		}
		id = other.id;

		return *this;
	}

	void startNewHand(uint8_t currTraverser = 0, uint64_t currIter = 0, bool mustPrint = false)
	{
		abcInfo_t::startNewHand();

		print = mustPrint;
		players.clear();
		actions.clear();
		actionIds.clear();
		bets.clear();
		count = 0;
		id = 0;
		traverser = currTraverser;

		if (print)
			std::cout
				<< printSep << "\n\n" << currIter + 1 << "/" << bp::endIter
				<< " | " << std::to_string(traverser) << "/" << std::to_string(bp::MAX_PLAYERS - 1) << "\n\n";
	}

	void nextState(uint8_t actionId)
	{
		players.push_back(this->state.actingPlayer);
		actions.push_back(this->actionAbc.legalActions[actionId]);
		actionIds.push_back(actionId);

		abcInfo_t::nextState(actionId);

		bets.push_back(this->state.bets[players.back()]);

		if (print) std::cout << *this;
		++count;
		id = count;
	}

	static const std::string printSep;
	bool print;

	std::vector<uint8_t> players;
	std::vector<uint8_t> actions;
	std::vector<uint8_t> actionIds;
	std::vector<egn::chips> bets;

	static unsigned count;
	unsigned id;
	uint8_t traverser;

}; // AbstractInfosetDebug

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
std::ostream& operator<<(std::ostream& os,
	const AbstractInfosetDebug<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver>& abcInfo)
{
	os << std::setw(3) << abcInfo.count;

	for (uint8_t i = 0; i < abcInfo.actions.size(); ++i) {
		os << " | " << std::to_string(abcInfo.players[i]) << ": " << std::to_string(abcInfo.actions[i]);
		if (abcInfo.players[i] == abcInfo.traverser) os << "/" << std::to_string(abcInfo.actionIds[i]);
		os << " (" << abcInfo.bets[i] << ")";
	}

	os << "\n";
	return os;
}

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
const std::string AbstractInfosetDebug<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver>::printSep(100, '_');

template<typename bckSize_t, bckSize_t nBckPreflop, bckSize_t nBckFlop, bckSize_t nBckTurn, bckSize_t nBckRiver>
unsigned AbstractInfosetDebug<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver>::count = 0;

} // abc

#endif // ABC_ABSTRACTINFOSETDEBUG_H