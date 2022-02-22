#ifndef OPT_STRINGMANIP_H
#define OPT_STRINGMANIP_H

#include <string>

namespace opt {

inline std::string toUpper(const std::string& s)
{
	std::string res = s;
	for (auto& c : res) c = toupper(c);
	return res;
}

inline double round(const double value, const unsigned precision = 0)
{
	const double multiplier = std::pow(10.0, precision);
	return std::round(value * multiplier) / multiplier;
}

inline std::string roundStr(const double value, const unsigned precision = 0)
{
	std::ostringstream os;
	os.precision(precision);
	os << std::fixed << round(value, precision);
	return os.str();
}

// Output the double d in a string with a maximum of 4 characters if precision = 0
// or 5 + precision characters using G (giga), M (mega), K (kilo), milli, micro, nano.
inline std::string prettyNumber(
	const double d, const unsigned precision = 0, const bool withSpace = false)
{
	// giga
	if (d >= 1e9)
		return roundStr(d * 1e-9, precision) + (withSpace ? " " : "") + "G";

	// mega
	else if (d >= 1e6)
		return roundStr(d * 1e-6, precision) + (withSpace ? " " : "") + "M";

	// kilo
	else if (d >= 1e3)
		return roundStr(d * 1e-3, precision) + (withSpace ? " " : "") + "K";

	// unity
	else if (d >= 1.0 || d == 0) {
		if (std::round(d) == d) return std::to_string((uint64_t)d);
		else return roundStr(d, precision);
	}

	// milli
	else if (d >= 1e-3)
		return roundStr(d * 1e3, precision) + (withSpace ? " " : "") + "milli";

	// micro
	else if (d >= 1e-6)
		return roundStr(d * 1e6, precision) + (withSpace ? " " : "") + "micro";

	// nano
	else
		return roundStr(d * 1e9, precision) + (withSpace ? " " : "") + "nano";
}

inline std::string prettyNumber(
	const uint64_t n, const unsigned precision = 0, const bool withSpace = false)
{
	return prettyNumber((double)n, precision, withSpace);
}

} // opt

#endif // OPT_STRINGMANIP_H