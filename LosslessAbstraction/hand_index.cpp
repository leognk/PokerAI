#include <cassert>
#include "hand_index.h"

#define MAX_GROUP_INDEX        0x100000 
#define MAX_CARDS_PER_ROUND    15
#define ROUND_SHIFT            4
#define ROUND_MASK             0xf

uint8_t hand_indexer_s::nth_unset[1 << RANKS][RANKS];
bool hand_indexer_s::equal[1 << (SUITS - 1)][SUITS];
uint_fast32_t hand_indexer_s::nCr_ranks[RANKS + 1][RANKS + 1];
uint_fast32_t hand_indexer_s::rank_set_to_index[1 << RANKS];
uint_fast32_t hand_indexer_s::index_to_rank_set[RANKS + 1][1 << RANKS];
uint_fast32_t (*hand_indexer_s::suit_permutations)[SUITS];
hand_index_t hand_indexer_s::nCr_groups[MAX_GROUP_INDEX][SUITS + 1];

void hand_indexer_s::static_init() {
  for(uint_fast32_t i=0; i<1<<(SUITS-1); ++i) {
    for(uint_fast32_t j=1; j<SUITS; ++j) {
      equal[i][j] = i&1<<(j-1);
    }
  }

  for(uint_fast32_t i=0; i<1<<RANKS; ++i) {
    for(uint_fast32_t j=0, set=~i&(1<<RANKS)-1; j<RANKS; ++j, set&=set-1) {
      nth_unset[i][j] = set?__builtin_ctz(set):0xff;
    }
  }

  nCr_ranks[0][0]     = 1;
  for(uint_fast32_t i=1; i<RANKS+1; ++i) {
    nCr_ranks[i][0]   = nCr_ranks[i][i] = 1;
    for(uint_fast32_t j=1; j<i; ++j) {
      nCr_ranks[i][j] = nCr_ranks[i-1][j-1] + nCr_ranks[i-1][j];
    }
  }

  nCr_groups[0][0] = 1;
  for(uint_fast32_t i=1; i<MAX_GROUP_INDEX; ++i) {
    nCr_groups[i][0] = 1;
    if (i < SUITS+1) {
      nCr_groups[i][i] = 1;
    }
    for(uint_fast32_t j=1; j<(i<(SUITS+1)?i:(SUITS+1)); ++j) {
      nCr_groups[i][j] = nCr_groups[i-1][j-1] + nCr_groups[i-1][j];
    } 
  }

  for(uint_fast32_t i=0; i<1<<RANKS; ++i) {
    for(uint_fast32_t set=i, j=1; set; ++j, set&=set-1) {
      rank_set_to_index[i]  += nCr_ranks[__builtin_ctz(set)][j];
    }
    index_to_rank_set[__builtin_popcount(i)][rank_set_to_index[i]] = i;
  }

  uint_fast32_t num_permutations = 1;
  for(uint_fast32_t i=2; i<=SUITS; ++i) {
    num_permutations *= i;
  }

  suit_permutations = (uint_fast32_t(*)[SUITS])calloc(num_permutations, SUITS*sizeof(uint_fast32_t));
  
  for(uint_fast32_t i=0; i<num_permutations; ++i) {
    for(uint_fast32_t j=0, index=i, used=0; j<SUITS; ++j) {
      uint_fast32_t suit = index%(SUITS-j); index /= SUITS-j;
      uint_fast32_t shifted_suit = nth_unset[used][suit];
      suit_permutations[i][j] = shifted_suit;
      used                   |= 1<<shifted_suit;
    }
  }
}

