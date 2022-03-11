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

	template<class Info>
	strat_t getProba(const Info& abcInfo, uint8_t actionId) const
	{
		return strat[abcInfo.roundIdx()][abcInfo.handIdx()][abcInfo.actionSeqIds[actionId]];
	}

	template<class Info>
	regret_t getRegret(const Info& abcInfo, uint8_t actionId) const
	{
		return regrets[abcInfo.roundIdx()][abcInfo.handIdx()][abcInfo.actionSeqIds[actionId]];
	}

	template<class Info>
	std::vector<uint8_t> calculateProbasPerc(const Info& abcInfo) const
	{
		// We apply the following weird procedure so that the sum of
		// the probas will be exactly equal to 100, getting around
		// rounding issues.
		std::vector<uint8_t> probas(abcInfo.nActions());
		probas[0] = getProba(abcInfo, 0);
		for (uint8_t a = 1; a < probas.size(); ++a)
			probas[a] = probas[a - 1] + getProba(abcInfo, a);
		for (uint8_t a = 0; a < probas.size(); ++a)
			probas[a] = (uint8_t)std::round(100.0 * probas[a] / probas.back());
		for (uint8_t a = (uint8_t)probas.size() - 1; a > 0; --a)
			probas[a] -= probas[a - 1];
		return probas;
	}

	template<class Info>
	uint8_t chooseAction(const Info& abcInfo)
	{
		calculateCumProbas(abcInfo);
		return (uint8_t)actionRandChoice(cumProbas, rng);
	}

	strats_t strat;
	regrets_t regrets;

private:
	typedef omp::XoroShiro128Plus Rng;

	template<class Info>
	void calculateCumProbas(const Info& abcInfo)
	{
		cumProbas.resize(abcInfo.nActions());
		cumProbas[0] = getProba(abcInfo, 0);
		for (uint8_t a = 1; a < cumProbas.size(); ++a)
			cumProbas[a] = cumProbas[a - 1] + getProba(abcInfo, a);
	}

	const std::string bpGameName;
	const std::string bpName;

	Rng rng;
	opt::FastRandomChoiceRNGRescale<16> actionRandChoice;
	std::vector<strat_t> cumProbas;

}; // Blueprint

} // bp

#endif // BP_BLUEPRINT_H