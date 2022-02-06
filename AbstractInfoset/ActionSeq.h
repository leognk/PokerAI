#ifndef ABC_ACTIONSEQ_H
#define ABC_ACTIONSEQ_H

#include <array>
#include "xxhash.h"

namespace abc {

// Return ceil(x / y), where x and y are integers.
template<typename T>
constexpr T ceilIntDiv(const T x, const T y)
{
	return (x + y - 1) / y;
}

class ActionSeqIterator;
class StdActionSeqIterator;

#pragma warning(push)
#pragma warning(disable: 4244)
// Data structure storing a sequence of actions.
// nBitsPerAction has to be a divisor of nBitsPerInt.
template<unsigned nBitsPerAction = 4, unsigned maxSizeActionSeq = 32>
class ActionSeq
{
public:
	typedef ActionSeqIterator iter_t;
	static const unsigned nBitsPerAction = nBitsPerAction;
	static const unsigned maxSizeActionSeq = maxSizeActionSeq;

	void clear()
	{
		for (uint64_t& x : data) x = 0;
		currInt = 0;
		currBit = 0;
	}

	// Add one entry to the end.
	void push_back(const uint64_t& x)
	{
		data[currInt] |= x << currBit;
		currBit += nBitsPerAction;
		if (currBit == nBitsPerInt) {
			++currInt;
			currBit = 0;
		}
	}

	// Pop the last entry.
	void pop_back()
	{
		if (currBit == 0) {
			--currInt;
			currBit = nBitsPerInt - nBitsPerAction;
		}
		else
			currBit -= nBitsPerAction;
		data[currInt] &= (1ull << currBit) - 1;
	}

	// Return the last entry.
	uint8_t back() const
	{
		if (currBit == 0)
			return data[currInt - 1] >> (nBitsPerInt - nBitsPerAction);
		else
			return data[currInt] >> (currBit - nBitsPerAction);
	}

	bool operator==(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& rhs) const
	{
		for (uint8_t i = 0; i < nInts; ++i) {
			if (data[i] != rhs.data[i])
				return false;
		}
		return true;
	}

private:
	static const unsigned nBitsPerInt = 64;
	static const unsigned nInts = ceilIntDiv(nBitsPerAction * maxSizeActionSeq, nBitsPerInt);
	static const unsigned nActionsPerInt = nBitsPerInt / nBitsPerAction;

	std::array<uint64_t, nInts> data{};

	uint8_t currInt = 0;
	uint8_t currBit = 0;

	uint8_t currIntIter = 0;
	uint8_t currBitIter = 0;

	friend class ActionSeqIterator;
	friend class ActionSeqHash;

};
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
// Data structure storing a sequence of actions.
// Optimized version for nBitsPerAction = 4 and maxSizeActionSeq = 32.
class StdActionSeq
{
public:
	typedef StdActionSeqIterator iter_t;
	static const unsigned nBitsPerAction = 4;
	static const unsigned maxSizeActionSeq = 32;

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
		(onFirstInt ? m1 : m2) |= x << currBit;
		currBit += nBitsPerAction;
		if (currBit == nBitsPerInt) {
			onFirstInt = false;
			currBit = 0;
		}
	}

	// Pop the last entry.
	void pop_back()
	{
		if (currBit == 0) {
			onFirstInt = true;
			currBit = nBitsPerInt - nBitsPerAction;
		}
		else
			currBit -= nBitsPerAction;
		(onFirstInt ? m1 : m2) &= (1ull << currBit) - 1;
	}

	// Return the last entry.
	uint8_t back() const
	{
		if (currBit == 0)
			return m1 >> (nBitsPerInt - nBitsPerAction);
		else
			return (onFirstInt ? m1 : m2) >> (currBit - nBitsPerAction);
	}

	bool operator==(const StdActionSeq& rhs) const
	{
		return m1 == rhs.m1 && m2 == rhs.m2;
	}

private:
	static const unsigned nBitsPerInt = 64;
	static const unsigned nInts = 2;

	uint64_t m1 = 0;
	uint64_t m2 = 0;

	bool onFirstInt = true;
	uint8_t currBit = 0;

	friend class StdActionSeqIterator;
	friend class StdActionSeqHash;

};
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
// A class to iterate over elements contained in ActionSeq.
class ActionSeqIterator
{
public:
	// Reset the state such that the next call to iterNext will return the first entry.
	void clearIter()
	{
		currInt = 0;
		currBit = 0;
	}

	// Return the entry in seq after the one returned by the previous call to this function.
	template<unsigned nBitsPerAction, unsigned maxSizeActionSeq>
	uint8_t iterNext(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& seq)
	{
		uint8_t res = seq.data[currInt] >> currBit;
		currBit += nBitsPerAction;
		if (currBit == seq.nBitsPerInt) {
			++currInt;
			currBit = 0;
		}
		return res;
	}

	// Return whether the last call to iterNext returned the last entry of seq.
	template<unsigned nBitsPerAction, unsigned maxSizeActionSeq>
	bool iterEnd(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& seq) const
	{
		return currInt == seq.currInt && currBit == seq.currBit;
	}

private:
	uint8_t currInt = 0;
	uint8_t currBit = 0;

};
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
// A class to iterate over elements contained in StdActionSeq.
class StdActionSeqIterator
{
public:
	// Reset the state such that the next call to iterNext will return the first entry.
	void clearIter()
	{
		onFirstInt = true;
		currBit = 0;
	}

	// Return the entry in seq after the one returned by the previous call to this function.
	uint8_t iterNext(const StdActionSeq& seq)
	{
		uint8_t res = (onFirstInt ? seq.m1 : seq.m2) >> currBit;
		currBit += seq.nBitsPerAction;
		if (currBit == seq.nBitsPerInt) {
			onFirstInt = false;
			currBit = 0;
		}
		return res;
	}

	// Return whether the last call to iterNext returned the last entry of seq.
	bool iterEnd(const StdActionSeq& seq) const
	{
		return onFirstInt == seq.onFirstInt && currBit == seq.currBit;
	}

private:
	bool onFirstInt = true;
	uint8_t currBit = 0;

};
#pragma warning(pop)

template<unsigned nBitsPerAction, unsigned maxSizeActionSeq, typename Seq1, typename Seq2>
ActionSeq<nBitsPerAction, maxSizeActionSeq> concatActionSeqs(const Seq1& seq1, const Seq2& seq2)
{
	ActionSeq<nBitsPerAction, maxSizeActionSeq> res;

	typename Seq1::iter_t iter1;
	iter1.clearIter();
	while (!iter1.iterEnd(seq1))
		res.push_back(iter1.iterNext(seq1));

	typename Seq2::iter_t iter2;
	iter2.clearIter();
	while (!iter2.iterEnd(seq2))
		res.push_back(iter2.iterNext(seq2));

	return res;
}

class ActionSeqHash
{
public:
	template<unsigned nBitsPerAction = 4, unsigned maxSizeActionSeq = 32>
	uint64_t operator()(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& h, uint64_t seed = 0) const
	{
		return XXH3_64bits_withSeed(h.data.data(), sizeof(h.data), seed);
	}
};

class StdActionSeqHash
{
public:
	// Source: https://stackoverflow.com/questions/35985960/c-why-is-boosthash-combine-the-best-way-to-combine-hash-values
	uint64_t operator()(const StdActionSeq& h, uint64_t seed = 0) const
	{
		seed ^= h.m1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= h.m2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}
};

} // abc

#endif // ABC_ACTIONSEQ_H