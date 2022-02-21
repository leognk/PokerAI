#ifndef OPT_TIME_H
#define OPT_TIME_H

#include <string>

namespace opt {

// Take a duration in seconds as the input and output
// the same duration in the format: "d day(s), h:m:s".
inline std::string prettyDuration(unsigned dt)
{
	unsigned d = dt / 86400; // 3600 * 24 = 86400
	dt %= 86400;
	unsigned h = dt / 3600;
	dt %= 3600;
	unsigned m = dt / 60;
	unsigned s = dt % 60;

	std::string res = "";

	// Day
	if (d != 0)
		res = std::to_string(d) + ((d == 1) ? " day, " : " days, ");

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

} // opt

#endif // OPT_TIME_H