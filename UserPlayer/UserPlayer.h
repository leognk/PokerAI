#ifndef OPT_USERPLAYER_H
#define OPT_USERPLAYER_H

#include "../GameEngine/Player.h"
#include <vector>

namespace opt {

// User player class which receives inputs for legal actions.
class UserPlayer : public egn::Player
{
public:
	UserPlayer(const std::string& separatorLine = "");
	void act(egn::GameState& state) override;

private:
	std::vector<char> printLegalActions(const egn::GameState& state) const;
	char getInputAction(const std::vector<char>& legalInputs) const;
	egn::Action charToAction(char c) const;
	egn::chips getInputRaise(egn::chips minRaise, egn::chips allin) const;
	egn::chips getInputBet(const egn::GameState& state) const;

	const std::string mSeparatorLine;

}; // UserPlayer

char getInputChar(const std::string& message);
egn::chips getInputInt(const std::string& message);

} // opt

#endif // OPT_USERPLAYER_H