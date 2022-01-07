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

class SplitMix64
{
    typedef uint64_t result_type;
public:
    SplitMix64(uint64_t seed) { x = seed; }
    uint64_t operator()()
    {
        uint64_t z = (x += 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }
    static uint64_t(min)() { return 0; }
    static uint64_t(max)() { return ~(uint64_t)0; }
private:
    uint64_t x;
};

class XoShiro256PlusPlus
{
    typedef uint64_t result_type;

public:
    XoShiro256PlusPlus(uint64_t seed)
    {
        SplitMix64 seeder(seed);
        mState[0] = seeder();
        mState[1] = seeder();
        mState[2] = seeder();
        mState[3] = seeder();
        // Warm-up the RNG.
        for (unsigned i = 0; i < 10000; ++i)
            operator()();
    }

    uint64_t operator()()
    {
        uint64_t s0 = mState[0];
        uint64_t s1 = mState[1];
        uint64_t s2 = mState[2];
        uint64_t s3 = mState[3];

        uint64_t result = rotl(s0 + s3, 23) + s0;
        uint64_t t = s1 << 17;

        mState[2] ^= s0;
        mState[3] ^= s1;
        mState[1] ^= s2;
        mState[0] ^= s3;

        mState[2] ^= t;

        mState[3] = rotl(s3, 45);

        return result;
    }

    static uint64_t(min)()
    {
        return 0;
    }

    static uint64_t(max)()
    {
        return ~(uint64_t)0;
    }

private:
    static uint64_t rotl(uint64_t x, unsigned k)
    {
        // MSVC and most g++ versions will compile this to rotl on x64.
        return (x << k) | (x >> (64 - k));
    }

    uint64_t mState[4];
};

} // opt

#endif // OPT_RANDOM_H