#ifndef OPT_HISTOGRAM_H
#define OPT_HISTOGRAM_H

#include <vector>
#include <algorithm>
#include <boost/histogram.hpp>

namespace opt {

static const double base = 10.0;
static const double linthresh = 2.0;
static const double linscale = 1.0;
static const double linscaleAdj = linscale / (1.0 - 1.0 / base);
static const double invlinthresh = linthresh * linscaleAdj;

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

// SymLog transform for equidistant bins in log-space.
struct symlog {

	template <class T>
	static T forward(T x) {
		const T absX = std::abs(x);
		if (absX <= linthresh)
			return x * linscaleAdj;
		else
			return sign(x) * linthresh * (linscaleAdj + std::log10(absX / linthresh));
	}

	template <class T>
	static T inverse(T x) {
		const T absX = std::abs(x);
		if (absX <= invlinthresh)
			return x / linscaleAdj;
		else
			return sign(x) * linthresh * std::pow(base, absX / linthresh - linscaleAdj);
	}
};

// Find the min and the max of a 1D vector.
template<typename T>
static std::pair<T, T> findMinMax(const std::vector<T>& v)
{
	const auto [min, max] = std::minmax_element(v.begin(), v.end());
	return { *min, *max };
}

// Find the min and the max of a 2D vector.
template<typename T>
static std::pair<T, T> findMinMax(const std::vector<std::vector<T>>& v)
{
	T min = (std::numeric_limits<T>::max)();
	T max = (std::numeric_limits<T>::min)();
	for (const auto& w : v) {
		const auto [min0, max0] = findMinMax(w);
		if (min0 < min) min = min0;
		if (max0 > max) max = max0;
	}
	return { min, max };
}

// Find the min and the max of a 3D vector.
template<typename T>
static std::pair<T, T> findMinMax(const std::vector<std::vector<std::vector<T>>>& v)
{
	T min = (std::numeric_limits<T>::max)();
	T max = (std::numeric_limits<T>::min)();
	for (const auto& w1 : v) {
		for (const auto& w2 : w1) {
			const auto [min0, max0] = findMinMax(w2);
			if (min0 < min) min = min0;
			if (max0 > max) max = max0;
		}
	}
	return { min, max };
}

// Fill the histogram with a 1D vector.
template<class H, typename T>
static void fillHist(H& hist, const std::vector<T>& v)
{
	hist.fill(v);
}

// Fill the histogram with a 2D vector.
template<class H, typename T>
static void fillHist(H& hist, const std::vector<std::vector<T>>& v)
{
	for (const auto& w : v)
		hist.fill(w);
}

// Fill the histogram with a 3D vector.
template<class H, typename T>
static void fillHist(H& hist, const std::vector<std::vector<std::vector<T>>>& v)
{
	for (const auto& w1 : v) {
		for (const auto& w2 : w1)
			hist.fill(w2);
	}
}

template<typename V, class Axis>
static void buildAndSaveHist(const unsigned nBins, const V& v, std::fstream& file)
{
	// Build the histogram.
	auto [min, max] = findMinMax(v);
	if (max == min) ++max;
	auto hist = boost::histogram::make_histogram(Axis(nBins, min, max));
	fillHist(hist, v);

	// Save the xticks.
	for (auto&& x : boost::histogram::indexed(hist)) {
		const double xtick = x.bin().lower();
		opt::saveVar(xtick, file);
		// Save the last xtick.
		if (x.index() == nBins - 1) {
			const double xtick = x.bin().upper();
			opt::saveVar(xtick, file);
		}
	}

	// Save the heights.
	for (auto&& x : boost::histogram::indexed(hist)) {
		const double h = *x;
		opt::saveVar(h, file);
	}
}

template<typename V>
static void buildAndSaveHist(const unsigned nBins, const V& v, std::fstream& file,
	const std::string& scale)
{
	if (scale == "linear")
		buildAndSaveHist<V, boost::histogram::axis::regular<>>(nBins, v, file);
	else if (scale == "symlog")
		buildAndSaveHist<V, boost::histogram::axis::regular<double, symlog>>(nBins, v, file);
	else
		throw std::runtime_error("Unknown scale option.");
}

} // opt

#endif // OPT_HISTOGRAM_H