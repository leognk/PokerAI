#ifndef ABC_METRICS_H
#define ABC_METRICS_H

#include <array>
#include <algorithm>
#include <cmath>

namespace abc {

//// Earth mover's distance multiplied by nFeatures.
//template<typename feature_t, uint8_t nFeatures>
//uint64_t emd(
//	const std::array<feature_t, nFeatures>& u,
//	const std::array<feature_t, nFeatures>& v)
//{
//	// Sort u and v.
//	std::array<feature_t, nFeatures> uSorted = u, vSorted = v;
//	std::sort(uSorted.begin(), uSorted.end());
//	std::sort(vSorted.begin(), vSorted.end());
//
//	// Concatenate u and v into a sorted array.
//	std::array<feature_t, 2 * nFeatures> allValues;
//	std::merge(uSorted.begin(), uSorted.end(),
//		vSorted.begin(), vSorted.end(), allValues.begin());
//
//	// Compute the adjacent differences of allValues.
//	std::array<feature_t, 2 * nFeatures - 1> deltas;
//	for (uint8_t i = 0; i < deltas.size(); ++i)
//#pragma warning(suppress: 26451)
//		deltas[i] = allValues[i + 1] - allValues[i];
//
//	// Get the respective positions of the values of u and v among the values of
//	// both distributions.
//	std::array<uint8_t, 2 * nFeatures - 1> uCdfIndices, vCdfIndices;
//	// Do a np.searchSorted with allValues on uSorted and vSorted.
//	for (uint8_t i = 0; i < uCdfIndices.size(); ++i) {
//		uint8_t j = 0;
//		while (uSorted[j] <= allValues[i])
//			if (++j == nFeatures) break;
//		uCdfIndices[i] = j;
//		j = 0;
//		while (vSorted[j] <= allValues[i])
//			if (++j == nFeatures) break;
//		vCdfIndices[i] = j;
//	}
//
//	// Compute the value of the integral based on the CDFs
//	// (modulo a nFeatures factor).
//	uint64_t distance = 0;
//	for (uint8_t i = 0; i < uCdfIndices.size(); ++i) {
//		uint64_t diff;
//		if (uCdfIndices[i] < vCdfIndices[i])
//			diff = vCdfIndices[i] - uCdfIndices[i];
//		else
//			diff = uCdfIndices[i] - vCdfIndices[i];
//		distance += diff * deltas[i];
//	}
//	return distance;
//}

// Earth mover's distance.
template<typename feature_t, uint8_t nFeatures>
uint64_t emd(
	const std::array<feature_t, nFeatures>& u,
	const std::array<feature_t, nFeatures>& v)
{
	int64_t diffAcc = 0;
	uint64_t distance = 0;
	for (uint8_t i = 0; i < nFeatures; ++i) {
		diffAcc += u[i] - v[i];
		distance += std::abs(diffAcc);
	}
	return distance;
}

// Squared earth mover's distance multiplied by nFeatures.
template<typename feature_t, uint8_t nFeatures>
uint64_t emdSq(
	const std::array<feature_t, nFeatures>& u,
	const std::array<feature_t, nFeatures>& v)
{
	uint64_t distance = emd(u, v);
	return distance * distance;
}

// Reference:
// Optimal Transport and Wasserstein Distance
// 4 - Barycenters and PCA
// https://www.stat.cmu.edu/~larry/=sml/Opt.pdf
// Quote: "In a sense, all we are really doing is
// converting to quantiles and averaging."
// So we average the quantiles of the data points and convert it to
// a distribution (histogram) which will be the barycenter.
//template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
//void emdCenter(
//	const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
//	std::array<feature_t, nFeatures>& center, feature_t sumFeatures)
//{
//	std::memset(center.data(), 0, nFeatures * sizeof(feature_t));
//	std::vector<uint64_t> centerQuantile(sumFeatures, 0);
//	for (uint32_t i = 0; i < nSamples; ++i) {
//		uint64_t cumul = 0;
//		for (uint8_t k = 0; k < nFeatures; ++k) {
//			if (data[i][k] != 0) {
//				cumul += data[i][k];
//#pragma warning(suppress: 26451)
//				centerQuantile[cumul - 1] += k + 1;
//			}
//		}
//	}
//	for (feature_t i = 0; i < sumFeatures; ++i) {
//#pragma warning(suppress: 4244)
//		uint8_t idx = std::round((double)centerQuantile[i] / nSamples);
//		if (idx != 0) center[idx - 1] = i + 1;
//	}
//}

// Reference:
// Optimal Transport and Wasserstein Distance
// 4 - Barycenters and PCA
// https://www.stat.cmu.edu/~larry/=sml/Opt.pdf
// Quote: "the Wasserstein barycenter which, in this case,
// can be obtained simply by averaging the order statistics"
template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
void emdCenter(
	const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
	std::array<feature_t, nFeatures>& center)
{
	std::memset(center.data(), 0, nFeatures * sizeof(feature_t));
	// Generate the average samples from the input histograms.
	feature_t sumFeatures =
		std::accumulate(data[0].begin(), data[0].end(), feature_t(0));
	std::vector<uint64_t> centerSamples(sumFeatures);
	for (uint32_t i = 0; i < nSamples; ++i) {
		uint8_t k = 0;
		while (data[i][k] == 0) ++k;
		feature_t count = 0;
		for (feature_t j = 0; j < sumFeatures; ++j) {
			centerSamples[j] += k;
			if (++count == data[i][k]) {
				++k;
				count = 0;
			}
		}
	}
	// Convert the average samples to a histogram which will be the center.
	for (feature_t j = 0; j < sumFeatures; ++j) {
#pragma warning(suppress: 4244)
		uint8_t idx = std::round((double)centerSamples[j] / nSamples);
		++center[idx];
	}
}

// Squared euclidian distance.
template<typename feature_t, uint8_t nFeatures>
uint64_t euclidianDistanceSq(
	const std::array<feature_t, nFeatures>& u,
	const std::array<feature_t, nFeatures>& v)
{
	uint64_t res = 0;
	for (uint8_t i = 0; i < nFeatures; ++i) {
		int64_t diff = u[i] - v[i];
		res += diff * diff;
	}
	return res;
}

// Euclidian distance.
template<typename feature_t, uint8_t nFeatures>
uint64_t euclidianDistance(
	const std::array<feature_t, nFeatures>& u,
	const std::array<feature_t, nFeatures>& v)
{
	uint64_t distSq = euclidianDistanceSq(u, v);
#pragma warning(suppress: 4244)
	return std::round(std::sqrt(distSq));
}

} // abc

#endif // ABC_METRICS_H