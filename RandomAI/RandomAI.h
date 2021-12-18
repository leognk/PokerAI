#ifndef OPT_RANDOMAI_H
#define OPT_RANDOMAI_H

#include "../GameEngine/Player.h"
#include "../Utils/Random.h"

namespace opt {

// Random AI which randomly chooses an action among legal ones.
class RandomAI : public egn::Player
{
public:
	// Raising proba will be set to 1 - foldProba - callProba.
	// Set rngSeed to 0 to set a random seed.
	RandomAI(
		double foldProba = 1./3, double callProba = 1./3,
		unsigned rngSeed = 0);
	egn::chips act(const egn::GameState& state) override;

private:
	typedef omp::XoroShiro128Plus Rng;

	Rng mRng;
	FastRandomChoice<16> mRandChoice;
	omp::FastUniformIntDistribution<egn::chips, 32> mRaiseDist;

	double mFoldProba, mCallProba, mRaiseProba;

}; // RandomAI

} // opt

#endif // OPT_RANDOMAI_H