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

template<unsigned nBitsPerAction, unsigned maxSizeActionSeq>
class ActionSeqIterator;
class StdActionSeqIterator;

class ActionSeqHash;
class StdActionSeqHash;

#pragma warning(push)
#pragma warning(disable: 4244)
// Data structure storing a sequence of actions.
// nBitsPerAction has to be a divisor of nBitsPerInt.
template<unsigned nBitsPerAction = 4, unsigned maxSizeActionSeq = 32>
class ActionSeq
{
public:
	typedef ActionSeqIterator<nBitsPerAction, maxSizeActionSeq> iter_t;
	typedef ActionSeqHash hasher_t;

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

	size_t sizeInBits() const
	{
		return currInt * nBitsPerInt + currBit;
	}

	size_t size() const
	{
		return sizeInBits() / nBitsPerAction;
	}

	bool operator==(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& rhs) const
	{
		if (currBit != rhs.currBit || currInt != rhs.currInt) return false;
		for (uint8_t i = 0; i < nInts; ++i) {
			if (data[i] != rhs.data[i]) return false;
		}
		return true;
	}

	bool operator<(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& rhs) const
	{
		if (currInt < rhs.currInt) return true;
		else if (currInt != rhs.currInt) return false;

		if (currBit < rhs.currBit) return true;
		else if (currBit != rhs.currBit) return false;

		for (uint8_t i = nInts; i > 0; --i) {
			if (data[i - 1] < rhs.data[i - 1]) return true;
			else if (data[i - 1] != rhs.data[i - 1]) return false;
		}

		return false;
	}

	// Return a sub-sequence of the current sequence from 0 to endIdx excluded.
	ActionSeq<nBitsPerAction, maxSizeActionSeq> extractSubSeq(uint8_t endIdx) const
	{
		ActionSeq<nBitsPerAction, maxSizeActionSeq> res;
		unsigned endBit = (unsigned)endIdx * nBitsPerAction;

		res.currInt = endBit / nBitsPerInt;
		res.currBit = endBit % nBitsPerInt;

		for (uint8_t i = 0; i < res.currInt; ++i)
			res.data[i] = data[i];
		res.data[res.currInt] = data[res.currInt] & ((1ull << res.currBit) - 1);

		return res;
	}

	bool isSubSeq(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& seq) const
	{
		if (currInt > seq.currInt
			|| (currInt == seq.currInt && currBit > seq.currBit))
			return false;

		for (uint8_t i = 0; i < currInt; ++i) {
			if (data[i] != seq.data[i]) return false;
		}

		uint64_t subInt = seq.data[currInt] & ((1ull << currBit) - 1);

		return data[currInt] == subInt;
	}

private:
	static const unsigned nBitsPerInt = 64;
	static const unsigned nInts = ceilIntDiv(nBitsPerAction * maxSizeActionSeq, nBitsPerInt);

	std::array<uint64_t, nInts> data{};

	uint8_t currInt = 0;
	uint8_t currBit = 0;

	friend class iter_t;
	friend class hasher_t;

};
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
// Data structure storing a sequence of actions.
// Optimized version of ActionSeq with nBitsPerAction = 4 and maxSizeActionSeq = 32.
class StdActionSeq
{
public:
	typedef StdActionSeqIterator iter_t;
	typedef StdActionSeqHash hasher_t;

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

	size_t sizeInBits() const
	{
		return onFirstInt ? currBit : (nBitsPerInt + currBit);
	}

	size_t size() const
	{
		return sizeInBits() / nBitsPerAction;
	}

	bool operator==(const StdActionSeq& rhs) const
	{
		return onFirstInt == rhs.onFirstInt && currBit == rhs.currBit
			&& m1 == rhs.m1 && m2 == rhs.m2;
	}

	bool operator<(const StdActionSeq& rhs) const
	{
		if (onFirstInt && !rhs.onFirstInt) return true;
		else if (onFirstInt != rhs.onFirstInt) return false;

		if (currBit < rhs.currBit) return true;
		else if (currBit != rhs.currBit) return false;

		return (m2 < rhs.m2) || (m2 == rhs.m2 && m1 < rhs.m1);
	}

	// Return a sub-sequence of the current sequence from 0 to endIdx excluded.
	StdActionSeq extractSubSeq(uint8_t endIdx) const
	{
		StdActionSeq res;
		unsigned endBit = (unsigned)endIdx * nBitsPerAction;

		res.onFirstInt = endBit < nBitsPerInt;
		if (res.onFirstInt) {
			res.currBit = endBit;
			res.m1 = m1 & ((1ull << res.currBit) - 1);
		}
		else {
			res.currBit = endBit - nBitsPerInt;
			res.m1 = m1;
			res.m2 = m2 & ((1ull << res.currBit) - 1);
		}

		return res;
	}

