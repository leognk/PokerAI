#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <random>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <bitset>

#include "../BlueprintAILib/BlueprintAILib.h"

int main()
{
	auto advisor = newBlueprintAIAdvisor(0);
	egn::chips stakes[6] = { 10000, 10000, 10000, 10000, 10000, 10000 };
	const char* hand = "As Tc";
	adv_startNewHand(advisor, 0, 100, stakes, 0, 3, hand);
	adv_getAdvices(advisor);
	adv_update(advisor, RAISE(), 200);
	adv_update(advisor, CALL(), 0);
	adv_update(advisor, CALL(), 0);

	std::cout << adv_aiAction(advisor) << " " << adv_aiBet(advisor) << "\n";
	std::cout << std::to_string(adv_nActions(advisor)) << "\n";
	std::cout << "[";
	for (size_t i = 0; i < adv_nActions(advisor); ++i) {
		if (i != 0) std::cout << ", ";
		std::cout << std::to_string(adv_probas(advisor)[i]);
	}
	std::cout << "]\n";
	std::cout << "[";
	for (size_t i = 0; i < adv_nActions(advisor); ++i) {
		if (i != 0) std::cout << ", ";
		std::cout << std::to_string(adv_actions(advisor)[i]);
	}
	std::cout << "]\n";
	std::cout << "[";
	for (size_t i = 0; i < adv_nActions(advisor); ++i) {
		if (i != 0) std::cout << ", ";
		std::cout << adv_bets(advisor)[i];
	}
	std::cout << "]\n";

	delBlueprintAIAdvisor(advisor);
}