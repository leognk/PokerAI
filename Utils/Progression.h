#ifndef OPT_PROGRESSION_H
#define OPT_PROGRESSION_H

#include "StringManip.h"
#include "Time.h"
#include <iomanip>

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

inline double remainingTime(
	const uint64_t currIter, const uint64_t endIter,
	const time_t startTime, const double extraDuration = 0.0)
{
	return (extraDuration + getDuration(startTime)) * ((double)endIter / currIter - 1.0);
}

inline double remainingTime(const double timeSpent, const double totalTime)
{
	return totalTime - timeSpent;
}

inline double iterPerSec(const uint64_t currIter, const double timeSpent)
{
	return currIter / timeSpent;
}

inline double secPerIter(const uint64_t currIter, const double timeSpent)
{
	return timeSpent / currIter;
}

inline std::string progressStr(
	const uint64_t currIter, const uint64_t endIter,
	const time_t startTime, double extraDuration = 0,
	bool align = false)
{
	const std::string perc = progressPerc(currIter, endIter);

	const std::string currIterStr = prettyNumDg(currIter, 3);
	const std::string endIterStr = prettyNumDg(endIter, 3);

	const double timeSpent = extraDuration + getDuration(startTime);
	const double totTime = totalTime(currIter, endIter, timeSpent);
	const std::string remTimeStr = prettyDuration(remainingTime(timeSpent, totTime));
	const std::string timeSpentStr = prettyDuration(timeSpent);
	const std::string totalTimeStr = prettyDuration(totTime);

	const std::string itPerSec = prettyNumDg(iterPerSec(currIter, timeSpent), 3, true);
	const std::string secPerIt = prettyNumDg(secPerIter(currIter, timeSpent), 3, true);

	std::ostringstream os;

	os << std::setw(align ? 4 : 0) << perc
		<< " | " << std::setw(align ? 5 : 0) << currIterStr << " / " << endIterStr
		<< " | " << remTimeStr << " -> " << timeSpentStr << " / " << totalTimeStr
		<< " | " << itPerSec << "it/s"
		<< " | " << secPerIt << "s/it";

	return os.str();
}

} // opt

#endif // OPT_PROGRESSION_H