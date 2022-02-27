#ifndef BP_BLUEPRINT_H
#define BP_BLUEPRINT_H

#include "BlueprintCalculator.h"
#include "../AbstractInfoset/ActionSeqSize.h"

namespace bp {

typedef std::vector<std::vector<std::vector<strat_t>>> strats_t;

class Blueprint
{
public:
	// Set rngSeed to 0 to set a random seed.
	Blueprint(
		const std::string& blueprintGameName,
		const std::string& blueprintBuildName,
		unsigned rngSeed = 0);

	uint8_t chooseAction(const abcInfo_t& abcInfo);

	const strats_t strat;

private:
	typedef omp::XoroShiro128Plus Rng;

	static strats_t loadStrat(const std::string& blueprintGameName, const std::string& blueprintBuildName);

	strat_t getProba(const abcInfo_t& abcInfo, uint8_t actionId);
	void calculateCumProbas(const abcInfo_t& abcInfo);

	Rng rng;
	opt::FastRandomChoiceRNGRescale<15> actionRandChoice;
	std::vector<strat_t> cumProbas;

}; // Blueprint

} // bp

#endif // BP_BLUEPRINT_H