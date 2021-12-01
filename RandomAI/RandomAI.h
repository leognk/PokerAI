#ifndef OPT_RANDOMAI_H
#define OPT_RANDOMAI_H

#include "../GameEngine/Player.h"

namespace opt {

class RandomAI : public egn::Player
{
public:
	// Set rngSeed to 0 to set a random seed.
	RandomAI(unsigned rngSeed = 0);
	uint32_t act(egn::GameState state);

private:
	typedef omp::XoroShiro128Plus Rng;

	Rng mRng;
	omp::FastUniformIntDistribution<uint32_t, 32> mRaiseDist;

}; // RandomAI

} // opt

#endif // OPT_RANDOMAI_H