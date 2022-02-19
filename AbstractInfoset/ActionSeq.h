#ifndef ABC_ACTIONSEQ_H
#define ABC_ACTIONSEQ_H

#include "xxhash.h"

namespace abc {

// Return ceil(x / y), where x and y are integers.
template<typename T>
constexpr T ceilIntDiv(const T x, const T y)
{
	return (x + y - 1) / y;
}

template<class Seq>
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
	typedef ActionSeqIterator<ActionSeq<nBitsPerAction, maxSizeActionSeq>> iter_t;

	static const unsigned nBitsPerAction = nBitsPerAction;
	static const unsigned maxSizeActionSeq = maxSizeActionSeq;
	static const unsigned nBitsPerInt = 64;
	static const unsigned nInts = ceilIntDiv(nBitsPerAction * maxSizeActionSeq, nBitsPerInt);
	static const unsigned nActionsPerInt = nBitsPerInt / nBitsPerAction;
	static const uint64_t actionMask = (1ull << nBitsPerAction) - 1;

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
		++data[nInts];
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
		--data[nInts];
	}

	// Return the last entry.
	uint8_t back() const
	{
		if (currBit == 0)
			return data[currInt - 1] >> (nBitsPerInt - nBitsPerAction);
		else
			return data[currInt] >> (currBit - nBitsPerAction);
	}

	size_t size() const
	{
		return data[nInts];
	}

	bool operator==(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& rhs) const
	{
		if (data[nInts] != rhs.data[nInts]) return false;

		if (currBit != 0) {
			for (uint8_t i = 0; i <= currInt; ++i) {
				if (data[i] != rhs.data[i]) return false;
			}
			return true;
		}
		
		else {
			for (uint8_t i = 0; i < currInt; ++i) {
				if (data[i] != rhs.data[i]) return false;
			}
			return true;
		}
	}

	bool operator<(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& rhs) const
	{
		if (data[nInts] < rhs.data[nInts]) return true;
		else if (data[nInts] != rhs.data[nInts]) return false;

		unsigned pAction = 0;
		for (uint8_t pInt = 0; pInt < nInts; ++pInt) {
			uint64_t n1 = data[pInt];
			uint64_t n2 = rhs.data[pInt];
			for (uint8_t i = 0; i < nActionsPerInt; ++i) {
				uint8_t x1 = n1 & actionMask;
				uint8_t x2 = n2 & actionMask;
				if (x1 < x2) return true;
				else if (x1 != x2) return false;
				// End of sequence.
				if (++pAction == data[nInts]) return false;
				n1 >>= nBitsPerAction;
				n2 >>= nBitsPerAction;
			}
		}
	}

	// Same as operator< but takes into account the number of players
	// at the end of the sequences.
	bool lessThanWithNPlayers(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& rhs) const
	{
		if (data[nInts] < rhs.data[nInts]) return true;
		else if (data[nInts] != rhs.data[nInts]) return false;

		// Compare the number of players.
		if (back() < rhs.back()) return true;
		else if (back() != rhs.back()) return false;

		unsigned pAction = 0;
		for (uint8_t pInt = 0; pInt < nInts; ++pInt) {
			uint64_t n1 = data[pInt];
			uint64_t n2 = rhs.data[pInt];
			for (uint8_t i = 0; i < nActionsPerInt; ++i) {
				uint8_t x1 = n1 & actionMask;
				uint8_t x2 = n2 & actionMask;
				if (x1 < x2) return true;
				else if (x1 != x2) return false;
				// End of sequence.
				if (++pAction == data[nInts] - 1) return false;
				n1 >>= nBitsPerAction;
				n2 >>= nBitsPerAction;
			}
		}
	}

	bool haveSameParents(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& seq) const
	{
		if (data[nInts] != seq.data[nInts]) return false;

		if (currBit != 0) {
			for (uint8_t i = 0; i < currInt; ++i) {
				if (data[i] != seq.data[i]) return false;
			}
			uint64_t mask = (1ull << (currBit - nBitsPerAction)) - 1;
			return (data[currInt] & mask) == (seq.data[currInt] & mask);
		}

		else {
			for (uint8_t i = 0; i < currInt - 1; ++i) {
				if (data[i] != seq.data[i]) return false;
			}
			uint64_t mask = (1ull << (nBitsPerInt - nBitsPerAction)) - 1;
			return (data[currInt - 1] & mask) == (seq.data[currInt - 1] & mask);
		}
	}

	// Same as haveSameParents but takes into account the number of players
	// at the end of the sequences.
	bool haveSameParentsWithNPlayers(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& seq) const
	{
		if (data[nInts] != seq.data[nInts]) return false;

		// Compare the number of players.
		if (back() != seq.back()) return false;

		uint8_t lastBit, lastInt;
		if (currBit != 0) {
			lastBit = currBit - nBitsPerAction;
			lastInt = currInt;
		}
		else {
			lastBit = nBitsPerInt - nBitsPerAction;
			lastInt = currInt - 1;
		}

		if (lastBit != 0) {
			for (uint8_t i = 0; i < lastInt; ++i) {
				if (data[i] != seq.data[i]) return false;
			}
			uint64_t mask = (1ull << (lastBit - nBitsPerAction)) - 1;
			return (data[lastInt] & mask) == (seq.data[lastInt] & mask);
		}

		else {
			for (uint8_t i = 0; i < lastInt - 1; ++i) {
				if (data[i] != seq.data[i]) return false;
			}
			uint64_t mask = (1ull << (nBitsPerInt - nBitsPerAction)) - 1;
			return (data[lastInt - 1] & mask) == (seq.data[lastInt - 1] & mask);
		}
	}

	// Return a sub-sequence of the current sequence from 0 to endIdx excluded.
	ActionSeq<nBitsPerAction, maxSizeActionSeq> extractSubSeq(uint8_t endIdx) const
	{
		ActionSeq<nBitsPerAction, maxSizeActionSeq> res;
		unsigned endBit = (unsigned)endIdx * nBitsPerAction;

		res.data[nInts] = endIdx;

		res.currInt = endBit / nBitsPerInt;
		res.currBit = endBit % nBitsPerInt;

		for (uint8_t i = 0; i < res.currInt; ++i)
			res.data[i] = data[i];
		res.data[res.currInt] = data[res.currInt] & ((1ull << res.currBit) - 1);

		return res;
	}

	bool isSubSeq(const ActionSeq<nBitsPerAction, maxSizeActionSeq>& seq) const
	{
		if (data[nInts] > seq.data[nInts]) return false;

		for (uint8_t i = 0; i < currInt; ++i) {
			if (data[i] != seq.data[i]) return false;
		}

		uint64_t subInt = seq.data[currInt] & ((1ull << currBit) - 1);

		return data[currInt] == subInt;
	}

	// The nInts first elements are the integers containing the sequence
	// and the last element is the size of the sequence.
	typedef std::array<uint64_t, nInts + 1> data_t;
	data_t data = { 0 };