void hand_indexer_s::enumerate_configurations_r(uint_fast32_t round, uint_fast32_t remaining, 
    uint_fast32_t suit, uint_fast32_t equal, uint_fast32_t used[], uint_fast32_t configuration[],
    bool tabulate) {
  if (suit == SUITS) {
    if (tabulate)
      tabulate_configurations(round, configuration);
    else
      count_configurations(round);

    if (round+1 < rounds) {
#pragma warning(suppress: 26451)
      enumerate_configurations_r(round+1, cards_per_round[round+1], 0, equal, used, configuration, tabulate);
    }
  } else {
    uint_fast32_t min = 0;
    if (suit == SUITS-1) {
      min = remaining;
    }
    
    uint_fast32_t max = RANKS-used[suit];
    if (remaining < max) {
      max = remaining;
    }
   
    uint_fast32_t previous = RANKS+1;
    bool was_equal = equal&1<<suit;
    if (was_equal) {
      previous = configuration[suit-1]>>ROUND_SHIFT*(rounds-round-1)&ROUND_MASK;
      if (previous < max) {
        max = previous;
      }
    }
    
    uint_fast32_t old_configuration = configuration[suit], old_used = used[suit];
    for(uint_fast32_t i=min; i<=max; ++i) {
      uint_fast32_t new_configuration = old_configuration | i<<ROUND_SHIFT*(rounds-round-1);
      uint_fast32_t new_equal = (equal&~(1<<suit))|(was_equal&(i==previous))<<suit;

      used[suit] = old_used+i;
      configuration[suit] = new_configuration;
      enumerate_configurations_r(round, remaining-i, suit+1, new_equal, used, configuration, tabulate);
      configuration[suit] = old_configuration;
      used[suit] = old_used;
    }
  }
}

void hand_indexer_s::enumerate_configurations(bool tabulate) {
  uint_fast32_t used[SUITS] = {0}, configuration[SUITS] = {0};
  enumerate_configurations_r(0, cards_per_round[0], 0, (1<<SUITS) - 2, used, configuration, tabulate);
}

void hand_indexer_s::count_configurations(uint_fast32_t round) {
  ++configurations[round];
}

void hand_indexer_s::tabulate_configurations(uint_fast32_t round, uint_fast32_t configuration0[]) {

  uint_fast32_t id = configurations[round]++;
  for(; id>0; --id) {
    for(uint_fast32_t i=0; i<SUITS; ++i) {
      if (configuration0[i] < configuration[round][id-1][i]) {
        break;
      } else if (configuration0[i] > configuration[round][id-1][i]) {
        goto out;
      }
    }
    for(uint_fast32_t i=0; i<SUITS; ++i) {
      configuration[round][id][i]              = configuration[round][id-1][i];
      configuration_to_suit_size[round][id][i] = configuration_to_suit_size[round][id-1][i];
    }
    configuration_to_offset[round][id] = configuration_to_offset[round][id-1];
    configuration_to_equal[round][id]  = configuration_to_equal[round][id-1];
  }
out:;

  configuration_to_offset[round][id] = 1; 
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    configuration[round][id][i] = configuration0[i];
  }

  uint_fast32_t equal = 0;
  for(uint_fast32_t i=0; i<SUITS;) {
    hand_index_t size = 1;
    for(uint_fast32_t j=0, remaining=RANKS; j<=round; ++j) {
      uint_fast32_t ranks = configuration0[i]>>ROUND_SHIFT*(rounds-j-1)&ROUND_MASK;
#pragma warning(suppress: 6385)
      size *= nCr_ranks[remaining][ranks];
      remaining -= ranks;
    }
    assert(size+SUITS-1 < MAX_GROUP_INDEX);
    
    uint_fast32_t j=i+1; for(; j<SUITS && configuration0[j] == configuration0[i]; ++j) {} 
    for(uint_fast32_t k=i; k<j; ++k) {
#pragma warning(suppress: 4244)
      configuration_to_suit_size[round][id][k] = size;
    }

    configuration_to_offset[round][id] *= nCr_groups[size+j-i-1][j-i];
    
    for(uint_fast32_t k=i+1; k<j; ++k) {
      equal |= 1<<k;
    }

    i = j;
  }
  
  configuration_to_equal[round][id] = equal>>1;
}

