#ifndef OPT_HISTOGRAM_H
#define OPT_HISTOGRAM_H

#include <vector>
#include <algorithm>
#include <boost/histogram.hpp>

namespace opt {

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

// Build the histogram from a 1D vector.
template<typename T>
static auto buildHist(
	const unsigned nBins,
	const std::vector<T>& v,
	const double min, const double max)
{
	auto hist = boost::histogram::make_histogram(
		boost::histogram::axis::regular(nBins, min, max));
	hist.fill(v);
	return hist;
}

// Build the histogram from a 2D vector.
template<typename T>
static auto buildHist(
	const unsigned nBins,
	const std::vector<std::vector<T>>& v,
	const double min, const double max)
{
	auto hist = boost::histogram::make_histogram(
		boost::histogram::axis::regular(nBins, min, max));
	for (const auto& w : v)
		hist.fill(w);
	return hist;
}

// Build the histogram from a 3D vector.
template<typename T>
static auto buildHist(
	const unsigned nBins,
	const std::vector<std::vector<std::vector<T>>>& v,
	const double min, const double max)
{
	auto hist = boost::histogram::make_histogram(
		boost::histogram::axis::regular(nBins, min, max));
	for (const auto& w1 : v) {
		for (const auto& w2 : w1)
			hist.fill(w2);
	}
	return hist;
}

template<typename V>
static void buildAndSaveHist(const unsigned nBins, const V& v, const double min, const double max, std::fstream& file)
{
	const auto hist = buildHist(nBins, v, min, max);

	// Save the xticks.
	for (auto&& x : boost::histogram::indexed(hist)) {
		const double xtick = x.bin().lower();
		opt::saveVar(xtick, file);
	}
	opt::saveVar(max, file);

	// Save the heights.
	for (auto&& x : boost::histogram::indexed(hist)) {
		const double h = *x;
		opt::saveVar(h, file);
	}
}

template<typename V>
static void buildAndSaveHist(const unsigned nBins, const V& v, std::fstream& file)
{
	const auto [min, max] = findMinMax(v);
	buildAndSaveHist(nBins, v, (double)min, (double)max, file);
}

} // opt

#endif // OPT_HISTOGRAM_H