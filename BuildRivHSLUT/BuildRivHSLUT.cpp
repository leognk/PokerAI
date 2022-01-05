#include "../LossyAbstraction/EquityCalculator.h"

int main()
{
	abc::EquityCalculator eqt;
	eqt.populateRivHSLUT();
	eqt.saveRivHSLUT();
}