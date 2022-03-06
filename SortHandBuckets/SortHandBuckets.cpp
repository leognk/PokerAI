#include "../LossyAbstraction/LossyIndexer.h"
#include "../Blueprint/Constants.h"

// Return the strengths a round's hand buckets.
// The strength of a bucket is defined to be the average
// of the equities of the canonical hands composing the bucket.
template<typename BCKLUT, typename HSLUT>
std::vector<double> calculateBckStrengths(
	const size_t nBck, const BCKLUT& bckLUT, const HSLUT& hsLUT)
{
	std::vector<double> bckStrengths(nBck);
	std::vector<uint64_t> bckWeights(nBck);

	// Loop over all canonical hands and update their corresponding bucket's strength.
	for (size_t handIdx = 0; handIdx < bckLUT.size(); ++handIdx) {
		bckStrengths[bckLUT[handIdx]] += (double)hsLUT[handIdx];
		++bckWeights[bckLUT[handIdx]];
	}

	// Normalize bckStrengths.
	for (size_t i = 0; i < nBck; ++i)
		bckStrengths[i] /= bckWeights[i];

	return bckStrengths;
}

// Return a round's hand buckets sorted by the buckets' strengths.
template<typename bckSize_t, typename BCKLUT, typename HSLUT>
std::vector<bckSize_t> sortBcksByStrength(
	const bckSize_t nBck, const BCKLUT& bckLUT, const HSLUT& hsLUT)
{
	std::vector<bckSize_t> sortedBcks(nBck);
	std::iota(sortedBcks.begin(), sortedBcks.end(), bckSize_t(0));

	const std::vector<double> bckStrengths = calculateBckStrengths(nBck, bckLUT, hsLUT);

	std::sort(sortedBcks.begin(), sortedBcks.end(),
		[&](bckSize_t i, bckSize_t j) { return bckStrengths[i] > bckStrengths[j]; });

	return sortedBcks;
}

template<typename bckSize_t, typename BCKLUT, typename HSLUT>
void buildAndSaveSortedBcks(
	const bckSize_t nBck, const BCKLUT& bckLUT, const HSLUT& hsLUT,
	const std::string& filePath)
{
	const std::vector<bckSize_t> sortedBcks = sortBcksByStrength(nBck, bckLUT, hsLUT);
	opt::save1DVector(sortedBcks, filePath);
}

int main()
{
	typedef bp::bckSize_t bckSize_t;
	static const bckSize_t nBckPreflop = bp::N_BCK_PREFLOP;
	static const bckSize_t nBckFlop = bp::N_BCK_FLOP;
	static const bckSize_t nBckTurn = bp::N_BCK_TURN;
	static const bckSize_t nBckRiver = bp::N_BCK_RIVER;

	static const std::string dir = "../data/AbstractionSaves/SortedBck/";

	static const std::string preflopFilePath = dir + std::format("PREFLOP_{}_BCK.bin", nBckPreflop);
	static const std::string flopFilePath = dir + std::format("FLOP_{}_BCK.bin", nBckFlop);
	static const std::string turnFilePath = dir + std::format("TURN_{}_BCK.bin", nBckTurn);
	static const std::string riverFilePath = dir + std::format("RIVER_{}_BCK.bin", nBckRiver);

	typedef abc::LossyIndexer<bckSize_t, nBckPreflop, nBckFlop, nBckTurn, nBckRiver> indexer_t;
	indexer_t::loadLUT();

	abc::EquityCalculator eqt;
	eqt.loadPreflopHSLUT();
	eqt.loadFlopHSLUT();
	eqt.loadTurnHSLUT();
	eqt.loadRivHSLUT();

	// Sort preflop buckets.
	if constexpr (nBckPreflop < abc::PREFLOP_SIZE)
		buildAndSaveSortedBcks(nBckPreflop, indexer_t::dkem.PREFLOP_BCK_LUT, eqt.PREFLOP_HS_LUT, preflopFilePath);
	else {
		std::array<uint8_t, abc::PREFLOP_SIZE> LOSSLESS_PREFLOP_BCK_LUT;
		std::iota(LOSSLESS_PREFLOP_BCK_LUT.begin(), LOSSLESS_PREFLOP_BCK_LUT.end(), uint8_t(0));
		buildAndSaveSortedBcks(nBckPreflop, LOSSLESS_PREFLOP_BCK_LUT, eqt.PREFLOP_HS_LUT, preflopFilePath);
	}
	// Sort flop buckets.
	buildAndSaveSortedBcks(nBckFlop, indexer_t::dkem.FLOP_BCK_LUT, eqt.FLOP_HS_LUT, flopFilePath);
	// Sort turn buckets.
	buildAndSaveSortedBcks(nBckTurn, indexer_t::dkem.TURN_BCK_LUT, eqt.TURN_HS_LUT, turnFilePath);
	// Sort river buckets.
	buildAndSaveSortedBcks(nBckRiver, indexer_t::koc.RIV_BCK_LUT, eqt.RIV_HS_LUT, riverFilePath);
}