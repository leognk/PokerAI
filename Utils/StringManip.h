#ifndef OPT_STRINGMANIP_H
#define OPT_STRINGMANIP_H

#include <sstream>

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

// Count the number of digits in the integral part of a positive double.
inline unsigned intPartSize(double d)
{
	unsigned size = 0;
	while (d >= 1) {
		d /= 10;
		++size;
	}
	return size;
}

inline std::string prettyNumCase(
	const double d, const unsigned precision, const bool withSpace,
	const std::string& suffix)
{
	return roundStr(d, precision) + (withSpace ? " " : "") + suffix;
}

// Output the double d in a string with a maximum of 4 characters if precision = 0
// or 5 + precision characters using G (giga), M (mega), K (kilo), milli, micro, nano.
inline std::string prettyNum(
	const double d, const unsigned precision = 0, const bool withSpace = false)
{
	// giga
	if (d >= 1e9) return prettyNumCase(d * 1e-9, precision, withSpace, "G");
	// mega
	else if (d >= 1e6) return prettyNumCase(d * 1e-6, precision, withSpace, "M");
	// kilo
	else if (d >= 1e3) return prettyNumCase(d * 1e-3, precision, withSpace, "K");
	// unity
	else if (d >= 1.0 || d == 0) {
		if (std::round(d) == d) return std::to_string((uint64_t)d);
		else return prettyNumCase(d, precision, false, "");
	}
	// milli
	else if (d >= 1e-3) return prettyNumCase(d * 1e3, precision, withSpace, "milli");
	// micro
	else if (d >= 1e-6) return prettyNumCase(d * 1e6, precision, withSpace, "micro");
	// nano
	else return prettyNumCase(d * 1e9, precision, withSpace, "nano");
}

inline std::string prettyNum(
	const uint64_t n, const unsigned precision = 0, const bool withSpace = false)
{
	return prettyNum((double)n, precision, withSpace);
}

inline std::string prettyNumDgCase(
	const double d, const unsigned nDigits, const bool withSpace,
	const std::string& suffix)
{
	const unsigned intSize = intPartSize(d);
	const unsigned precision = (nDigits > intSize) ? nDigits - intSize : 0;
	return roundStr(d, precision) + (withSpace ? " " : "") + suffix;
}

//Same as prettyNum but with a fixed number of digits (except in the case of unity).
inline std::string prettyNumDg(
	const double d, const unsigned nDigits, const bool withSpace = false)
{
	// giga
	if (d >= 1e9) return prettyNumDgCase(d * 1e-9, nDigits, withSpace, "G");
	// mega
	else if (d >= 1e6) return prettyNumDgCase(d * 1e-6, nDigits, withSpace, "M");
	// kilo
	else if (d >= 1e3) return prettyNumDgCase(d * 1e-3, nDigits, withSpace, "K");
	// unity
	else if (d >= 1.0 || d == 0) {
		if (std::round(d) == d) return std::to_string((uint64_t)d);
		else return prettyNumDgCase(d, nDigits, false, "");
	}
	// milli
	else if (d >= 1e-3) return prettyNumDgCase(d * 1e3, nDigits, withSpace, "milli");
	// micro
	else if (d >= 1e-6) return prettyNumDgCase(d * 1e6, nDigits, withSpace, "micro");
	// nano
	else return prettyNumDgCase(d * 1e9, nDigits, withSpace, "nano");
}

inline std::string prettyNumDg(
	const uint64_t n, const unsigned nDigits, const bool withSpace = false)
{
	return prettyNumDg((double)n, nDigits, withSpace);
}

} // opt

#endif // OPT_STRINGMANIP_H