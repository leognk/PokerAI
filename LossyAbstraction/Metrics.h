#ifndef ABC_METRICS_H
#define ABC_METRICS_H

#include <array>
#include <cmath>
#include <numeric>

namespace abc {

// Earth mover's distance.
template<typename C1, typename C2>
uint16_t emd(const C1& u, const C2& v)
{
	int16_t diffAcc = 0;
	uint16_t distance = 0;
	for (uint8_t i = 0; i < u.size(); ++i) {
		diffAcc += (int16_t)u[i] - v[i];
		distance += std::abs(diffAcc);
	}
	return distance;
}

// Squared earth mover's distance multiplied by nFeatures.
template<typename C1, typename C2>
uint32_t emdSq(const C1& u, const C2& v)
{
	uint32_t distance = emd(u, v);
	return distance * distance;
}

// Reference:
// Optimal Transport and Wasserstein Distance
// Quote: "the Wasserstein barycenter which, in this case,
// can be obtained simply by averaging the order statistics"
template<typename cluSize_t, typename feature_t, uint32_t nSamples, uint8_t nFeatures>
void emdCenter(
	const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
	const std::vector<cluSize_t>& labels,
	cluSize_t label,
	uint32_t weight,
	std::vector<feature_t>& center)
{
	// Generate the average samples from the input histograms.
	feature_t sumFeatures =
		std::accumulate(data[0].begin(), data[0].end(), feature_t(0));
	std::vector<uint64_t> centerSamples(sumFeatures);
	for (uint32_t i = 0; i < nSamples; ++i) {
		if (labels[i] != label) continue;
		uint8_t k = 0;
		while (data[i][k] == 0) ++k;
		feature_t count = 0;
		for (feature_t j = 0; j < sumFeatures; ++j) {
			centerSamples[j] += k;
			if (++count == data[i][k] && k != nFeatures - 1) {
				while (data[i][++k] == 0);
				count = 0;
			}
		}
	}
	// Convert the average samples to a histogram which will be the center.
	std::memset(center.data(), 0, nFeatures * sizeof(feature_t));
	for (feature_t j = 0; j < sumFeatures; ++j) {
#pragma warning(suppress: 4244)
		uint8_t idx = std::round((double)centerSamples[j] / weight);
		++center[idx];
	}
}

// Squared euclidian distance.
template<typename C1, typename C2>
uint32_t euclidianDistanceSq(const C1& u, const C2& v)
{
	uint32_t res = 0;
	for (uint8_t i = 0; i < u.size(); ++i) {
		int32_t diff = (int32_t)u[i] - v[i];
		res += diff * diff;
	}
	return res;
}

// Euclidian distance.
template<typename C1, typename C2>
uint16_t euclidianDistance(const C1& u, const C2& v)
{
	uint32_t distSq = euclidianDistanceSq(u, v);
#pragma warning(suppress: 4244)
	return std::round(std::sqrt(distSq));
}

template<typename cluSize_t, typename feature_t, uint32_t nSamples, uint8_t nFeatures>
void euclidianCenter(
	const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
	const std::vector<cluSize_t>& labels,
	cluSize_t label,
	uint32_t weight,
	std::vector<feature_t>& center)
{
	for (feature_t k = 0; k < nFeatures; ++k) {
		uint64_t acc = 0;
		for (uint32_t i = 0; i < nSamples; ++i) {
			if (labels[i] == label)
				acc += data[i][k];
		}
#pragma warning(suppress: 4244)
		center[k] = std::round((double)acc / weight);
	}
}

} // abc

#endif // ABC_METRICS_H