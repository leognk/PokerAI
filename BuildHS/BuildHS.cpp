#include "../LossyAbstraction/EquityCalculator.h"

int main()
{
	abc::EquityCalculator eqt;

	eqt.populateRivHSLUT();
	eqt.saveRivHSLUT();

	eqt.populateTurnHSLUT();
	eqt.saveTurnHSLUT();

	eqt.populateFlopHSLUT();
	eqt.saveFlopHSLUT();

	eqt.populatePreflopHSLUT();
	eqt.savePreflopHSLUT();

	eqt.populateTurnHSHists();
	eqt.saveTurnHSHists();

	eqt.populateFlopHSHists();
	eqt.saveFlopHSHists();

	eqt.populatePreflopHSHists();
	eqt.savePreflopHSHists();
}