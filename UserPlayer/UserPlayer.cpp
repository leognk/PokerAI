#include "UserPlayer.h"
#include <iomanip>
#include <sstream>

namespace opt {

UserPlayer::UserPlayer(std::string separatorLine) :
	mSeparatorLine(separatorLine)
{
}

void UserPlayer::act(egn::GameState& state)
{
	std::vector<char> legalInputs = printLegalActions(state);
	state.action = charToAction(getInputAction(legalInputs));
	state.bet = getInputBet(state);
	std::cout << mSeparatorLine;
}

std::vector<char> UserPlayer::printLegalActions(const egn::GameState& state) const
{
	std::cout << "Choose an action:" << std::endl << std::endl;
	std::vector<char> legalInputs(state.nActions);
	for (uint8_t i = 0; i < state.nActions; ++i) {
		switch (state.actions[i]) {
		case egn::FOLD:
			std::cout << "f: fold";
			legalInputs[i] = 'f';
			break;
		case egn::CALL:
			std::cout << "c: call " << state.call;
			legalInputs[i] = 'c';
			break;
		case egn::RAISE:
			std::cout << "r: raise "
				<< state.minRaise << " to " << state.allin;
			legalInputs[i] = 'r';
			break;
		default:
			throw std::runtime_error("Unknown action.");
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
	case 'f': return egn::FOLD;
	case 'c': return egn::CALL;
	case 'r': return egn::RAISE;
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

egn::chips UserPlayer::getInputBet(const egn::GameState& state) const
{
	if (state.action == egn::RAISE) {
		if (state.minRaise < state.allin)
			return getInputRaise(state.minRaise, state.allin);
		else
			return state.allin;
	}
	else
		return 0;
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