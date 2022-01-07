#ifndef ABC_KMEANS_H
#define ABC_KMEANS_H

#include <array>
#include <random>
#include "../Utils/Random.h"

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
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		bool useEMD, uint8_t nRestarts, unsigned rngSeed = 0)
	{
		std::array<std::array<feature_t, nFeatures>, nClusters> centroids =
			kMeansPlusPlus(data, rngSeed);
	}

	template<typename feature_t, uint32_t nSamples, uint16_t nFeatures>
	std::array<std::array<feature_t, nFeatures>, nClusters> kMeansPlusPlus(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		unsigned rngSeed)
	{
		std::array<std::array<feature_t, nFeatures>, nClusters> centroids;
		opt::XoShiro128PlusPlus rng(
			(!rngSeed) ? std::random_device{}() : rngSeed);
		std::uniform_int_distribution<uint32_t> dist(0, nSamples - 1);
		uint32_t randIdx = dist(rng);
		centroids[0] = data[randIdx];
		cluSize_t nChosenCentroids = 1;
	}

private:


}; // KMeans

} // abc

#endif // ABC_KMEANS_H