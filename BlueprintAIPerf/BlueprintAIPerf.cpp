#include "../Blueprint/EvalBlueprintAI.h"

int main()
{
    const double endDuration = 2;
    const unsigned rngSeed = 2;

    double currDuration = 0;
    uint64_t gameCount = 0;
    double gainAvg = 0;
    double gainStd = 0;
    double minAccGain = 0;

    bp::BlueprintAIEvaluator::evalBlueprintAI(
        currDuration, gameCount,
        gainAvg, gainStd, minAccGain,
        endDuration, rngSeed);

    // Print stats.
    std::cout
        << opt::prettyNumDg(currDuration, 3, true) << "s\n"
        << opt::prettyNumDg(gameCount, 3, true) << "games\n"
        << opt::prettyNumDg(gameCount / currDuration, 3, true) << "games/s\n\n"

        << "Avg gain: " << opt::prettyNumDg(gainAvg, 3, true) << "BB/game\n"
        << "Std gain: " << opt::prettyNumDg(gainStd, 3, true) << "BB/game\n"
        << "Min acc gain: " << opt::prettyNumDg(minAccGain, 3, true) << "BB\n";
}