#ifndef ABC_ACTIONHIST_H
#define ABC_ACTIONHIST_H

#include <array>

namespace abc {

// Return ceil(x / y), where x and y are integers.
template<typename T>
constexpr T ceilIntDiv(const T x, const T y)
{
	return (x + y - 1) / y;
}

// Data structure storing a sequence of actions.
// N_BITS_PER_ACTION has to be a divisor of N_BITS_PER_INT.
template<unsigned N_BITS_PER_ACTION = 4, unsigned MAX_SIZE_ACTION_SEQ = 32>
class ActionHist
{
public:
	void clear()
	{
		for (uint64_t& x : data) x = 0;
		currInt = 0;
		currBit = 0;
	}

	// Add one entry to the end.
	void push_back(const uint64_t& x)
	{
		currBit += N_BITS_PER_ACTION;
		if (currBit == N_BITS_PER_INT) {
			++currInt;
			currBit = 0;
		}
		data[currInt] |= x << currBit;
	}

	// Pop the last entry.
	void pop_back()
	{
		data[currInt] &= (1ull << currBit) - 1;
		if (currBit == 0) {
			--currInt;
			currBit = N_BITS_PER_INT - N_BITS_PER_ACTION;
		}
		else
			currBit -= N_BITS_PER_ACTION;
	}

	// Return the last entry.
	uint8_t back() const
	{
		return data[currInt] >> currBit;
	}

	bool operator==(const ActionHist<N_BITS_PER_ACTION, MAX_SIZE_ACTION_SEQ>& rhs)
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

// Data structure storing a sequence of actions.
// Optimized version for N_BITS_PER_ACTION = 4 and MAX_SIZE_ACTION_SEQ = 32.
class StdActionHist
{
public:
	void clear()
	{
		m1 = 0;
		m2 = 0;
		onFirstInt = true;
		currBit = 0;
	}

	// Add one entry to the end.
	void push_back(const uint64_t& x)
	{
		currBit += N_BITS_PER_ACTION;
		if (currBit == N_BITS_PER_INT) {
			onFirstInt = false;
			currBit = 0;
		}
		(onFirstInt ? m1 : m2) |= x << currBit;
	}

	// Pop the last entry.
	void pop_back()
	{
		(onFirstInt ? m1 : m2) &= (1ull << currBit) - 1;
		if (currBit == 0) {
			onFirstInt = true;
			currBit = N_BITS_PER_INT - N_BITS_PER_ACTION;
		}
		else
			currBit -= N_BITS_PER_ACTION;
	}

	// Return the last entry.
	uint8_t back() const
	{
		return (onFirstInt ? m1 : m2) >> currBit;
	}

	bool operator==(const StdActionHist& rhs)
	{
		return m1 == rhs.m1 && m2 == rhs.m2;
	}

private:
	static const unsigned N_BITS_PER_ACTION = 4;
	static const unsigned MAX_SIZE_ACTION_SEQ = 32;
	static const unsigned N_BITS_PER_INT = 64;
	static const unsigned N_INTS = 2;

	uint64_t m1 = 0;
	uint64_t m2 = 0;

	bool onFirstInt = true;
	uint8_t currBit = 0;

};

} // abc

#endif // ABC_ACTIONHIST_H