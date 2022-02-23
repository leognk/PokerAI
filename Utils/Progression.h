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
	const time_t startTime, bool withSpace = true, bool compactDuration = false)
{
	const std::string perc = progressPerc(currIter, endIter);

	const std::string currIterStr = prettyNumDg(currIter, 3);
	const std::string endIterStr = prettyNumDg(endIter, 3);

	const double timeSpent = getDuration(startTime);
	const double totTime = totalTime(currIter, endIter, timeSpent);
	const std::string remTimeStr = prettyDuration(remainingTime(timeSpent, totTime), compactDuration);
	const std::string timeSpentStr = prettyDuration(timeSpent, compactDuration);
	const std::string totalTimeStr = prettyDuration(totTime, compactDuration);

	const std::string itPerSec = prettyNumDg(iterPerSec(currIter, timeSpent), 3, true);
	const std::string secPerIt = prettyNumDg(secPerIter(currIter, timeSpent), 3, true);

	std::ostringstream os;
	std::string sp = withSpace ? " " : "";

	os << std::setw(4) << perc
		<< " | " << std::setw(5) << currIterStr << sp << "/" << sp << endIterStr
		<< " | " << remTimeStr << sp << "->" << sp << timeSpentStr << sp << "/" << sp << totalTimeStr
		<< " | " << std::setw(5) << itPerSec << "it/s"
		<< " | " << std::setw(5) << secPerIt << "sec/it";

	return os.str();
}

} // opt

#endif // OPT_PROGRESSION_H