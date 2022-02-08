#include "pch.h"
#include "../AbstractInfoset/ActionSeqIndexer.h"
#include "../AbstractInfoset/TreeTraverser.h"
#include "../Blueprint/Constants.h"

TEST(ActionSeqIndexerTest, HashIsMinimalPerfect)
{
	abc::ActionSeqIndexer indexer(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, bp::ACTION_SEQ_INDEXER_NAME);
	indexer.loadMPHF();

	abc::TreeTraverser traverser(
		bp::MAX_PLAYERS, bp::ANTE, bp::BIG_BLIND,
		bp::INITIAL_STAKE, bp::BET_SIZES, false);
	// Collect all the action sequences of all rounds.
	std::vector<std::vector<abc::TreeTraverser::seq_t>> actionSeqs = traverser.traverseTree();

	// Test perfect hash function for each round.
	for (uint8_t r = 0; r < egn::N_ROUNDS; ++r) {

		// Verify that the hash function is a bijection from the action sequences
		// to the integers between 0 and n_action_sequences - 1.
		std::vector<bool> flags(actionSeqs[r].size(), false);
		for (const auto& actionSeq : actionSeqs[r]) {
			uint64_t index = indexer.index(egn::Round(r), actionSeq);
			EXPECT_LT(index, flags.size());
			EXPECT_FALSE(flags[index]);
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

	const abc::betSizes_t BET_SIZES = {
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

	abc::TreeTraverser traverser(MAX_PLAYERS, ANTE, BIG_BLIND, INITIAL_STAKE, BET_SIZES, false);
	std::vector<std::vector<abc::TreeTraverser::seq_t>> actionSeqs = traverser.traverseTree();

	EXPECT_EQ(actionSeqs[egn::PREFLOP].size(), 45);
	EXPECT_EQ(actionSeqs[egn::FLOP].size(), 32);
	EXPECT_EQ(actionSeqs[egn::TURN].size(), 32);
	EXPECT_EQ(actionSeqs[egn::RIVER].size(), 32);
}