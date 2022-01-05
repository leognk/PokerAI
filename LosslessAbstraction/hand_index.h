/**
 * hand_index.h
 *
 * @author Kevin Waugh (waugh@cs.cmu.edu)
 * @date April 13, 2013
 * 
 * map poker hands to an index shared by all isomorphic hands, and
 * map an index to a canonical poker hand
 */

#ifndef _HAND_INDEX_H_
#define _HAND_INDEX_H_

#include <array>
#include "deck.h"

namespace abc {

static const uint8_t MAX_ROUNDS = 4;
static const uint32_t MAX_GROUP_INDEX = 0x100000;
static const uint8_t MAX_CARDS_PER_ROUND = 3;
static const uint8_t ROUND_SHIFT = 4;
static const uint8_t ROUND_MASK = 0xf;

static const uint32_t PREFLOP_SIZE = 169;
static const uint32_t FLOP_SIZE = 1286792; // 1.3M
static const uint32_t TURN_SIZE = 55190538; // 55M
static const uint32_t RIVER_SIZE = 2428287420; // 2.4G

// Sizes with combined public cards.
static const uint32_t CMB_TURN_SIZE = 13960050; // 14M
static const uint32_t CMB_RIVER_SIZE = 123156254; // 123M

typedef uint32_t hand_index_t;
typedef struct hand_indexer_s hand_indexer_t;
typedef struct hand_indexer_state_s hand_indexer_state_t;

struct hand_indexer_s
{
	/**
	 * Initialize a hand indexer.  This generates a number of lookup tables and is relatively
	 * expensive compared to indexing a hand.
	 *
	 * @param rounds number of rounds
	 * @param cards_per_round number of cards in each round
	 */
	hand_indexer_s(uint_fast32_t rounds, const std::array<uint8_t, MAX_ROUNDS> cards_per_round);

	/**
	 * Free a hand indexer.
	 */
	~hand_indexer_s();

	/**
	 * @param round
	 * @returns size of index for hands on round
	 */
	hand_index_t hand_indexer_size(uint_fast32_t round);

	/**
	 * Initialize a hand index state.  This is used for incrementally indexing a hand as
	 * new rounds are dealt and determining if a hand is canonical.
	 *
	 * @param state
	 */
	void hand_indexer_state_init(hand_indexer_state_t* state);

	/**
	 * Index a hand on every round.  This is not more expensive than just indexing the last round.
	 *
	 * @param cards
	 * @param indices
	 * @returns hand's index on the last round
	 */
	hand_index_t hand_index_all(const uint8_t cards[], hand_index_t indices[]);

	/**
	 * Index a hand on the last round.
	 *
	 * @param cards
	 * @returns hand's index on the last round
	 */
	hand_index_t hand_index_last(const uint8_t cards[]);

	/**
	 * Incrementally index the next round.
	 *
	 * @param cards the cards for the next round only!
	 * @param state
	 * @returns the hand's index at the latest round
	 */
	hand_index_t hand_index_next_round(const uint8_t cards[], hand_indexer_state_t* state);

	/**
	 * Recover the canonical hand from a particular index.
	 *
	 * @param round
	 * @param index
	 * @param cards
	 * @returns true if successful
	 */
	bool hand_unindex(uint_fast32_t round, hand_index_t index, uint8_t cards[]);



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////// IMPLEMENTATION DETAILS ///////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	static void static_init();
	void enumerate_configurations_r(uint_fast32_t round, uint_fast32_t remaining,
		uint_fast32_t suit, uint_fast32_t equal, uint_fast32_t used[], uint_fast32_t configuration[],
		bool tabulate);
	void enumerate_configurations(bool tabulate);
	void count_configurations(uint_fast32_t round);
	void tabulate_configurations(uint_fast32_t round, uint_fast32_t configuration[]);
	void enumerate_permutations_r(uint_fast32_t round, uint_fast32_t remaining,
		uint_fast32_t suit, uint_fast32_t used[], uint_fast32_t count[], bool tabulate);
	void enumerate_permutations(bool tabulate);
	void count_permutations(uint_fast32_t round, uint_fast32_t count[]);
	void tabulate_permutations(uint_fast32_t round, uint_fast32_t count[]);

	static uint8_t nth_unset[1 << RANKS][RANKS];
	static bool equal[1 << (SUITS - 1)][SUITS];
	static uint_fast32_t nCr_ranks[RANKS + 1][RANKS + 1], rank_set_to_index[1 << RANKS], index_to_rank_set[RANKS + 1][1 << RANKS], (*suit_permutations)[SUITS];
	static hand_index_t nCr_groups[MAX_GROUP_INDEX][SUITS + 1];

	std::array<uint8_t, MAX_ROUNDS> cards_per_round;
	uint8_t round_start[MAX_ROUNDS];
	uint_fast32_t rounds, configurations[MAX_ROUNDS], permutations[MAX_ROUNDS];
	hand_index_t round_size[MAX_ROUNDS];

	uint_fast32_t* permutation_to_configuration[MAX_ROUNDS], * permutation_to_pi[MAX_ROUNDS], * configuration_to_equal[MAX_ROUNDS];
	uint_fast32_t(*configuration[MAX_ROUNDS])[SUITS];
	uint_fast32_t(*configuration_to_suit_size[MAX_ROUNDS])[SUITS];
	hand_index_t* configuration_to_offset[MAX_ROUNDS];
};

struct hand_indexer_state_s {
	uint_fast32_t suit_index[SUITS];
	uint_fast32_t suit_multiplier[SUITS];
	uint_fast32_t round, permutation_index, permutation_multiplier;
	uint32_t used_ranks[SUITS];
};

} // abc

#include <intrin.h>

static inline int __builtin_ctz(unsigned x) {
	unsigned long ret;
	_BitScanForward(&ret, x);
	return (int)ret;
}

#define __builtin_popcount __popcnt

#endif /* _HAND_INDEX_H_ */