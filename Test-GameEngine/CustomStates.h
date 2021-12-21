#ifndef CUSTOMSTATES_H
#define CUSTOMSTATES_H

#include "../GameEngine/GameState.h"
#include <json/json.h>

namespace cus {

static const std::string customStatesPath =
"C:/Users/leota/Desktop/Info/Poker/My projets/Optimus/Test-GameEngine/CustomStates.json";

struct State
{
	unsigned idx;

	std::array<egn::chips, opt::MAX_PLAYERS> stakes{};

	uint8_t actingPlayer;
	egn::chips call, minRaise, allin;
	std::array<egn::Action, 3> actions{};
	uint8_t nActions;

	egn::Round round;
	bool finished;

	egn::Action nextAction;
	egn::chips nextBet;
};

struct History
{
	unsigned id;
	egn::chips ante;
	egn::chips bb;
	uint8_t dealer;
	std::array<egn::Hand, opt::MAX_PLAYERS> hands{};
	egn::Hand boardCards;
	std::array<egn::chips, opt::MAX_PLAYERS> initialStakes{};
	std::vector<State> states;
	std::array<egn::dchips, opt::MAX_PLAYERS> rewards{};
};

bool boolFromString(const std::string& str)
{
	if (str == "true")
		return true;
	else if (str == "false")
		return false;
	else
		throw std::runtime_error("String does not represent a bool.");
}

std::istream& operator>>(std::istream& is, std::vector<History>& listHist)
{
	Json::Value listHistJs;
	is >> listHistJs;

	// Loop over list of histories.
	for (const Json::Value& histJs : listHistJs) {

		// Read history.
		History hist;

		hist.id = histJs["id"].asUInt();
		hist.ante = histJs["ante"].asUInt();
		hist.bb = histJs["bb"].asUInt();
		hist.dealer = histJs["dealer"].asUInt();
		// Read hands.
		uint8_t i = 0;
		for (const Json::Value& hand : histJs["hands"]) {
			hist.hands[i] = egn::Hand(hand.asString());
			++i;
		}
		hist.boardCards = egn::Hand::empty()
			+ egn::Hand(histJs["boardCards"].asString());
		// Read initialStakes.
		i = 0;
		for (const Json::Value& s : histJs["initialStakes"]) {
			hist.initialStakes[i] = s.asUInt();
			++i;
		}

		// Read states.
		for (const Json::Value& stateJs : histJs["states"]) {
			State state;

			state.idx = stateJs["idx"].asUInt();

			// Read stakes.
			uint8_t i = 0;
			for (const Json::Value& s : stateJs["stakes"]) {
				state.stakes[i] = s.asUInt();
				++i;
			}
			
			state.actingPlayer = stateJs["actingPlayer"].asUInt();
			state.call = stateJs["call"].asUInt();
			state.minRaise = stateJs["minRaise"].asUInt();
			state.allin = stateJs["allin"].asUInt();

			// Read actions.
			state.nActions = 0;
			for (const Json::Value& a : stateJs["actions"]) {
				state.actions[state.nActions] =
					egn::actionFromString(a.asString());
				++state.nActions;
			}

			state.round = egn::roundFromString(stateJs["round"].asString());
			state.finished = boolFromString(stateJs["finished"].asString());

			state.nextAction = egn::actionFromString(
				stateJs["nextAction"].asString());
			state.nextBet = stateJs["nextBet"].asUInt();

			hist.states.emplace_back(state);
		}

		// Read rewards.
		i = 0;
		for (const Json::Value& r : histJs["rewards"]) {
			hist.rewards[i] = r.asInt();
			++i;
		}

		listHist.emplace_back(hist);
	}
	return is;
}

} // cus

#endif // CUSTOMSTATES_H