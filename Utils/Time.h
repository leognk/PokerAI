#ifndef OPT_TIME_H
#define OPT_TIME_H

#include <string>
#include <chrono>

namespace opt {

typedef std::chrono::high_resolution_clock::time_point time_t;

inline time_t getTime()
{
	return std::chrono::high_resolution_clock::now();
}

inline double getDuration(const time_t& startTime, const time_t& endTime)
{
	return 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>
		(endTime - startTime).count();
}

inline double getDuration(const time_t& startTime)
{
	return getDuration(startTime, getTime());
}

// Take a duration in seconds as the input and output
// the same duration in the format: "d day(s), h:m:s".
inline std::string prettyDuration(unsigned dt, bool compact = false)
{
	unsigned d = dt / 86400; // 3600 * 24 = 86400
	dt %= 86400;
	unsigned h = dt / 3600;
	dt %= 3600;
	unsigned m = dt / 60;
	unsigned s = dt % 60;

	std::string res = "";

	// Day
	if (d != 0) {
		res = std::to_string(d);
		if (compact) res += "d,";
		else res += ((d == 1) ? " day, " : " days, ");
	}

	// Hour
	if (h < 10) res += "0";
	res += std::to_string(h) + ":";

	// Minute
	if (m < 10) res += "0";
	res += std::to_string(m) + ":";

	// Second
	if (s < 10) res += "0";
	res += std::to_string(s);

	return res;
}

inline std::string prettyDuration(const double& dt, bool compact = false)
{
	return prettyDuration((unsigned)std::round(dt), compact);
}

inline std::string prettyDuration(const time_t& startTime, const time_t& endTime, bool compact = false)
{
	return prettyDuration(getDuration(startTime, endTime), compact);
}

inline std::string prettyDuration(const time_t& startTime, bool compact = false)
{
	return prettyDuration(startTime, getTime(), compact);
}

} // opt

#endif // OPT_TIME_H