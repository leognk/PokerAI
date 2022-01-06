#ifndef ABC_DKEM_H
#define ABC_DKEM_H

#include "../LosslessAbstraction/hand_index.h"

namespace abc {

typedef uint8_t idx_t;

// Class generating lossy information abstraction
// with distribution-aware k-means earth mover's distance.
template<typename bck_t = uint8_t>
class DKEM
{
public:
	// BCK for BUCKET
	static void populateFlopBckLUT(bck_t nBck)
	{

	}

	static std::array<bck_t, FLOP_SIZE> FLOP_BCK_LUT;
	static std::array<bck_t, CMB_TURN_SIZE> TURN_BCK_LUT;
	static std::array<bck_t, CMB_RIVER_SIZE> RIV_BCK_LUT;

private:


}; // DKEM

} // abc

#endif // ABC_DKEM_H