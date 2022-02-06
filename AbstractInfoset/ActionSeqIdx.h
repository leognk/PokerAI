#ifndef ABC_ACTIONSEQIDX_H
#define ABC_ACTIONSEQIDX_H

#include <array>

namespace abc {

// Return ceil(x / y), where x and y are integers.
template<typename T>
constexpr T ceilIntDiv(const T x, const T y)
{
	return (x + y - 1) / y;
}

// N_BITS_PER_ACTION has to be a divisor of N_BITS_PER_INT.
template<unsigned N_BITS_PER_ACTION = 4, unsigned MAX_SIZE_ACTION_SEQ = 32>
class ActionSeqIdx
{
public:
	void addAction(const uint64_t& a)
	{
		data[currInt] |= a << currBit;
		currBit += N_BITS_PER_ACTION;
		if (currBit == N_BITS_PER_INT) {
			++currInt;
			currBit = 0;
		}
	}

	bool operator==(const ActionSeqIdx<N_BITS_PER_ACTION, MAX_SIZE_ACTION_SEQ>& rhs)
	{
		for (uint8_t i = 0; i < N_INTS; ++i) {
			if (data[i] != rhs.data[i])
				return false;
		}
		return true;
	}

private:
	static const unsigned N_BITS_PER_INT = 64;
	static const unsigned N_INTS = ceilIntDiv(N_BITS_PER_ACTION * MAX_SIZE_ACTION_SEQ, N_BITS_PER_INT);

	std::array<uint64_t, N_INTS> data{};

	uint8_t currInt = 0;
	uint8_t currBit = 0;

};

} // abc

#endif // ABC_ACTIONSEQIDX_H