void hand_indexer_s::enumerate_permutations_r(uint_fast32_t round, uint_fast32_t remaining, 
    uint_fast32_t suit, uint_fast32_t used[], uint_fast32_t count[], bool tabulate) {
  if (suit == SUITS) {
    if (tabulate)
      tabulate_permutations(round, count);
    else
      count_permutations(round, count);

    if (round+1 < rounds) {
#pragma warning(suppress: 26451)
      enumerate_permutations_r(round+1, cards_per_round[round+1], 0, used, count, tabulate);
    }
  } else {
    uint_fast32_t min = 0;
    if (suit == SUITS-1) {
      min = remaining;
    }
    uint_fast32_t max = RANKS-used[suit];
    if (remaining < max) {
      max = remaining;
    }
    
    uint_fast32_t old_count = count[suit], old_used = used[suit];
    for(uint_fast32_t i=min; i<=max; ++i) {
      uint_fast32_t new_count = old_count | i<<ROUND_SHIFT*(rounds-round-1);

      used[suit] = old_used+i;
      count[suit] = new_count;
      enumerate_permutations_r(round, remaining-i, suit+1, used, count, tabulate);
      count[suit] = old_count;
      used[suit] = old_used;
    }
  }
}

void hand_indexer_s::enumerate_permutations(bool tabulate) {
  uint_fast32_t used[SUITS] = {0}, count[SUITS] = {0};
  enumerate_permutations_r(0, cards_per_round[0], 0, used, count, tabulate);
}

void hand_indexer_s::count_permutations(uint_fast32_t round, uint_fast32_t count[]) {
  uint_fast32_t idx = 0, mult = 1;
  for(uint_fast32_t i=0; i<=round; ++i) {
    for(uint_fast32_t j=0, remaining=cards_per_round[i]; j<SUITS-1; ++j) {
      uint_fast32_t size = count[j]>>(rounds-i-1)*ROUND_SHIFT&ROUND_MASK;
      idx  += mult*size;
      mult *= remaining+1;
      remaining -= size;
    }
  }
  
  if (permutations[round] < idx+1) {
    permutations[round] = idx+1;
  }
}

void hand_indexer_s::tabulate_permutations(uint_fast32_t round, uint_fast32_t count[]) {
  uint_fast32_t idx = 0, mult = 1;
  for(uint_fast32_t i=0; i<=round; ++i) {
    for(uint_fast32_t j=0, remaining=cards_per_round[i]; j<SUITS-1; ++j) {
      uint_fast32_t size = count[j]>>(rounds-i-1)*ROUND_SHIFT&ROUND_MASK;
      idx       += mult*size;
      mult      *= remaining+1;
      remaining -= size;
    }
  }
  
  uint_fast32_t pi[SUITS];
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    pi[i] = i;
  }

  for(uint_fast32_t i=1; i<SUITS; ++i) {
    uint_fast32_t j=i, pi_i = pi[i]; for(; j>0; --j) {
      if (count[pi_i] > count[pi[j-1]]) {
        pi[j] = pi[j-1];
      } else {
        break;
      }
    }
    pi[j] = pi_i;
  }

  uint_fast32_t pi_idx = 0, pi_mult = 1, pi_used = 0;
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    uint_fast32_t this_bit = 1<<pi[i];
    uint_fast32_t smaller  = __builtin_popcount((this_bit-1)&pi_used);
    pi_idx  += (pi[i]-smaller)*pi_mult;
    pi_mult *= SUITS-i;
    pi_used |= this_bit;
  }

  permutation_to_pi[round][idx] = pi_idx;

  uint_fast32_t low = 0, high = configurations[round];
  while(low < high) {
    uint_fast32_t mid = (low+high)/2;

    int_fast32_t compare = 0;
    for(uint_fast32_t i=0; i<SUITS; ++i) {
      uint_fast32_t thisOne  = count[pi[i]];
      uint_fast32_t other = configuration[round][mid][i];
      if (other > thisOne) {
        compare = -1; break;
      } else if (other < thisOne) {
        compare = 1; break;
      }
    }
 
    if (compare == -1) {
      high = mid;
    } else if (compare == 0) {
      low = high = mid;
    } else {
      low = mid+1;
    }
  }

  permutation_to_configuration[round][idx] = low;
}

