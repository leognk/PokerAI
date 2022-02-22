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

inline double round(double value, unsigned precision)
{
	const double multiplier = std::pow(10.0, precision);
	return std::round(value * multiplier) / multiplier;
}

// Output the double d in a string with a maximum of 4 characters if precision = 0
// or 5 + precision characters using G (giga), M (mega), K (kilo), milli, micro, nano.
inline std::string prettyNumber(double d, unsigned precision = 0, bool withSpace = false)
{
	std::ostringstream os;
	os.precision(precision);
	os << std::fixed;

	// giga
	if (d >= 1e9) {
		os << round(d * 1e-9, precision);
		if (withSpace) os << " ";
		os << "G";
		return os.str();
	}

	// mega
	else if (d >= 1e6) {
		os << round(d * 1e-6, precision);
		if (withSpace) os << " ";
		os << "M";
		return os.str();
	}

	// kilo
	else if (d >= 1e3) {
		os << round(d * 1e-3, precision);
		if (withSpace) os << " ";
		os << "K";
		return os.str();
	}

	// unity
	else if (d >= 1.0 || d == 0) {
		if (std::round(d) == d) os << (uint64_t)d;
		else os << round(d, precision);
		return os.str();
	}

	// milli
	else if (d >= 1e-3) {
		os << round(d * 1e3, precision);
		if (withSpace) os << " ";
		os << "milli";
		return os.str();
	}

	// micro
	else if (d >= 1e-6) {
		os << round(d * 1e6, precision);
		if (withSpace) os << " ";
		os << "micro";
		return os.str();
	}

	// nano
	else {
		os << round(d * 1e9, precision);
		if (withSpace) os << " ";
		os << "nano";
		return os.str();
	}
}

inline std::string prettyNumber(uint64_t n, unsigned precision = 0, bool withSpace = false)
{
	return prettyNumber((double)n, precision, withSpace);
}

} // opt

#endif // OPT_STRINGMANIP_H