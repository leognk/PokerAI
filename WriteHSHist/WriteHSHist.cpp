#include "../Utils/ioArray.h"
#include "../LossyAbstraction/EquityCalculator.h"
#include "../GameEngine/Hand.h"

int main()
{
	// Build hsHist.
	abc::EquityCalculator eqt;
	eqt.loadRivHSLUT();
	auto hand = egn::Hand::stringToArray<5>("5c 9d 3d 5d 7d");
	auto hsHist = eqt.buildFlopHSHist(hand.data());

	// Write hsHist into a file.
	opt::saveArray(hsHist,
		"../data/AbstractionSaves/HSHistExamples/Flop equity distribution - 5c9d-3d5d7d.bin");
}