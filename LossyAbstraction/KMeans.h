#ifndef ABC_KMEANS_H
#define ABC_KMEANS_H

#include <cstdint>
#include <array>

namespace abc {

// Class implementing k-means++ with earth mover's distance
// or L2 distance.
template<typename cluster_t = uint8_t>
class KMeans
{
public:
	template<typename bin_t, uint32_t nHists, uint16_t nBins>
	static std::array<cluster_t, nHists> buildClustersEMD(
		const std::array<std::array<bin_t, nBins>, nHists>& hists,
		cluster_t nClusters, uint8_t nRestarts)
	{

	}

	template<typename coord_t, uint32_t nVects, uint8_t nCoords>
	static std::array<cluster_t, nVects> buildClustersL2(
		const std::array<std::array<coord_t, nCoords>, nVects>& vects,
		cluster_t nClusters, uint8_t nRestarts)
	{

	}

private:


}; // KMeans

} // abc

#endif // ABC_KMEANS_H