hand_indexer_s::hand_indexer_s(uint_fast32_t rounds, const std::array<uint8_t, MAX_ROUNDS> cards_per_round) :
    rounds(rounds),
    cards_per_round(cards_per_round)
{
  static bool initVar = (static_init(), true);

  assert(rounds != 0);
  assert(rounds <= MAX_ROUNDS);
  for(uint_fast32_t i=0, count=0; i<rounds; ++i) {
    count += cards_per_round[i];
    assert(count <= CARDS);
  }

  for(uint_fast32_t i=0, j=0; i<rounds; ++i) {
    round_start[i] = j; j += cards_per_round[i];
  }

  memset(configurations, 0, sizeof(configurations));
  enumerate_configurations(false);

  for(uint_fast32_t i=0; i<rounds; ++i) {
    configuration_to_equal[i]     = (uint_fast32_t*)calloc(configurations[i], sizeof(uint_fast32_t));
    configuration_to_offset[i]    = (hand_index_t*)calloc(configurations[i], sizeof(hand_index_t));
    configuration[i]              = (uint_fast32_t(*)[SUITS])calloc(configurations[i], SUITS*sizeof(uint_fast32_t));
    configuration_to_suit_size[i] = (uint_fast32_t(*)[SUITS])calloc(configurations[i], SUITS*sizeof(uint_fast32_t));
    assert(configuration_to_equal[i] &&
        configuration_to_offset[i] &&
        configuration[i] &&
        configuration_to_suit_size[i]);
  }

  memset(configurations, 0, sizeof(configurations));
  enumerate_configurations(true);
  
  for(uint_fast32_t i=0; i<rounds; ++i) {
    hand_index_t accum = 0; for(uint_fast32_t j=0; j<configurations[i]; ++j) {
      hand_index_t next = accum + configuration_to_offset[i][j];
      configuration_to_offset[i][j] = accum;
      accum = next;
    }
    round_size[i] = accum;
  }

  memset(permutations, 0, sizeof(permutations));
  enumerate_permutations(false);
  
  for(uint_fast32_t i=0; i<rounds; ++i) {
    permutation_to_configuration[i] = (uint_fast32_t*)calloc(permutations[i], sizeof(uint_fast32_t));
    permutation_to_pi[i] = (uint_fast32_t*)calloc(permutations[i], sizeof(uint_fast32_t));
    assert(permutation_to_configuration &&
        permutation_to_pi);
  }

  enumerate_permutations(true);
}

hand_indexer_s::~hand_indexer_s() {
  for(uint_fast32_t i=0; i<rounds; ++i) {
    free(permutation_to_configuration[i]);
    free(permutation_to_pi[i]);
    free(configuration_to_equal[i]);
    free(configuration_to_offset[i]);
    free(configuration[i]);
    free(configuration_to_suit_size[i]);
  }
}

hand_index_t hand_indexer_s::hand_indexer_size(uint_fast32_t round) {
  assert(round < rounds);
  return round_size[round];
}

void hand_indexer_s::hand_indexer_state_init(hand_indexer_state_t * state) {
  memset(state, 0, sizeof(hand_indexer_state_t));
 
  state->permutation_multiplier = 1;
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    state->suit_multiplier[i] = 1;
  }
}

hand_index_t hand_indexer_s::hand_index_all(const uint8_t cards[], hand_index_t indices[]) {
  if (rounds) {
    hand_indexer_state_t state; hand_indexer_state_init(&state);

    for(uint_fast32_t i=0, j=0; i<rounds; j+=cards_per_round[i++]) {
      indices[i] = hand_index_next_round(cards+j, &state);
    }

    return indices[rounds-1];
  }

  return 0;
}

