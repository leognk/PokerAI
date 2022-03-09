#include "../Blueprint/Blueprint.h"

int main()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (unsigned handIdx = 0; handIdx < 169; ++handIdx) {
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const uint8_t round = 0;
	//const unsigned handIdx = 0;

	bp::Blueprint blueprint(bp::BLUEPRINT_GAME_NAME, bp::BLUEPRINT_BUILD_NAME);
	blueprint.loadStrat();
	blueprint.loadRegrets();

	abc::GroupedActionSeqs gpSeqs(bp::BLUEPRINT_GAME_NAME);
	gpSeqs.load();

	size_t currSeq = 0;
	for (const uint8_t nLegalActions : gpSeqs.lens[round]) {
		for (uint8_t a = 0; a < nLegalActions; ++a) {

			const auto seqIdx = gpSeqs.seqs[round][currSeq];
			const auto p = blueprint.strat[round][handIdx][seqIdx];
			const auto r = blueprint.regrets[round][handIdx][seqIdx];

			std::cout
				<< std::setw(4) << currSeq
				<< " | seq: " << std::setw(4) << seqIdx
				<< " | p: " << std::setw(4) << opt::prettyPerc(p, bp::sumStrat)
				<< " | r: " << opt::prettyNumDg((int64_t)r, 3) << "\n";
			++currSeq;
		}
		std::cout << "\n";
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		size_t currSeq = 0;
		uint8_t sum = 0;
		for (uint8_t a = 0; a < nLegalActions; ++a) {
			const auto seqIdx = gpSeqs.seqs[round][currSeq];
			sum += blueprint.strat[round][handIdx][seqIdx];
			++currSeq;
		}
		if (sum != 255) {
			std::cout << "error\n";
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	}
}