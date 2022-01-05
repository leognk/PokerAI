#include <iostream>
#include <iomanip>
#include <cassert>
#include "../LosslessAbstraction/hand_index.h"

using namespace abc;

static uint8_t nth_bit_[1 << 16][16];
static void nth_bit_ctor() {
    for (uint_fast32_t i = 0; i < 1 << 16; ++i) {
        for (uint_fast32_t j = 0, set = i; set; ++j, set &= set - 1) {
            nth_bit_[i][j] = __builtin_ctz(set);
        }
    }
}

static uint_fast32_t nth_bit(uint64_t used, uint8_t bit) {
    for (uint_fast32_t i = 0; i < 4; ++i) {
        uint_fast32_t pop = __builtin_popcount(used & 0xffff);
        if (pop > bit) {
            return 16 * i + nth_bit_[used & 0xffff][bit];
        }
        else {
            used >>= 16;
            bit -= pop;
        }
    }
    return UINT_FAST32_MAX;
}

void test_full(hand_indexer_t& indexer) {
  uint_fast32_t total_cards = 0; for(uint_fast32_t i=0; i<indexer.rounds; ++i) {
    total_cards += indexer.cards_per_round[i];
  }
  uint_fast32_t permutations = 1; for(uint_fast32_t i=0; i<total_cards; ++i) {
    permutations *= CARDS-i;
  }
  
#pragma warning(suppress: 6385)
  hand_index_t size  = indexer.round_size[indexer.rounds-1];
  uint_fast32_t * seen = (uint_fast32_t*)calloc(size, sizeof(uint_fast32_t)); assert(seen);
  
  uint8_t cards[7]; assert(total_cards <= 7);
  for(uint64_t i=0; i<permutations; ++i) {
    uint64_t p = i, used = 0;
    for(uint_fast32_t j=0; j<total_cards; ++j) {
#pragma warning(suppress: 4244)
      cards[j] = nth_bit(~used, p%(CARDS-j)); p /= CARDS-j;
      used    |= 1ull<<cards[j];
    } 

    hand_index_t index = indexer.hand_index_last(cards);
    assert(index < size);
#pragma warning(suppress: 6011)
    ++seen[index];
  }

  free(seen);

  for(uint64_t i=0; i<size; ++i) {
#pragma warning (suppress: 4244)
    indexer.hand_unindex(indexer.rounds-1, i, cards);
    assert(indexer.hand_index_last(cards) == i);
  }
}

void test_random(hand_indexer_t& indexer) {
  uint_fast32_t total_cards = 0; for(uint_fast32_t i=0; i<indexer.rounds; ++i) {
    total_cards += indexer.cards_per_round[i];
  }
  
#pragma warning(suppress: 6385)
  hand_index_t size  = indexer.round_size[indexer.rounds-1];
  
  uint8_t deck[CARDS], pi[SUITS];
  for(uint_fast32_t i=0; i<CARDS; ++i) {
    deck[i] = i;
  }
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    pi[i] = i;
  }

  uint8_t cards[7]; assert(total_cards <= 7);
  for(uint64_t i=0; i<10000000; ++i) {
    for(uint_fast32_t i=0; i<total_cards; ++i) {
      uint_fast32_t j = rand()%(total_cards-i);
      uint8_t t = deck[i]; deck[i] = deck[i+j]; deck[i+j] = t;
    }
    for(uint_fast32_t i=0; i<SUITS; ++i) {
      uint_fast32_t j = rand()%(SUITS-i);
      uint8_t t = pi[i]; pi[i] = pi[i+j]; pi[i+j] = t;
    }
    memcpy(cards, deck, total_cards);
    for(uint_fast32_t i=0; i<total_cards; ++i) {
      cards[i] = deck_make_card(pi[deck_get_suit(cards[i])], deck_get_rank(cards[i]));
    }
    for(uint_fast32_t i=0, j=0; i<total_cards; ++j) {
      for(uint_fast32_t k=0; k<indexer.cards_per_round[j]; ++k, ++i) {
        uint_fast32_t ii = rand()%(indexer.cards_per_round[j]-k);
#pragma warning(suppress: 6385)
        uint8_t t = cards[i]; cards[i] = cards[i+ii]; cards[i+ii] = t;
      }
    }

    hand_index_t index  = indexer.hand_index_last(deck);
    hand_index_t index2 = indexer.hand_index_last(cards);
    assert(index < size);
    assert(index == index2);

    indexer.hand_unindex(indexer.rounds-1, index, cards);
    assert(indexer.hand_index_last(cards) == index);
  }
}

int main(int argc, char ** argv) {
  nth_bit_ctor();

  std::cout << "testing hand-isomorphism...\n";  

  hand_indexer_t preflop_indexer(1, { 2 });
  hand_indexer_t flop_indexer(2, { 2, 3 });
  hand_indexer_t turn_indexer(3, { 2, 3, 1 });
  hand_indexer_t river_indexer(4, { 2, 3, 1, 1 });

  std::cout << "sizes:"
      << " " << river_indexer.round_size[0]
      << " " << river_indexer.round_size[1]
      << " " << river_indexer.round_size[2]
      << " " << river_indexer.round_size[3] << "\n";

  std::cout << "configurations:"
      << " " << river_indexer.configurations[0]
      << " " << river_indexer.configurations[1]
      << " " << river_indexer.configurations[2]
      << " " << river_indexer.configurations[3] << "\n";

  std::cout << "permutations:"
      << " " << river_indexer.permutations[0]
      << " " << river_indexer.permutations[1]
      << " " << river_indexer.permutations[2]
      << " " << river_indexer.permutations[3] << "\n";

  assert(preflop_indexer.hand_indexer_size(0) == 169);
  assert(flop_indexer.hand_indexer_size(0) == 169);
  assert(turn_indexer.hand_indexer_size(0) == 169);
  assert(river_indexer.hand_indexer_size(0) == 169);
  assert(flop_indexer.hand_indexer_size(1) == 1286792);
  assert(turn_indexer.hand_indexer_size(1) == 1286792);
  assert(river_indexer.hand_indexer_size(1) == 1286792);
  assert(turn_indexer.hand_indexer_size(2) == 55190538);
  assert(river_indexer.hand_indexer_size(2) == 55190538);
  assert(river_indexer.hand_indexer_size(3) == 2428287420);

  uint8_t cards[7];
  std::cout << "preflop table:\n";
  std::cout << " ";
  for(uint_fast32_t i=0; i<RANKS; ++i) {
    std::cout << "  " << RANK_TO_CHAR[RANKS-1-i] << " ";
  }
  std::cout << "\n";
  for(uint_fast32_t i=0; i<RANKS; ++i) {
    std::cout << RANK_TO_CHAR[RANKS-1-i];
    for(uint_fast32_t j=0; j<RANKS; ++j) {
      cards[0] = deck_make_card(0, RANKS-1-j);
      cards[1] = deck_make_card(j<=i, RANKS-1-i);

      hand_index_t index = preflop_indexer.hand_index_last(cards);
      std::cout << " " << std::setw(3) << index;
    }
    printf("\n");
  }

  std::cout << "full preflop...\n";
  test_full(preflop_indexer);

  std::cout << "full flop...\n";
  test_full(flop_indexer);

  std::cout << "random turn...\n";
  test_random(turn_indexer);

  std::cout << "random river...\n";
  test_random(river_indexer);
}