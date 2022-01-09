#ifndef ABC_KMEANS_H
#define ABC_KMEANS_H

#include <random>
#include <cstring>
#include <set>
#include "Metrics.h"
#include "../Utils/Random.h"
#include "../cpptqdm/tqdm.h"

namespace abc {

// Class implementing k-means++ with earth mover's distance
// or Euclidian distance.
template<typename cluSize_t, cluSize_t nClusters>
class KMeans
{
public:

	// Set rngSeed to 0 to set a random seed.
	KMeans(bool useEMD, uint16_t nRestarts,
		uint16_t maxIter, unsigned rngSeed = 0) :
		useEMD(useEMD),
		nRestarts(nRestarts),
		maxIter(maxIter),
		rngSeed(rngSeed)
	{}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	std::array<cluSize_t, nSamples> buildClusters(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data)
	{
		Rng rng((!rngSeed) ? std::random_device{}() : rngSeed);

		uint64_t bestInertia = 0;
		std::array<cluSize_t, nSamples> bestLabels;

		std::array<std::array<feature_t, nFeatures>, nClusters> centers;
		std::array<cluSize_t, nSamples> labels;

		tqdm bar;
		for (uint16_t i = 0; i < nRestarts; ++i) {
			bar.progress(i, nRestarts);

			// Initialize centers.
			kMeansPlusPlus(data, centers, rng);

			// Run k-means Elkan once.
			kMeansSingleElkan(data, centers, labels);

			// Update the best clustering by looking at the inertia.
			uint64_t inertia = calculateInertia(data, centers, labels);
			std::cout << "Restart: " << nRestarts
				<< " | Inertia: " << inertia << "\n";
			if (i == 0 || inertia < bestInertia) {
				bestInertia = inertia;
				bestLabels = labels;
			}
		}

		// Count number of distinct clusters found.
		cluSize_t nDistinctClusters = std::set(
			bestLabels.begin(), bestLabels.end()).size();
		if (nDistinctClusters != nClusters)
			throw std::runtime_error(
				"Number of distinct clusters found smaller than n_clusters.");

		return bestLabels;
	}

private:

	typedef opt::XoShiro128PlusPlus Rng;

