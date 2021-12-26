#ifndef OPT_RANDOM_H
#define OPT_RANDOM_H

#include <cstdint> 

namespace opt {

    // Generate a random index according to the given
    // vector of probabilities. The precision is 1 / 2^tBits.
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
        size_t operator()(C& proba, TRng& rng)
        {
            double x = rand(rng);
            size_t i = 0;
            double cumProba = proba[0];
            while (cumProba <= x)
                cumProba += proba[++i];
            return i;
        }

    private:
        // Generate a random real number between 0 and 1.
        template<class TRng>
        double rand(TRng& rng)
        {
            static_assert(sizeof(typename TRng::result_type) == sizeof(uint64_t), "64-bit RNG required.");
            if (mBufferUsesLeft == 0) {
                mBuffer = rng();
                mBufferUsesLeft = sizeof(mBuffer) * CHAR_BIT / tBits;
            }
            double res = (unsigned)mBuffer & MASK;
            mBuffer >>= tBits;
            --mBufferUsesLeft;
            // We divide by the range, which is MASK + 1.
            return res / (MASK + 1);
        }

        static const unsigned MASK = (1u << tBits) - 1;

        uint64_t mBuffer;
        unsigned mBufferUsesLeft;
    };

} // opt

#endif // OPT_RANDOM_H