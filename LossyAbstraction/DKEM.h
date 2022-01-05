#ifndef ABC_DKEM_H
#define ABC_DKEM_H

#include <cstdint>
#include <array>
#include "../GameEngine/GameState.h"

namespace abc {

typedef uint8_t idx_t;

// Class defining lossy information abstraction
// with distribution-aware k-means earth mover's distance.
class DKEM
{
public:
	DKEM(const std::array<uint8_t, egn::N_ROUNDS>& cardsPerRound);
	idx_t handIndex(const std::array<uint8_t, 7>& cards);

private:


}; // DKEM

} // abc

#endif // ABC_DKEM_H