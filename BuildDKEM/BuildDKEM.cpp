#include "../LossyAbstraction/DKEM.h"

int main()
{
	// 0: preflop
	// 1: flop
	// 2: turn
	const uint8_t round = 0;

	typedef uint8_t bckSize_t;
	const bckSize_t nBck = 8;

	const unsigned kMeansNRestarts = 100; // 25
	const unsigned kMeansMaxIter = 300; // 300
	const uint64_t kMeansInvTolerance = 0; // 0 or 1000
	const unsigned kMeansRngSeed = 1; // 0 for random seed
	const abc::KMeansInitMode kmeansInitMode = abc::KMeansInitMode::PlusPlusMax; // PlusPlus
	const abc::KMeansIterMode kmeansIterMode = abc::KMeansIterMode::Elkan; // Elkan

	abc::DKEM<bckSize_t, nBck> dkem(
		kMeansNRestarts, kMeansMaxIter, kMeansInvTolerance,
		kMeansRngSeed, kmeansInitMode, kmeansIterMode);
	switch (round) {
	case 0:
		dkem.populatePreflopBckLUT();
		dkem.savePreflopBckLUT();
		break;
	case 1:
		dkem.populateFlopBckLUT();
		dkem.saveFlopBckLUT();
		break;
	case 2:
		dkem.populateTurnBckLUT();
		dkem.saveTurnBckLUT();
		break;
	default:
		break;
	}
}