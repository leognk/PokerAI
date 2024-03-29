#ifndef OPT_HASH_H
#define OPT_HASH_H

#include "xxhash.h"

namespace opt {

class ArrayHash
{
public:
	template<typename A>
	uint64_t operator()(const A& a, uint64_t seed = 0) const
	{
		return XXH3_64bits_withSeed(a.data(), sizeof(a), seed);
	}
};

class ContainerHash
{
public:
	template<typename C>
	uint64_t operator()(const C& c, uint64_t seed = 0) const
	{
		return XXH3_64bits_withSeed(c.data(), c.size() * sizeof(C::value_type), seed);
	}
};

} // opt

#endif // OPT_HASH_H