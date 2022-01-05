#include <fstream>
#include "../LossyAbstraction/EquityCalculator.h"
#include "../GameEngine/Hand.h"

int main()
{
	// Build hsHist.
	abc::EquityCalculator eqt;
	eqt.loadRivHSLUT();
	auto hand = egn::Hand::stringToArray<6>("As Ac 3h Kh Ks Qc");
	auto hsHist = eqt.buildTurnHSHist<50>(hand.data());

	// Write hsHist into a file.
	auto file = std::fstream("../data/AbstractionSaves/HSHistExamples/Turn equity distribution - AsAc-3hKhKsQc.bin",
		std::ios::out | std::ios::binary);
	file.write((char*)&hsHist[0], hsHist.size() * sizeof(uint8_t));
	file.close();
}