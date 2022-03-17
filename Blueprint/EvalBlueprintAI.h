#include "../Blueprint/BlueprintAI.h"
#include "../RandomAI/RandomAI.h"

namespace bp {

class BlueprintAIEvaluator
{
public:

    // Simulate random games with 1 blueprintAI vs randomAIs (randomly built)
    // and calculate the blueprintAI's average gain, std and minimum of the
    // accumulated gain in BB in-place.
    static void evalBlueprintAI(
        double& currDuration, uint64_t& gameCount,
        double& gainAvg, double& gainStd, double& minAccGain,
        const double endDuration, const unsigned rngSeed = 0);

    static void evalBlueprintAI(
        double& gainAvg, double& gainStd, double& minAccGain,
        const double endDuration, const unsigned rngSeed = 0);
};

} // bp