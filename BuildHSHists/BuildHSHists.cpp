#include "../LossyAbstraction/EquityCalculator.h"

int main()
{
	abc::EquityCalculator eqt;

	eqt.populateTurnHSHists();
	eqt.saveTurnHSHists();

	eqt.populateFlopHSHists();
	eqt.saveFlopHSHists();

	eqt.populatePreflopHSHists();
	eqt.savePreflopHSHists();
}