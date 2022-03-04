#ifndef BP_BLUEPRINT_H
#define BP_BLUEPRINT_H

#include "BlueprintCalculator.h"

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

	void loadStrat();
	void loadRegrets();

	uint8_t chooseAction(const abcInfo_t& abcInfo);

	static const strat_t stratMax = 1u << 15;

	strats_t strat;
	regrets_t regrets;

private:
	typedef omp::XoroShiro128Plus Rng;

	strat_t getProba(const abcInfo_t& abcInfo, uint8_t actionId);
	regret_t getRegret(const abcInfo_t& abcInfo, uint8_t actionId);

	void calculateCumProbas(const abcInfo_t& abcInfo);

	const std::string bpGameName;
	const std::string bpName;

	Rng rng;
	opt::FastRandomChoiceRNGRescale<15> actionRandChoice;
	std::vector<strat_t> cumProbas;

}; // Blueprint

} // bp

#endif // BP_BLUEPRINT_H