private:
	uint8_t currInt = 0;
	uint8_t currBit = 0;

	friend class iter_t;

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

	static const unsigned nBitsPerAction = 4;
	static const unsigned maxSizeActionSeq = 32;
	static const unsigned nBitsPerInt = 64;
	static const unsigned nInts = 2;
	static const unsigned nActionsPerInt = nBitsPerInt / nBitsPerAction;
	static const uint64_t actionMask = (1ull << nBitsPerAction) - 1;

	void clear()
	{
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
		onFirstInt = true;
		currBit = 0;
	}

	// Add one entry to the end.
	void push_back(const uint64_t& x)
	{
		(onFirstInt ? data[0] : data[1]) |= x << currBit;
		currBit += nBitsPerAction;
		if (currBit == nBitsPerInt) {
			onFirstInt = false;
			currBit = 0;
		}
		++data[2];
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
		(onFirstInt ? data[0] : data[1]) &= (1ull << currBit) - 1;
		--data[2];
	}

	// Return the last entry.
	uint8_t back() const
	{
		if (currBit == 0)
			return data[0] >> (nBitsPerInt - nBitsPerAction);
		else
			return (onFirstInt ? data[0] : data[1]) >> (currBit - nBitsPerAction);
	}

	size_t size() const
	{
		return data[2];
	}

	bool operator==(const StdActionSeq& rhs) const
	{
		return data[2] == rhs.data[2] && data[0] == rhs.data[0] && data[1] == rhs.data[1];
	}

	bool operator<(const StdActionSeq& rhs) const
	{
		if (data[nInts] < rhs.data[nInts]) return true;
		else if (data[nInts] != rhs.data[nInts]) return false;

		unsigned pAction = 0;
		for (uint8_t pInt = 0; pInt < nInts; ++pInt) {
			uint64_t n1 = data[pInt];
			uint64_t n2 = rhs.data[pInt];
			for (uint8_t i = 0; i < nActionsPerInt; ++i) {
				uint8_t x1 = n1 & actionMask;
				uint8_t x2 = n2 & actionMask;
				if (x1 < x2) return true;
				else if (x1 != x2) return false;
				// End of sequence.
				if (++pAction == data[nInts]) return false;
				n1 >>= nBitsPerAction;
				n2 >>= nBitsPerAction;
			}
		}
	}

	// Same as operator< but takes into account the number of players
	// at the end of the sequences.
	bool lessThanWithNPlayers(const StdActionSeq& rhs) const
	{
		if (data[nInts] < rhs.data[nInts]) return true;
		else if (data[nInts] != rhs.data[nInts]) return false;

		// Compare the number of players.
		if (back() < rhs.back()) return true;
		else if (back() != rhs.back()) return false;

		unsigned pAction = 0;
		for (uint8_t pInt = 0; pInt < nInts; ++pInt) {
			uint64_t n1 = data[pInt];
			uint64_t n2 = rhs.data[pInt];
			for (uint8_t i = 0; i < nActionsPerInt; ++i) {
				uint8_t x1 = n1 & actionMask;
				uint8_t x2 = n2 & actionMask;
				if (x1 < x2) return true;
				else if (x1 != x2) return false;
				// End of sequence.
				if (++pAction == data[nInts] - 1) return false;
				n1 >>= nBitsPerAction;
				n2 >>= nBitsPerAction;
			}
		}
	}

	bool haveSameParents(const StdActionSeq& seq) const
	{
		if (data[nInts] != seq.data[nInts]) return false;

		if (onFirstInt) {
			uint64_t mask = (1ull << (currBit - nBitsPerAction)) - 1;
			return (data[0] & mask) == (seq.data[0] & mask);
		}

		else {
			if (currBit != 0) {
				uint64_t mask = (1ull << (currBit - nBitsPerAction)) - 1;
				return data[0] == seq.data[0] && (data[1] & mask) == (seq.data[1] & mask);
			}
			else {
				uint64_t mask = (1ull << (nBitsPerInt - nBitsPerAction)) - 1;
				return (data[0] & mask) == (seq.data[0] & mask);
			}
		}
	}

	// Same as haveSameParents but takes into account the number of players
	// at the end of the sequences.
	bool haveSameParentsWithNPlayers(const StdActionSeq& seq) const
	{
		if (data[nInts] != seq.data[nInts]) return false;

		// Compare the number of players.
		if (back() != seq.back()) return false;

		uint8_t prevOnFirstInt, prevBit;
		if (onFirstInt) {
			prevOnFirstInt = true;
			prevBit = currBit - nBitsPerAction;
		}
		else {
			if (currBit != 0) {
				prevOnFirstInt = false;
				prevBit = currBit - nBitsPerAction;
			}
			else {
				prevOnFirstInt = true;
				prevBit = nBitsPerInt - nBitsPerAction;
			}
		}

		if (prevOnFirstInt) {
			uint64_t mask = (1ull << (prevBit - nBitsPerAction)) - 1;
			return (data[0] & mask) == (seq.data[0] & mask);
		}

		else {
			if (prevBit != 0) {
				uint64_t mask = (1ull << (prevBit - nBitsPerAction)) - 1;
				return data[0] == seq.data[0] && (data[1] & mask) == (seq.data[1] & mask);
			}
			else {
				uint64_t mask = (1ull << (nBitsPerInt - nBitsPerAction)) - 1;
				return (data[0] & mask) == (seq.data[0] & mask);
			}
		}
	}

	// Return a sub-sequence of the current sequence from 0 to endIdx excluded.
	StdActionSeq extractSubSeq(uint8_t endIdx) const
	{
		StdActionSeq res;
		unsigned endBit = (unsigned)endIdx * nBitsPerAction;

		res.data[2] = endIdx;
		res.onFirstInt = endBit < nBitsPerInt;
		if (res.onFirstInt) {
			res.currBit = endBit;
			res.data[0] = data[0] & ((1ull << res.currBit) - 1);
		}
		else {
			res.currBit = endBit - nBitsPerInt;
			res.data[0] = data[0];
			res.data[1] = data[1] & ((1ull << res.currBit) - 1);
		}

		return res;
	}

	bool isSubSeq(const StdActionSeq& seq) const
	{
		if (data[2] > seq.data[2]) return false;

		if (onFirstInt) {
			uint8_t subInt = seq.data[0] & ((1ull << currBit) - 1);
			return data[0] == subInt;
		}

		else {
			if (data[0] != seq.data[0]) return false;
			uint8_t subInt = seq.data[1] & ((1ull << currBit) - 1);
			return data[1] == subInt;
		}
	}

	// The first two elements are the integers containing the sequence
	// and the third element is the size of the sequence.
	// Can be used to hash the sequence.
	typedef std::array<uint64_t, nInts + 1> data_t;
	data_t data = { 0 };

private:
	bool onFirstInt = true;
	uint8_t currBit = 0;

	friend class iter_t;

};
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
// A class to iterate over elements contained in ActionSeq.
template<class Seq>
class ActionSeqIterator
{
public:
	ActionSeqIterator(const Seq& seq) :
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
		uint8_t res = (seq->data[currInt] >> currBit) & ((1ull << seq->nBitsPerAction) - 1);
		currBit += seq->nBitsPerAction;
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
	const Seq* seq;

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
		uint8_t res = ((onFirstInt ? seq->data[0] : seq->data[1]) >> currBit) & ((1ull << seq->nBitsPerAction) - 1);
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

template<class Seq>
bool compareSeqsWithNPlayers(const Seq& lhs, const Seq& rhs)
{
	return lhs.lessThanWithNPlayers(rhs);
}

class ActionSeqHash
{
public:
	template<class Seq>
	uint64_t operator()(const Seq& seq, uint64_t seed = 0) const
	{
		return XXH3_64bits_withSeed(seq.data.data(), sizeof(seq.data), seed);
	}
};

} // abc

#endif // ABC_ACTIONSEQ_H