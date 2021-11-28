#include "RandomAI.h"

namespace opt {

RandomAI::RandomAI()
{

}

uint32_t RandomAI::act(egn::GameState state)
{
	if (state.stake() <= state.call()) {

	}
	else if (state.notFacingFullRaise()) {

	}
	else if (state.stake() <= state.minRaise()) {
		if (state.call()) {

		}
		else {

		}
	}
	else {
		if (state.call()) {

		}
		else {

		}
	}
}

} // opt