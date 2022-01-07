#ifndef ABC_KMEANS_H
#define ABC_KMEANS_H

#include <cstdint>
#include <array>
#include <random>

namespace abc {

// Class implementing k-means++ with earth mover's distance
// or L2 distance.
template<typename cluSize_t = uint8_t, cluSize_t nClusters = 200>
class KMeans
{
public:
	// Set rngSeed to 0 to set a random seed.
	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	static std::array<cluSize_t, nSamples> buildClusters(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& vects,
		bool useEMD, uint8_t nRestarts, unsigned rngSeed = 0)
	{
		std::mt19937 rng((!rngSeed) ? std::random_device{}() : rngSeed);
		std::array<std::array<feature_t, nFeatures>, nClusters> centroids =
			kmeanspp(vects);
	}

	template<typename feature_t, uint32_t nSamples, uint16_t nFeatures>
	std::array<std::array<feature_t, nFeatures>, nClusters> kmeanspp(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& vects)
	{
		std::array<std::array<feature_t, nFeatures>, nClusters> centroids;

	}

private:


}; // KMeans

} // abc

#endif // ABC_KMEANS_H