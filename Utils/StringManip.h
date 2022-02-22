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

// Output the integer n in a string with max 4 characters if precision = 0
// or else 5 + precision characters by using the letters
// K for kilo, M for mega, and G for giga.
inline std::string prettyBigNum(uint64_t n, uint8_t precision = 0, bool withSpace = false)
{
	// Giga
	if (n >= uint64_t(1e9)) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(n / 1e9, precision);
		if (withSpace) os << " ";
		os << "G";
		return os.str();
	}

	// Mega
	else if (n >= uint64_t(1e6)) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(n / 1e6, precision);
		if (withSpace) os << " ";
		os << "M";
		return os.str();
	}

	// Kilo
	else if (n >= uint64_t(1e3)) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(n / 1e3, precision);
		if (withSpace) os << " ";
		os << "K";
		return os.str();
	}

	else
		return std::to_string(n);
}

inline std::string prettyBigNum(double d, uint8_t precision = 0, bool withSpace = false)
{
	return prettyBigNum((uint64_t)std::round(d), precision, withSpace);
}

// Output the double d in a string with max 4 characters if precision = 0
// or else 5 + precision characters by using milli, micro, and nano.
inline std::string prettySmallNum(double d, uint8_t precision = 0, bool withSpace = false)
{
	// Nano
	if (d < 1e-6) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(d * 1e9, precision);
		if (withSpace) os << " ";
		os << "nano";
		return os.str();
	}

	// Micro
	else if (d < 1e-3) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(d * 1e6, precision);
		if (withSpace) os << " ";
		os << "micro";
		return os.str();
	}

	// Milli
	else if (d < 1.0) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(d * 1e3, precision);
		if (withSpace) os << " ";
		os << "milli";
		return os.str();
	}

	else {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(d, precision);
		return os.str();
	}
}

} // opt

#endif // OPT_STRINGMANIP_H