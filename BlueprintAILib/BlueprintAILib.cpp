#include "pch.h"
#include "BlueprintAILib.h"

bp::Blueprint* newBlueprint(int rngSeed)
{
	auto blueprint = new bp::Blueprint(
		bp::BLUEPRINT_GAME_NAME,
		bp::BLUEPRINT_BUILD_NAME,
		rngSeed);
	blueprint->loadStrat();
	return blueprint;
}

void delBlueprint(bp::Blueprint* blueprint)
{
	delete blueprint;
}

blueprintAI_t* newBlueprintAI(bp::Blueprint* blueprint, int rngSeed)
{
	return new BLUEPRINT_AI_BUILDER(
		bp::BP_GAME_NAMESPACE,
		bp::BIG_BLIND,
		blueprint,
		rngSeed);
}

void delBlueprintAI(blueprintAI_t* blueprintAI)
{
	delete blueprintAI;
}