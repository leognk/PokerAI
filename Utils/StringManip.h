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
// or 5 + precision characters using G (giga), M (mega), k (kilo), m (milli), u (micro), n (nano).
inline std::string prettyNum(
	const double d, const unsigned precision = 0, const bool withSpace = false)
{
	// giga
	if (d >= 1e9) return prettyNumCase(d * 1e-9, precision, withSpace, "G");
	// mega
	else if (d >= 1e6) return prettyNumCase(d * 1e-6, precision, withSpace, "M");
	// kilo
	else if (d >= 1e3) return prettyNumCase(d * 1e-3, precision, withSpace, "k");
	// unity
	else if (d >= 1.0 || d == 0) {
		if (std::round(d) == d) return prettyNumCase(d, 0, withSpace, "");
		else return prettyNumCase(d, precision, withSpace, "");
	}
	// milli
	else if (d >= 1e-3) return prettyNumCase(d * 1e3, precision, withSpace, "m");
	// micro
	else if (d >= 1e-6) return prettyNumCase(d * 1e6, precision, withSpace, "u");
	// nano
	else return prettyNumCase(d * 1e9, precision, withSpace, "n");
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
	else if (d >= 1e3) return prettyNumDgCase(d * 1e-3, nDigits, withSpace, "k");
	// unity
	else if (d >= 1.0 || d == 0) {
		if (std::round(d) == d) return std::to_string((uint64_t)d) + (withSpace ? " " : "");
		return prettyNumDgCase(d, nDigits, withSpace, "");
	}
	// milli
	else if (d >= 1e-3) return prettyNumDgCase(d * 1e3, nDigits, withSpace, "m");
	// micro
	else if (d >= 1e-6) return prettyNumDgCase(d * 1e6, nDigits, withSpace, "u");
	// nano
	else return prettyNumDgCase(d * 1e9, nDigits, withSpace, "n");
}

inline std::string prettyNumDg(
	const uint64_t n, const unsigned nDigits, const bool withSpace = false)
{
	return prettyNumDg((double)n, nDigits, withSpace);
}

inline std::string prettyNumDg(
	const int64_t n, const unsigned nDigits, const bool withSpace = false)
{
	return ((n < 0) ? "-" : "") + prettyNumDg((double)std::abs(n), nDigits, withSpace);
}

template<typename T>
inline std::string prettyPerc(
	const T x, const T total,
	const unsigned precision = 0, const bool withSpace = false)
{
	return roundStr(100.0 * x / total, precision) + (withSpace ? " " : "") + "%";
}

} // opt

#endif // OPT_STRINGMANIP_H