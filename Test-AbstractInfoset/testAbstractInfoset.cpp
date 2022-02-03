#include "pch.h"
#include "../AbstractInfoset/ActionSeqIndexer.h"
#include "../Blueprint/Constants.h"

TEST(ActionSeqIndexerTest, HashIsMinimalPerfect)
{
	abc::ActionSeqIndexer indexer(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, bp::ACTION_SEQ_INDEXER_NAME);
	indexer.loadPHF();

	abc::TreeTraverser traverser(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, true, false);

	// Test perfect hash function for each round.
	for (uint8_t i = 0; i < egn::N_ROUNDS; ++i) {

		// Collect all the action sequences of the round.
		std::vector<std::vector<uint8_t>> actionSeqs;
		traverser.traverseRoundTree(egn::Round(i), actionSeqs);

		// Verify that the hash function is a bijection from the action sequences
		// to the integers between 0 and n_action_sequences - 1.
		std::vector<bool> flags(actionSeqs.size(), false);
		for (const std::vector<uint8_t>& actionSeq : actionSeqs) {
			uint64_t index = indexer.index(egn::Round(i), actionSeq);
			EXPECT_LT(index, flags.size());
			EXPECT_FALSE(flags[index]);
			// For debug purposes, if the index has already been generated,
			// verify that actionSeq is unique as it should be.
			if (flags[index]) {
				int64_t count = std::count(actionSeqs.begin(), actionSeqs.end(), actionSeq);
				EXPECT_EQ(count, 1);
			}
			flags[index] = true;
		}
		// Not necessary in theory but just in case.
		for (bool flag : flags) EXPECT_TRUE(flag);
	}
}

TEST(ActionAbstractionTest, TreeNodesCountIsCorrect)
{
	const uint8_t MAX_PLAYERS = 3;

	const egn::chips ANTE = 0;
	const egn::chips BIG_BLIND = 2;
	const egn::chips INITIAL_STAKE = 6;

	const std::vector<std::vector<std::vector<float>>> BET_SIZES = {
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		},
		{
			{ 1, 2 },
			{ 1 }
		}
	};

	abc::TreeTraverser traverser(
		MAX_PLAYERS, ANTE, BIG_BLIND, INITIAL_STAKE, BET_SIZES, false, false);

	uint64_t count;

	count = traverser.traverseRoundTree(egn::PREFLOP);
	EXPECT_EQ(count, 45);

	count = traverser.traverseRoundTree(egn::FLOP);
	EXPECT_EQ(count, 32);

	count = traverser.traverseRoundTree(egn::TURN);
	EXPECT_EQ(count, 32);

	count = traverser.traverseRoundTree(egn::RIVER);
	EXPECT_EQ(count, 32);
}