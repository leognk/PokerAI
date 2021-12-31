#ifndef OPT_RANDOM_H
#define OPT_RANDOM_H

#include <cstdint> 

namespace opt {

// Generate a random index with the given
// array of cumulated weights (maximum weight is RANGE).
// The precision is 1 / 2^tBits.
// tBits must be less than 32 (excluded).
template<unsigned tBits = 16>
class FastRandomChoice
{
public:
    FastRandomChoice()
    {
        mBuffer = 0;
        mBufferUsesLeft = 0;
    }

    template<class C, class TRng>
    unsigned operator()(C& cumWeights, TRng& rng)
    {
        unsigned x = rand(rng);
        unsigned i = 0;
        while (cumWeights[i] <= x)
            ++i;
        return i;
    }

    static const unsigned RANGE = 1u << tBits;

private:
    // Generate a random unsigned int between 0 and RANGE excluded.
    template<class TRng>
    unsigned rand(TRng& rng)
    {
        static_assert(sizeof(typename TRng::result_type) == sizeof(uint64_t), "64-bit RNG required.");
        if (mBufferUsesLeft == 0) {
            mBuffer = rng();
            mBufferUsesLeft = sizeof(mBuffer) * CHAR_BIT / tBits;
        }
        unsigned res = (unsigned)mBuffer & MASK;
        mBuffer >>= tBits;
        --mBufferUsesLeft;
        return res;
    }

    static const unsigned MASK = (1u << tBits) - 1;

    uint64_t mBuffer;
    unsigned mBufferUsesLeft;
};

} // opt

#endif // OPT_RANDOM_H