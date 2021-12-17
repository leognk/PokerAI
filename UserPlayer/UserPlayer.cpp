#include "UserPlayer.h"
#include <iomanip>
#include <sstream>

namespace opt {

UserPlayer::UserPlayer(std::string separatorLine) :
	mSeparatorLine(separatorLine)
{
}

egn::chips UserPlayer::act(const egn::GameState& state)
{
	std::vector<char> legalInputs = printLegalActions(state);
	egn::Action action = charToAction(getInputAction(legalInputs));
	egn::chips bet = getInputBet(action, state);
	std::cout << mSeparatorLine;
	return bet;
}

std::vector<char> UserPlayer::printLegalActions(const egn::GameState& state) const
{
	std::cout << "Choose an action:" << std::endl << std::endl;
	std::vector<char> legalInputs(state.nActions);
	for (uint8_t i = 0; i < state.nActions; ++i) {
		switch (state.actions[i]) {
		case egn::Action::fold:
			std::cout << "f: fold";
			legalInputs[i] = 'f';
			break;
		case egn::Action::call:
			std::cout << "c: call " << state.call;
			legalInputs[i] = 'c';
			break;
		case egn::Action::raise:
			std::cout << "r: raise "
				<< state.minRaise << " to " << state.allin;
			legalInputs[i] = 'r';
			break;
		case egn::Action::allin:
			std::cout << "a: all-in " << state.allin;
			legalInputs[i] = 'a';
			break;
		default:
			break;
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	return legalInputs;
}

char UserPlayer::getInputAction(const std::vector<char>& legalInputs) const
{
	char action = ' ';
	do {
		action = std::tolower(getInputChar("Enter a valid action: "));
	} while (std::find(legalInputs.begin(), legalInputs.end(), action)
		== legalInputs.end());
	return action;
}

egn::Action UserPlayer::charToAction(char c) const
{
	switch (c) {
	case 'f': return egn::Action::fold;
	case 'c': return egn::Action::call;
	case 'r': return egn::Action::raise;
	case 'a': return egn::Action::allin;
	default:
		throw std::runtime_error("Character not convertible to Action.");
	}
}

egn::chips UserPlayer::getInputRaise(egn::chips minRaise, egn::chips allin) const
{
	egn::chips raise = 0;
	do {
		std::stringstream message;
		message << "Enter an amount between "
			<< minRaise << " and " << allin << ": ";
		raise = getInputInt(message.str());
	} while (!(minRaise <= raise && raise <= allin));
	return raise;
}

egn::chips UserPlayer::getInputBet(egn::Action action, const egn::GameState& state) const
{
	switch (action) {
	case egn::Action::fold:
		return state.fold;
	case egn::Action::call:
		return state.call;
	case egn::Action::raise:
		return getInputRaise(state.minRaise, state.allin);
	case egn::Action::allin:
		return state.allin;
	default:
		throw std::runtime_error("Not a valid action.");
	}
}

char getInputChar(const std::string& message)
{
	std::string input;
	do {
		std::cout << message;
		std::getline(std::cin, input);
	} while (input.size() != 1);
	return input[0];
}

egn::chips getInputInt(const std::string& message)
{
	egn::chips x;
	std::cout << message;
	std::cin >> x;
	while (std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore(256, '\n');
		std::cout << message;
		std::cin >> x;
	}
	return x;
}

} // opt