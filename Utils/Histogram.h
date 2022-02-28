#ifndef OPT_HISTOGRAM_H
#define OPT_HISTOGRAM_H

#include <vector>
#include <algorithm>
#include "ioContainer.h"

namespace opt {

// Find the min and the max of a 2D vector.
template<typename T>
static std::pair<T, T> findMinMax(const std::vector<std::vector<T>>& v)
{
	T min = (std::numeric_limits<T>::max)();
	T max = (std::numeric_limits<T>::min)();
	for (const auto& w : v) {
		const auto [min0, max0] = std::minmax_element(w.begin(), w.end());
		if (*min0 < min) min = *min0;
		if (*max0 > max) max = *max0;
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
			const auto [min0, max0] = std::minmax_element(w2.begin(), w2.end());
			if (*min0 < min) min = *min0;
			if (*max0 > max) max = *max0;
		}
	}
	return { min, max };
}

// Modify xticks in-place and return the width.
template<typename T>
static T calculateXTicks(std::vector<T>& xticks, const T min, const T max)
{
	const size_t nBins = xticks.size() - 1;
	const T width = (T)std::round(((double)max - min) / nBins);

	xticks[0] = min;
	for (size_t i = 1; i < nBins; ++i)
		xticks[i] = xticks[i - 1] + width;
	xticks.back() = max;

	return width;
}

// Modify hist in-place. Its size has to be nBins.
template<typename T>
static void accHist(
	const std::vector<T>& v, std::vector<uint64_t>& hist, const T min, const T width)
{
	for (const T& x : v) {
		size_t i = (size_t)((x - min) / width);
		if (i == hist.size()) --i;
		++hist[i];
	}
}

// xticks is modified in-place. Its size has to be nBins + 1.
template<typename T>
static std::vector<uint64_t> buildHist(
	const std::vector<T>& v, std::vector<T>& xticks, const T min, const T max)
{
	const T width = calculateXTicks(xticks, min, max);
	std::vector<uint64_t> hist(xticks.size() - 1);
	accHist(v, hist, min, width);
	return hist;
}

template<typename T>
static std::vector<uint64_t> buildHist(
	const std::vector<T>& v, std::vector<T>& xticks)
{
	const auto [min, max] = std::minmax_element(v.begin(), v.end());
	return buildHist<T>(v, xticks, *min, *max);
}

template<typename T>
static std::vector<uint64_t> buildHist(
	const std::vector<std::vector<T>>& v, std::vector<T>& xticks, T min, T max)
{
	const T width = calculateXTicks(xticks, min, max);
	std::vector<uint64_t> hist(xticks.size() - 1);
	for (const auto& w : v)
		accHist(w, hist, min, width);
	return hist;
}

template<typename T>
static std::vector<uint64_t> buildHist(
	const std::vector<std::vector<T>>& v, std::vector<T>& xticks)
{
	const auto [min, max] = findMinMax(v);
	return buildHist<T>(v, xticks, min, max);
}

template<typename T>
static std::vector<uint64_t> buildHist(
	const std::vector<std::vector<std::vector<T>>>& v, std::vector<T>& xticks, T min, T max)
{
	const T width = calculateXTicks(xticks, min, max);
	std::vector<uint64_t> hist(xticks.size() - 1);
	for (const auto& w1 : v) {
		for (const auto& w2 : w1)
			accHist(w2, hist, min, width);
	}
	return hist;
}

template<typename T>
static std::vector<uint64_t> buildHist(
	const std::vector<std::vector<std::vector<T>>>& v, std::vector<T>& xticks)
{
	const auto [min, max] = findMinMax(v);
	return buildHist<T>(v, xticks, min, max);
}

template<typename T, typename V>
static void buildAndSaveHist(const V& v, const size_t nBins, std::fstream& file)
{
	std::vector<T> xticks(nBins + 1);
	const std::vector<uint64_t> hist = buildHist(v, xticks);
	save1DVector(hist, file);
	save1DVector(xticks, file);
}

template<typename T>
static void buildAndSaveHist(const std::vector<T>& v, const size_t nBins, std::fstream& file)
{
	buildAndSaveHist<T, std::vector<T>>(v, nBins, file);
}

template<typename T>
static void buildAndSaveHist(const std::vector<std::vector<T>>& v, const size_t nBins, std::fstream& file)
{
	buildAndSaveHist<T, std::vector<std::vector<T>>>(v, nBins, file);
}

template<typename T>
static void buildAndSaveHist(const std::vector<std::vector<std::vector<T>>>& v, const size_t nBins, std::fstream& file)
{
	buildAndSaveHist<T, std::vector<std::vector<std::vector<T>>>>(v, nBins, file);
}

template<typename V>
static void buildAndSaveHist(const V& v, const size_t nBins, const std::string& filePath)
{
	auto file = std::fstream(filePath, std::ios::in | std::ios::binary);
	buildAndSaveHist(v, nBins, file);
	file.close();
}

} // opt

#endif // OPT_HISTOGRAM_H