	bool isSubSeq(const StdActionSeq& seq) const
	{
		if ((!onFirstInt && seq.onFirstInt)
			|| (onFirstInt == seq.onFirstInt && currBit > seq.currBit))
			return false;

		if (onFirstInt) {
			uint8_t subInt = seq.m1 & ((1ull << currBit) - 1);
			return m1 == subInt;
		}

		else {
			if (m1 != seq.m1) return false;
			uint8_t subInt = seq.m2 & ((1ull << currBit) - 1);
			return m2 == subInt;
		}
	}

private:
	static const unsigned nBitsPerInt = 64;
	static const unsigned nInts = 2;

	uint64_t m1 = 0;
	uint64_t m2 = 0;

	bool onFirstInt = true;
	uint8_t currBit = 0;

	friend class iter_t;
	friend class hasher_t;

};
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
// A class to iterate over elements contained in ActionSeq.
template<unsigned nBitsPerAction, unsigned maxSizeActionSeq>
class ActionSeqIterator
{
public:
	ActionSeqIterator(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& seq) :
		seq(&seq)
	{
	}

	// Reset the state such that the next call to next will return the first entry.
	void clear(uint8_t startIdx = 0)
	{
		currInt = 0;
		currBit = 0;
	}

	// Return the entry in seq after the one returned by the previous call to this function.
	uint8_t next()
	{
		uint8_t res = (seq->data[currInt] >> currBit) & ((1ull << nBitsPerAction) - 1);
		currBit += nBitsPerAction;
		if (currBit == seq->nBitsPerInt) {
			++currInt;
			currBit = 0;
		}
		return res;
	}

	// Return whether the last call to next returned the last entry of seq.
	bool end() const
	{
		return currInt == seq->currInt && currBit == seq->currBit;
	}

private:
	const ActionSeq<nBitsPerAction, maxSizeActionSeq>* seq;

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
	StdActionSeqIterator(const StdActionSeq& seq) :
		seq(&seq)
	{
	}

	// Reset the state such that the next call to next will return the first entry.
	void clear(uint8_t startIdx = 0)
	{
		onFirstInt = true;
		currBit = 0;
	}

	// Return the entry in seq after the one returned by the previous call to this function.
	uint8_t next()
	{
		uint8_t res = ((onFirstInt ? seq->m1 : seq->m2) >> currBit) & ((1ull << seq->nBitsPerAction) - 1);
		currBit += seq->nBitsPerAction;
		if (currBit == seq->nBitsPerInt) {
			onFirstInt = false;
			currBit = 0;
		}
		return res;
	}

	// Return whether the last call to next returned the last entry of seq.
	bool end() const
	{
		return onFirstInt == seq->onFirstInt && currBit == seq->currBit;
	}

private:
	const StdActionSeq* seq;

	bool onFirstInt = true;
	uint8_t currBit = 0;

};
#pragma warning(pop)

template<class ResSeq, class Seq1, class Seq2>
ResSeq concatActionSeqs(const Seq1& seq1, const Seq2& seq2)
{
	ResSeq res;

	typename Seq1::iter_t iter1(seq1);
	while (!iter1.end())
		res.push_back(iter1.next());

	typename Seq2::iter_t iter2(seq2);
	while (!iter2.end())
		res.push_back(iter2.next());

	return res;
}

template<class Seq>
std::vector<uint8_t> seqToVect(const Seq& seq)
{
	std::vector<uint8_t> res;
	res.reserve(seq.maxSizeActionSeq);
	typename Seq::iter_t iter(seq);
	while (!iter.end())
		res.push_back(iter.next());
	return res;
}

class ActionSeqHash
{
public:
	template<unsigned nBitsPerAction, unsigned maxSizeActionSeq>
	uint64_t operator()(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& h, uint64_t seed = 0) const
	{
		std::array<uint64_t, h.nInts + 1> buffer = { h.sizeInBits() };
		std::copy(h.data.begin(), h.data.end(), buffer.begin() + 1);
		return XXH3_64bits_withSeed(buffer.data(), sizeof(buffer), seed);
	}
};

class StdActionSeqHash
{
public:
	uint64_t operator()(const StdActionSeq& h, uint64_t seed = 0) const
	{
		uint64_t buffer[3] = { h.sizeInBits(), h.m1, h.m2 };
		return XXH3_64bits_withSeed(buffer, sizeof(buffer), seed);
	}
};

} // abc

#endif // ABC_ACTIONSEQ_H