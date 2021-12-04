#ifndef OPT_USERPLAYER_H
#define OPT_USERPLAYER_H

#include "../GameEngine/Player.h"

namespace opt {

// User player class which receives inputs for legal actions.
class UserPlayer : public egn::Player
{
public:
	UserPlayer(std::string separtorLine = "");
	uint32_t act(const egn::GameState& state) override;

private:
	std::vector<char> printLegalActions(const egn::GameState& state) const;
	char getInputAction(const std::vector<char>& legalInputs) const;
	egn::Action charToAction(char c) const;
	uint32_t getInputRaise(uint32_t minRaise, uint32_t allin) const;
	uint32_t getInputBet(egn::Action action, const egn::GameState& state) const;

	std::string mSeparatorLine;

}; // UserPlayer

char getInputChar(const std::string& message);
uint32_t getInputInt(const std::string& message);

} // opt

#endif // OPT_USERPLAYER_H