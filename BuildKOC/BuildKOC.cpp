#include "../LossyAbstraction/KOC.h"

int main()
{
	typedef uint8_t bckSize_t;
	const bckSize_t nBckRiver = 200;

	const unsigned kMeansNRestarts = 1; // 25
	const unsigned kMeansMaxIter = 300; // 300
	const uint64_t kMeansInvTolerance = 0; // 0 or 1000
	const unsigned kMeansRngSeed = 1; // 0 for random seed
	const abc::KMeansInitMode kmeansInitMode = abc::KMeansInitMode::PlusPlus; // PlusPlus
	const abc::KMeansIterMode kmeansIterMode = abc::KMeansIterMode::Elkan; // Elkan

	abc::KOC<bckSize_t, nBckRiver> koc(
		kMeansNRestarts, kMeansMaxIter, kMeansInvTolerance,
		kMeansRngSeed, kmeansInitMode, kmeansIterMode);
	koc.populateRivBckLUT();
	koc.saveRivBckLUT();
}