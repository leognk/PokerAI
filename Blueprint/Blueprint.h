#ifndef BP_BLUEPRINT_H
#define BP_BLUEPRINT_H

#include "BlueprintCalculator.h"
#include "../AbstractInfoset/ActionSeqSize.h"

namespace bp {

typedef std::vector<std::vector<std::vector<strat_t>>> strats_t;

class Blueprint
{
public:
	static strats_t loadStrat();

	static const strats_t strat;

private:
	static const abc::ActionSeqSize seqSizes;

}; // Blueprint

} // bp

#endif // BP_BLUEPRINT_H