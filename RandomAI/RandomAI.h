#ifndef OPT_RANDOMAI_H
#define OPT_RANDOMAI_H

#include "../GameEngine/Player.h"

namespace opt {

class RandomAI : public egn::Player
{
public:
	RandomAI();
	uint32_t act(egn::GameState state);

private:


}; // RandomAI

} // opt

#endif // OPT_RANDOMAI_H