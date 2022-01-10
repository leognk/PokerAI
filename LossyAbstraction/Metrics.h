#ifndef ABC_METRICS_H
#define ABC_METRICS_H

#include <array>
#include <algorithm>
#include <cmath>

namespace abc {

// Earth mover's distance multiplied by nFeatures.
template<typename feature_t, uint8_t nFeatures>
uint64_t emd(
	const std::array<feature_t, nFeatures>& u,
	const std::array<feature_t, nFeatures>& v)
{
	// Sort u and v.
	std::array<feature_t, nFeatures> uSorted = u, vSorted = v;
	std::sort(uSorted.begin(), uSorted.end());
	std::sort(vSorted.begin(), vSorted.end());

	// Concatenate u and v into a sorted array.
	std::array<feature_t, 2 * nFeatures> allValues;
	std::merge(uSorted.begin(), uSorted.end(),
		vSorted.begin(), vSorted.end(), allValues.begin());

	// Compute the adjacent differences of allValues.
	std::array<feature_t, 2 * nFeatures - 1> deltas;
	for (uint8_t i = 0; i < deltas.size(); ++i)
#pragma warning(suppress: 26451)
		deltas[i] = allValues[i + 1] - allValues[i];

	// Get the respective positions of the values of u and v among the values of
	// both distributions.
	std::array<uint8_t, 2 * nFeatures - 1> uCdfIndices, vCdfIndices;
	// Do a np.searchSorted with allValues on uSorted and vSorted.
	for (uint8_t i = 0; i < uCdfIndices.size(); ++i) {
		uint8_t j = 0;
		while (uSorted[j] <= allValues[i])
			if (++j == nFeatures) break;
		uCdfIndices[i] = j;
		j = 0;
		while (vSorted[j] <= allValues[i])
			if (++j == nFeatures) break;
		vCdfIndices[i] = j;
	}

	// Compute the value of the integral based on the CDFs
	// (modulo a nFeatures factor).
	uint64_t distance = 0;
	for (uint8_t i = 0; i < uCdfIndices.size(); ++i) {
		uint64_t diff;
		if (uCdfIndices[i] < vCdfIndices[i])
			diff = vCdfIndices[i] - uCdfIndices[i];
		else
			diff = uCdfIndices[i] - vCdfIndices[i];
		distance += diff * deltas[i];
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

// Squared euclidian distance.
template<typename feature_t, uint8_t nFeatures>
uint64_t euclidianDistanceSq(
	const std::array<feature_t, nFeatures>& u,
	const std::array<feature_t, nFeatures>& v)
{
	uint64_t res = 0;
	for (uint8_t i = 0; i < nFeatures; ++i) {
		feature_t diff;
		if (u[i] < v[i])
			diff = v[i] - u[i];
		else
			diff = u[i] - v[i];
		res += (uint64_t)diff * diff;
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