hand_index_t hand_indexer_s::hand_index_last(const uint8_t cards[]) {
  hand_index_t indices[MAX_ROUNDS];
  return hand_index_all(cards, indices);
}

hand_index_t hand_indexer_s::hand_index_next_round(const uint8_t cards[], hand_indexer_state_t * state) {
  uint_fast32_t round = state->round++;
  assert(round < rounds);

  uint_fast32_t ranks[SUITS] = {0}, shifted_ranks[SUITS] = {0};
  for(uint_fast32_t i=0; i<cards_per_round[round]; ++i) {
    assert(cards[i] < CARDS);                 /* valid card */

    uint_fast32_t rank         = deck_get_rank(cards[i]), suit = deck_get_suit(cards[i]), rank_bit = 1<<rank;
    assert(!(ranks[suit]&rank_bit));
    ranks[suit]               |= rank_bit;
    shifted_ranks[suit]       |= rank_bit>>__builtin_popcount((rank_bit-1)&state->used_ranks[suit]);
  }

  for(uint_fast32_t i=0; i<SUITS; ++i) {
    assert(!(state->used_ranks[i]&ranks[i])); /* no duplicate cards */

    uint_fast32_t used_size    = __builtin_popcount(state->used_ranks[i]), this_size = __builtin_popcount(ranks[i]);
    state->suit_index[i]      += state->suit_multiplier[i]*rank_set_to_index[shifted_ranks[i]];
    state->suit_multiplier[i] *= nCr_ranks[RANKS-used_size][this_size];
    state->used_ranks[i]      |= ranks[i];
  }

  for(uint_fast32_t i=0, remaining=cards_per_round[round]; i<SUITS-1; ++i) {
    uint_fast32_t this_size          = __builtin_popcount(ranks[i]);
    state->permutation_index        += state->permutation_multiplier*this_size;
    state->permutation_multiplier   *= remaining+1;
    remaining                       -= this_size;
  }

  uint_fast32_t configuration = permutation_to_configuration[round][state->permutation_index];
  uint_fast32_t pi_index      = permutation_to_pi[round][state->permutation_index];
  uint_fast32_t equal_index   = configuration_to_equal[round][configuration];
  hand_index_t offset         = configuration_to_offset[round][configuration];
  const uint_fast32_t * pi    = suit_permutations[pi_index];

  hand_index_t suit_index[SUITS], suit_multiplier[SUITS];
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    suit_index[i]      = state->suit_index[pi[i]]; 
    suit_multiplier[i] = state->suit_multiplier[pi[i]];
  }
  
  /* sort using an optimal sorting network */
#define swap(u, v) \
  do {\
    if (suit_index[u] > suit_index[v]) {\
      suit_index[u] ^= suit_index[v];\
      suit_index[v] ^= suit_index[u];\
      suit_index[u] ^= suit_index[v];\
    }\
  } while(0)

  hand_index_t index = offset, multiplier = 1;
  for(uint_fast32_t i=0; i<SUITS;) {
    hand_index_t part, size;

    if (i+1 < SUITS && equal[equal_index][i+1]) {
      if (i+2 < SUITS && equal[equal_index][i+2]) {
        if (i+3 < SUITS && equal[equal_index][i+3]) {
          /* four equal suits */
          swap(i, i+1); swap(i+2, i+3); swap(i, i+2); swap(i+1, i+3); swap(i+1, i+2);
          part = suit_index[i] + nCr_groups[suit_index[i+1]+1][2] + nCr_groups[suit_index[i+2]+2][3] + nCr_groups[suit_index[i+3]+3][4];
          size = nCr_groups[suit_multiplier[i]+3][4];
          i += 4;
        } else {
          /* three equal suits */
          swap(i, i+1); swap(i, i+2); swap(i+1, i+2);
          part = suit_index[i] + nCr_groups[suit_index[i+1]+1][2] + nCr_groups[suit_index[i+2]+2][3];
          size = nCr_groups[suit_multiplier[i]+2][3];
          i += 3;
        }
      } else {
        /* two equal suits*/
        swap(i, i+1);
        part = suit_index[i] + nCr_groups[suit_index[i+1]+1][2];
        size = nCr_groups[suit_multiplier[i]+1][2];
        i += 2;
      }
    } else {
      /* no equal suits */
      part = suit_index[i];
      size = suit_multiplier[i];
      i += 1;
    }

    index      += multiplier*part;
    multiplier *= size;
  }

