#include "../LossyAbstraction/OCHSCalculator.h"

int main()
{
	abc::OCHSCalculator ochs;

	ochs.populateRivOCHSLUT();
	ochs.saveRivOCHSLUT();
}