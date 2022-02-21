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

// Output the integer n in a string with max 4 if precision = 0
// or else 5 + precision characters by using the letters
// K for kilo, M for mega, and G for giga.
inline std::string prettyBigNum(uint64_t n, uint8_t precision = 0)
{
	// Giga
	if (n >= uint64_t(1e9)) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(n / 1e9, precision) << "G";
		return os.str();
	}

	// Mega
	else if (n >= uint64_t(1e6)) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(n / 1e6, precision) << "M";
		return os.str();
	}

	// Kilo
	else if (n >= uint64_t(1e3)) {
		std::ostringstream os;
		os.precision(precision);
		os << std::fixed << round(n / 1e3, precision) << "K";
		return os.str();
	}

	else
		return std::to_string(n);
}

} // opt

#endif // OPT_STRINGMANIP_H