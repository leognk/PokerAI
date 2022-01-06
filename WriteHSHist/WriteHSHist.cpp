#include <fstream>
#include "../LossyAbstraction/EquityCalculator.h"
#include "../GameEngine/Hand.h"

int main()
{
	// Build hsHist.
	abc::EquityCalculator eqt;
	eqt.loadRivHSLUT();
	auto hand = egn::Hand::stringToArray<2>("Qs Ks");
	auto hsHist = eqt.buildPreflopHSHist<50>(hand.data());

	// Write hsHist into a file.
	auto file = std::fstream("../data/AbstractionSaves/HSHistExamples/Preflop equity distribution - QsKs.bin",
		std::ios::out | std::ios::binary);
	file.write((char*)&hsHist[0], hsHist.size() * sizeof(uint32_t));
	file.close();
}