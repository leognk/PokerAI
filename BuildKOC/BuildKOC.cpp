#include "../LossyAbstraction/KOC.h"

int main()
{
	typedef uint8_t bckSize_t;
	const bckSize_t nBck = 200;

	const unsigned kMeansNRestarts = 1; // 25
	const unsigned kMeansMaxIter = 300; // 300
	const unsigned kMeansRngSeed = 1; // 0 for random seed
	abc::KMeansInitMode kmeansInitMode = abc::KMeansInitMode::PlusPlus; // PlusPlus

	abc::KOC<bckSize_t, nBck> koc(
		kMeansNRestarts, kMeansMaxIter, kMeansRngSeed, kmeansInitMode);
	koc.populateRivBckLUT();
	koc.saveRivBckLUT();
}