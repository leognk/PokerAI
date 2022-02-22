#ifndef OPT_PROGRESSION_H
#define OPT_PROGRESSION_H

#include "StringManip.h"

namespace opt {

inline std::string progressPerc(
	const uint64_t currIter, const uint64_t endIter,
	const unsigned precision = 0, const bool withSpace = false)
{
	return roundStr(100.0 * currIter / endIter, precision) + (withSpace ? " " : "") + "%";
}

// Return the total time to finish all the iterations in seconds.
inline double totalTime(
	const uint64_t currIter, const uint64_t endIter,
	const double timeSpent)
{
	return timeSpent * endIter / currIter;
}

// Return the remaining time to finish all the iterations in seconds.
inline double remainingTime(
	const uint64_t currIter, const uint64_t endIter,
	const double timeSpent)
{
	return timeSpent * ((double)endIter / currIter - 1.0);
}

inline double remainingTime(const double timeSpent, const double totalTime)
{
	return totalTime - timeSpent;
}

} // opt

#endif // OPT_PROGRESSION_H