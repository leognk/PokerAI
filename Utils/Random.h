#ifndef OPT_RANDOM_H
#define OPT_RANDOM_H

#include <cstdint>
#include <stdexcept>

namespace opt {

// Generate a random index with the given
// array of cumulated weights (uintN_t with N > tBits
// or float with enough bits on the fractional part).
// The sum of the weights must be equal to RANGE,
// which is 2^tBits.
// Ideally, the number N of elements must be small
// compared to RANGE (and it has to be less than RANGE),
// because the precision of the probability is 1 / RANGE and
// the order of magnitude of the actual probabilities is 1 / N.
// In other words, log2(N) < tBits.
// Also preferably, cumWeights.back() < RANGE.
// tBits must be less than 53 excluded because in rescaleCumWeights,
// RANGE will be converted to a double which uses 53 bits for
// the fractional part.
template<unsigned tBits = 16>
class FastRandomChoice
{
public:
    FastRandomChoice()
    {
        mBuffer = 0;
        mBufferUsesLeft = 0;
    }

    // Rescale each cumulated weight.
    // After this, the total sum is guaranteed to be
    // exactly equal to RANGE.
    template<class C>
    void rescaleCumWeights(C& cumWeights)
    {
        double rescaleFactor = (double)RANGE / cumWeights.back();
        for (unsigned i = 0; i < cumWeights.size(); ++i) 
#pragma warning(suppress: 4244)
            cumWeights[i] = round(rescaleFactor * cumWeights[i]);
        if (cumWeights.back() != RANGE)
            throw std::runtime_error("Sum of weights not equal to RANGE.");
    }

    template<class C, class TRng>
    unsigned operator()(const C& cumWeights, TRng& rng)
    {
        uint64_t x = rand(rng);
        unsigned i = 0;
        while (cumWeights[i] <= x)
            ++i;
        return i;
    }

    static const uint64_t RANGE = 1ull << tBits;

private:
    // Generate a random uint64_t between 0 and RANGE excluded.
    template<class TRng>
    uint64_t rand(TRng& rng)
    {
        static_assert(sizeof(typename TRng::result_type) == sizeof(uint64_t), "64-bit RNG required.");
        if (mBufferUsesLeft == 0) {
            mBuffer = rng();
            mBufferUsesLeft = sizeof(mBuffer) * CHAR_BIT / tBits;
        }
        uint64_t res = mBuffer & MASK;
        mBuffer >>= tBits;
        --mBufferUsesLeft;
        return res;
    }

#pragma warning(suppress: 26454)
    static const uint64_t MASK = (1ull << tBits) - 1;

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
public:
    typedef uint64_t result_type;

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