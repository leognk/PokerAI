#include "../LossyAbstraction/DKEM.h"

int main()
{
	// 0: preflop
	// 1: flop
	// 2: turn
	const uint8_t round = 0;

	typedef uint8_t bckSize_t;
	const bckSize_t nBck = 8;

	const unsigned kMeansNRestarts = 10; // 25
	const unsigned kMeansMaxIter = 300; // 300
	const unsigned kMeansRngSeed = 0; // 0 for random seed
	const abc::KMeansInitMode kmeansInitMode = abc::KMeansInitMode::PlusPlusMaxMax; // PlusPlus

	abc::DKEM<bckSize_t, nBck> dkem(
		kMeansNRestarts, kMeansMaxIter, kMeansRngSeed, kmeansInitMode);
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