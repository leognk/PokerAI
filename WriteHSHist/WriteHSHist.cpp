#include "../Utils/ioArray.h"
#include "../LossyAbstraction/EquityCalculator.h"
#include "../GameEngine/Hand.h"

int main()
{
	// Build hsHist.
	abc::EquityCalculator eqt;
	eqt.loadRivHSLUT();
	auto hand = egn::Hand::stringToArray<2>("Qs Ks");
	auto hsHist = eqt.buildPreflopHSHist(hand.data());

	// Write hsHist into a file.
	saveArray(hsHist,
		"../data/AbstractionSaves/HSHistExamples/Preflop equity distribution - QsKs.bin");
}