	// Initialize the centers with k-means++.
	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void kMeansPlusPlus(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		std::array<std::array<feature_t, nFeatures>, nClusters>& centers,
		Rng& rng)
	{
		// Choose one center uniformly at random among the data.
		std::uniform_int_distribution<uint32_t> distr(0, nSamples - 1);
		centers[0] = data[distr(rng)];

		// Initialize the squared distances between each data point
		// and the nearest center that has already been chosen.
		std::array<uint64_t, nSamples> minSqDists;
		for (uint32_t i = 0; i < nSamples; ++i) {
			minSqDists[i] = calculateDistSq(data[i], centers[0]);
		}

		// Choose the remaining nClusters - 1 centers
		// among the data points.
		opt::FastRandomChoice<63> randChoice;
		std::array<uint64_t, nSamples> cumWeights;
		for (cluSize_t c = 1; c < nClusters; ++c) {

			// Build the cumulated weights of the data points,
			// each weight being proportional to minSqDists.
			cumWeights[0] = minSqDists[0];
			for (uint32_t i = 1; i < nSamples; ++i)
				cumWeights[i] = cumWeights[i - 1] + minSqDists[i];
			randChoice.rescaleCumWeights(cumWeights);

			// Choose a data point with cumWeights
			// and assign it to a new center.
			centers[c] = data[randChoice(cumWeights, rng)];

			// Update minSqDists.
			for (uint32_t i = 0; i < nSamples; ++i) {
				// Skip if point i has already been chosen as a center.
				if (minSqDists[i] == 0) continue;
				uint64_t newSqDist = calculateDistSq(data[i], centers[c]);
				if (newSqDist < minSqDists[i])
					minSqDists[i] = newSqDist;
			}
		}
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void kMeansSingleElkan(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		std::array<std::array<feature_t, nFeatures>, nClusters>& centers,
		std::array<cluSize_t, nSamples>& labels)
	{
		std::array<std::array<feature_t, nFeatures>, nClusters> newCenters;
		std::array<uint32_t, nClusters> weightInClusters;
		std::array<cluSize_t, nSamples> oldLabels = labels;

		// Init the matrix of the half of the distance between any 2 clusters centers.
		std::array<std::array<uint64_t, nClusters>, nClusters> centerHalfDists;
		// Init the array of the half of the distance between each center
		// and its closest center.
		std::array<uint64_t, nClusters> distNextCenter;
		updateCenterDists(centers, centerHalfDists, distNextCenter);

		// Init the matrix of the upper bound on the distance between
		// each sample and its closest cluster center.
		std::array<uint64_t, nSamples> upperBounds;
		// Init the matrix of the lower bound on the distance between
		// each sample and each cluster center.
		std::array<std::array<uint64_t, nClusters>, nSamples> lowerBounds;
		initBounds(data, centers, centerHalfDists, labels, upperBounds, lowerBounds);

		// Proceed to the iterations of k-means.
		bool strictConvergence = false;
		tqdm bar;
		for (uint16_t i = 0; i < maxIter; ++i) {
			bar.progress(i, maxIter);

			elkanIter(
				data, centers, newCenters, weightInClusters,
				centerHalfDists, distNextCenter,
				upperBounds, lowerBounds, labels);
			updateCenterDists(newCenters, centerHalfDists, distNextCenter);
			std::swap(centers, newCenters);
			if (labels == oldLabels) {
				strictConvergence = true;
				break;
			}
			oldLabels = labels;
		}

		if (!strictConvergence)
			// Run one last step so that predicted labels match cluster centers.
			elkanIter(
				data, centers, newCenters, weightInClusters,
				centerHalfDists, distNextCenter,
				upperBounds, lowerBounds, labels);
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void updateCenterDists(
		const std::array<std::array<feature_t, nFeatures>, nClusters>& centers,
		std::array<std::array<uint64_t, nClusters>, nSamples>& centerHalfDists,
		std::array<uint64_t, nClusters>& distNextCenter)
	{
		// Update centerHalfDists.
		for (cluSize_t c1 = 1; c1 < nClusters; ++c1) {
			for (cluSize_t c2 = 0; c2 < c1; ++c2) {
				centerHalfDists[c1][c2] = calculateDist(centers[c1], centers[c2]) / 2;
				centerHalfDists[c2][c1] = centerHalfDists[c1][c2];
			}
		}
		// Update distNextCenter.
		for (cluSize_t c1 = 0; c1 < nClusters; ++c1) {
			uint64_t minDist = 0;
			for (cluSize_t c2 = 0; c2 < nClusters; ++c2) {
				if (c2 == c1) continue;
				if (minDist == 0 || centerHalfDists[c1][c2] < minDist)
					minDist = centerHalfDists[c1][c2];
			}
			distNextCenter[c1] = minDist;
		}
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void initBounds(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		const std::array<std::array<feature_t, nFeatures>, nClusters>& centers,
		const std::array<std::array<uint64_t, nClusters>, nSamples>& centerHalfDists,
		std::array<cluSize_t, nSamples>& labels,
		std::array<uint64_t, nSamples>& upperBounds,
		std::array<std::array<uint64_t, nClusters>, nSamples>& lowerBounds)
	{
		for (uin32_t i = 0; i < nSamples; ++i) {
			cluSize_t bestCluster = 0;
			uint64_t minDist = calculateDist(data[i], centers[0]);
			lowerBounds[i][0] = minDist;
			for (cluSize_t j = 1; j < nClusters; ++j) {
				if (centerHalfDists[j][bestCluster] < minDist) {
					uint64_t dist = calculateDist(data[i], centers[j]);
					lowerBounds[i][j] = dist;
					if (dist < minDist) {
						minDist = dist;
						bestCluster = j;
					}
				}
			}
			labels[i] = bestCluster;
			upperBounds[i] = minDist;
		}
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void elkanIter(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		const std::array<std::array<feature_t, nFeatures>, nClusters>& centers,
		std::array<std::array<feature_t, nFeatures>, nClusters>& newCenters,
		std::array<uint32_t, nClusters>& weightInClusters,
		const std::array<std::array<uint64_t, nClusters>, nSamples>& centerHalfDists,
		const std::array<uint64_t, nClusters>& distNextCenter,
		std::array<uint64_t, nSamples>& upperBounds,
		std::array<std::array<uint64_t, nClusters>, nSamples>& lowerBounds,
		std::array<cluSize_t, nSamples>& labels)
	{
		std::memset(newCenters.data(), 0, nClusters * nFeatures * sizeof(feature_t));
		std::memset(weightInClusters.data(), 0, nClusters * sizeof(uint32_t));

		for (uint32_t i = 0; i < nSamples; ++i) {
			uint64_t upperBound = upperBounds[i];
			bool boundsTight = false;
			cluSize_t label = labels[i];

			// Next center is not far away from the currently assigned center.
			// Sample might need to be assigned to another center.
			if (distNextCenter[label] < upperBound) {

				for (cluSize_t j = 0; j < nClusters; ++j) {

					// If this holds, then j is a good candidate for the
					// sample to be relabelled, and we need to confirm this by
					// recomputing the upper and lower bounds.
					if (j != label
						&& (upperBound > lowerBounds[i, j])
						&& (upperBound > centerHalfDistances[label, j])) {

						// Recompute upper bound by calculating the actual distance
						// between the sample and its current assigned center.
						if (!boundsTight) {
							upperBound = calculateDist(data[i], centers[label]);
							lowerBounds[i, label] = upperBound;
							boundsTight = true;
						}

						// If the condition still holds, then compute the actual
						// distance between the sample and center. If this is less
						// than the previous distance, reassign label.
						if (upperBound > lowerBounds[i, j]
							|| (upperBound > centerHalfDists[label, j])) {

							uint64_t dist = calculateDist(data[i], centers[j]);
							lowerBounds[i, j] = dist;
							if (dist < upperBounds) {
								label = j;
								upperBound = dist;
							}
						}
					}
				}
				labels[i] = label;
				upperBounds[i] = upperBound;
			}
			// Update centers.
			++weightInClusters[label];
			for (uint8_t k = 0; k < nFeatures; ++k)
				newCenters[label][k] += data[i][k];
		}

		// Update centers.
		relocateEmptyClusters(data, centers, newCenters, weightInClusters, labels);
		// Average centers.
		for (cluSize_t j = 0; j < nClusters; ++j) {
			for (uint8_t k = 0; k < nFeatures; ++k)
				newCenters[j][k] /= weightInClusters[j];
		}
		// Center shift.
		std::array<uint64_t, nClusters> centerShift;
		for (cluSize_t j = 0; j < nClusters; ++j)
			centerShift[j] = calculateDist(newCenters[j], centers[j]);

		// Update lower and upper bounds.
		for (uint32_t i = 0; i < nSamples; ++i) {
			upperBounds[i] += centerShift[labels[i]];
			for (cluSize_t j = 0; j < nClusters; ++j) {
				if (lowerBounds[i][j] > centerShift[j])
					lowerBounds[i][j] -= centerShift[j];
				else
					lowerBounds[i][j] = 0;
			}
		}
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void relocateEmptyClusters(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		const std::array<std::array<feature_t, nFeatures>, nClusters>& centers,
		std::array<std::array<feature_t, nFeatures>, nClusters>& newCenters,
		std::array<uint32_t, nClusters>& weightInClusters,
		std::array<cluSize_t, nSamples>& labels)
	{
		// Look for empty clusters.
		std::vector<cluSize_t> emptyClusters;
		for (cluSize_t j = 0; j < nClusters; ++j) {
			if (weightInClusters[j] == 0)
				emptyClusters.push_back(j);
		}
		cluSize_t nEmpty = emptyClusters.size();
		if (nEmpty == 0) return;

		// Find the nEmpty farthest data points from their centers.
		std::array<uint64_t, nSamples> dists;
		for (uint32_t i = 0; i < nSamples; ++i)
			dists[i] = calculateDist(data[i], centers[labels[i]]);
		std::vector<uint32_t> farFromCenters(nSamples);
		std::iota(farFromCenters.begin(), farFromCenters.end(), uint32_t(0));
		std::nth_element(
			farFromCenters.begin(),
			farFromCenters.begin() + nEmpty,
			farFromCenters.end(),
			[&dists](uint32_t i, uint32_t j) { return dists[i] > dists[j]; });
		farFromCenters.erase(farFromCenters.begin() + nEmpty, farFromCenters.end());

		// Relocate empty clusters to the data points far from their centers.
		for (cluSize_t j = 0; j < nEmpty; ++j) {

			cluSize_t newClusterId = emptyClusters[j];
			uint32_t farIdx = farFromCenters[j];
			cluSize_t oldClusterId = labels[farIdx];

			for (uint8_t k = 0; k < nFeatures; ++k) {
				newCenters[oldClusterId][k] -= data[farIdx][k];
				newCenters[newClusterId][k] = data[farIdx][k];
			}
			weightInClusters[newClusterId] = 1;
			--weightInClusters[oldClusterId];
			labels[farIdx] = newClusterId;
		}
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	uint64_t calculateInertia(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		const std::array<std::array<feature_t, nFeatures>, nClusters>& centers,
		const std::array<cluSize_t, nSamples>& labels)
	{
		uint64_t inertia = 0;
		for (uint32_t i = 0; i < nSamples; ++i)
			inertia += calculateDistSq(data[i], centers[labels[i]]);
		return inertia;
	}

	template<typename feature_t, uint8_t nFeatures>
	uint64_t calculateDist(
		const std::array<feature_t, nFeatures>& u,
		const std::array<feature_t, nFeatures>& v)
	{
		if (useEMD) return emd(u, v);
		else return euclidianDistance(u, v);
	}

	template<typename feature_t, uint8_t nFeatures>
	uint64_t calculateDistSq(
		const std::array<feature_t, nFeatures>& u,
		const std::array<feature_t, nFeatures>& v)
	{
		if (useEMD) return emdSq(u, v);
		else return euclidianDistanceSq(u, v);
	}

	bool useEMD;
	uint16_t nRestarts;
	uint16_t maxIter;
	unsigned rngSeed;

}; // KMeans

} // abc

#endif // ABC_KMEANS_H