#undef swap

  return index;
}

bool hand_indexer_s::hand_unindex(uint_fast32_t round, hand_index_t index, uint8_t cards[]) {
  if (round >= rounds || index >= round_size[round]) {
    return false;
  }

  uint_fast32_t low = 0, high = configurations[round], configuration_idx = 0;
  while(low < high) {
    uint_fast32_t mid = (low+high)/2;
    if (configuration_to_offset[round][mid] <= index) {
      configuration_idx = mid;
      low = mid+1;
    } else {
      high = mid;
    }
  }
  index -= configuration_to_offset[round][configuration_idx];

  hand_index_t suit_index[SUITS];
  for(uint_fast32_t i=0; i<SUITS;) {
    uint_fast32_t j=i+1; for(; j<SUITS && configuration[round][configuration_idx][j] == configuration[round][configuration_idx][i]; ++j) {}
    
    uint_fast32_t suit_size  = configuration_to_suit_size[round][configuration_idx][i];
    hand_index_t group_size  = nCr_groups[suit_size+j-i-1][j-i];
    hand_index_t group_index = index%group_size; index /= group_size;

    for(; i<j-1; ++i) {
#pragma warning(suppress: 4244)
      suit_index[i] = low = floor(exp(log(group_index)/(j-i) - 1 + log(j-i))-j-i); high = ceil(exp(log(group_index)/(j-i) + log(j-i))-j+i+1);
      if (high > suit_size) {
        high = suit_size;
      }
      if (high <= low) {
        low = 0;
      }
      while(low < high) {
        uint_fast32_t mid = (low+high)/2;
        if (nCr_groups[mid+j-i-1][j-i] <= group_index) {
          suit_index[i] = mid;
          low = mid+1;
        } else {
          high = mid;
        }
      }

      //for(suit_index[i]=0; nCr_groups[suit_index[i]+1+j-i-1][j-i] <= group_index; ++suit_index[i]) {}
      group_index -= nCr_groups[suit_index[i]+j-i-1][j-i]; 
    }

    suit_index[i] = group_index; ++i;
  }
  
  uint8_t location[MAX_ROUNDS]; memcpy(location, round_start, MAX_ROUNDS);
  for(uint_fast32_t i=0; i<SUITS; ++i) {
    uint_fast32_t used = 0, m = 0;
    for(uint_fast32_t j=0; j<rounds; ++j) {
      uint_fast32_t n              = configuration[round][configuration_idx][i]>>ROUND_SHIFT*(rounds-j-1)&ROUND_MASK;
#pragma warning(suppress: 6385)
      uint_fast32_t round_size     = nCr_ranks[RANKS-m][n]; m += n;
      uint_fast32_t round_idx      = suit_index[i]%round_size; suit_index[i] /= round_size;
#pragma warning(suppress: 6385)
      uint_fast32_t shifted_cards  = index_to_rank_set[n][round_idx], rank_set = 0;
      for(uint_fast32_t k=0; k<n; ++k) {
#pragma warning(suppress: 4146)
        uint_fast32_t shifted_card = shifted_cards&-shifted_cards; shifted_cards ^= shifted_card;
        uint_fast32_t card         = nth_unset[used][__builtin_ctz(shifted_card)]; rank_set |= 1<<card;
        cards[location[j]++]       = deck_make_card(i, card);
      }
      used |= rank_set;
    }
  }

  return true;
}