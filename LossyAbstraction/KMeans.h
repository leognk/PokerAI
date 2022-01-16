#ifndef ABC_KMEANS_H
#define ABC_KMEANS_H

#include <random>
#include <cstring>
#include <chrono>
#include <iostream>
#include "Metrics.h"
#include "../Utils/Random.h"

namespace abc {

// Class implementing k-means++ with earth mover's distance
// or Euclidian distance.
template<typename cluSize_t, cluSize_t nClusters>
class KMeans
{
public:

	// Set rngSeed to 0 to set a random seed.
	KMeans(bool useEMD, unsigned nRestarts,
		unsigned maxIter, unsigned rngSeed = 0) :
		useEMD(useEMD),
		nRestarts(nRestarts),
		maxIter(maxIter),
		rngSeed(rngSeed),
		startTime(std::chrono::high_resolution_clock::now()),
		restartCount(0)
	{
	}

	// Run k-means and modify bestLabels in-place.
	// Return inertia and minWeight.
	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	std::pair<uint64_t, uint32_t> buildClusters(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		std::array<cluSize_t, nSamples>& bestLabels,
		std::array<std::array<feature_t, nFeatures>, nClusters>& bestCenters)
	{
		Rng rng{ (!rngSeed) ? std::random_device{}() : rngSeed };

		uint64_t bestInertia = 0;
		uint32_t bestMinWeight = 0;

		std::vector<std::vector<feature_t>> centers(
			nClusters, std::vector<feature_t>(nFeatures));
		std::vector<cluSize_t> labels(nSamples);
		
		while (restartCount < nRestarts) {

			// Initialize centers.
			kMeansPlusPlusInit(data, centers, rng);

			// Run k-means Elkan once.
			kMeansSingleElkan(data, centers, labels);

			// Update the best clustering by looking at the inertia.
			uint64_t inertia = calculateInertia(data, centers, labels);
			uint32_t minWeight = calculateMinWeight(labels);
			if (restartCount == 0 || inertia < bestInertia) {
				bestInertia = inertia;
				bestMinWeight = minWeight;
				std::copy(labels.begin(), labels.end(), bestLabels.begin());
				for (cluSize_t j = 0; j < nClusters; ++j)
					std::copy(centers[j].begin(), centers[j].end(), bestCenters[j].begin());
			}

			++restartCount;
		}

		// Count number of distinct clusters found.
		if (countUniqueLabels(bestLabels) != nClusters)
			throw std::runtime_error(
				"Number of distinct clusters found smaller than n_clusters.");

		return std::make_pair(bestInertia, bestMinWeight);
	}

private:

	typedef opt::XoShiro256PlusPlus Rng;

	// Initialize the centers with k-means++.
	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void kMeansPlusPlusInit(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		std::vector<std::vector<feature_t>>& centers,
		Rng& rng)
	{
		// Choose one center uniformly at random among the data.
		std::uniform_int_distribution<unsigned> distr(0, nSamples - 1);
		unsigned randIdx = distr(rng);
		std::copy(data[randIdx].begin(), data[randIdx].end(), centers[0].begin());

		// Initialize the squared distances between each data point
		// and the nearest center that has already been chosen.
		std::vector<uint32_t> minSqDists(nSamples);
		for (uint32_t i = 0; i < nSamples; ++i)
			minSqDists[i] = calculateDistSq(data[i], centers[0]);

		// Choose the remaining nClusters - 1 centers
		// among the data points.
		opt::FastRandomChoice<52> randChoice;
		std::vector<uint64_t> cumWeights(nSamples);
		for (cluSize_t c = 1; c < nClusters; ++c) {

			// Build the cumulated weights of the data points,
			// each weight being proportional to minSqDists.
			cumWeights[0] = minSqDists[0];
			for (uint32_t i = 1; i < nSamples; ++i)
				cumWeights[i] = cumWeights[i - 1] + minSqDists[i];
			randChoice.rescaleCumWeights(cumWeights);

			// Choose a data point with cumWeights
			// and assign it to a new center.
			unsigned randIdx = randChoice(cumWeights, rng);
			std::copy(data[randIdx].begin(), data[randIdx].end(), centers[c].begin());

			// Update minSqDists.
			for (uint32_t i = 0; i < nSamples; ++i) {
				// Skip if point i has already been chosen as a center.
				if (minSqDists[i] == 0) continue;
				uint32_t newSqDist = calculateDistSq(data[i], centers[c]);
				if (newSqDist < minSqDists[i])
					minSqDists[i] = newSqDist;
			}
		}
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void kMeansSingleElkan(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		std::vector<std::vector<feature_t>>& centers,
		std::vector<cluSize_t>& labels)
	{
		std::vector<std::vector<feature_t>> newCenters(
			nClusters, std::vector<feature_t>(nFeatures));
		std::vector<uint32_t> weightInClusters(nClusters);
		std::vector<cluSize_t> oldLabels = labels;

		// Init the matrix of the half of the distance between any 2 clusters centers.
		std::vector<std::vector<uint16_t>> centerHalfDists(
			nClusters, std::vector<uint16_t>(nClusters));
		// Init the array of the half of the distance between each center
		// and its closest center.
		std::vector<uint16_t> distNextCenter(nClusters);
		updateCenterDists(centers, centerHalfDists, distNextCenter);

		// Init the matrix of the upper bound on the distance between
		// each sample and its closest cluster center.
		std::vector<uint16_t> upperBounds(nSamples);
		// Init the matrix of the lower bound on the distance between
		// each sample and each cluster center.
		std::vector<std::vector<uint16_t>> lowerBounds(
			nSamples, std::vector<uint16_t>(nClusters));
		initBounds(data, centers, centerHalfDists, labels, upperBounds, lowerBounds);

		// Proceed to the iterations of k-means.
		for (unsigned i = 0; i < maxIter; ++i) {

			elkanIter(
				data, centers, newCenters, weightInClusters,
				centerHalfDists, distNextCenter,
				upperBounds, lowerBounds, labels);

			updateCenterDists(newCenters, centerHalfDists, distNextCenter);
			std::swap(centers, newCenters);

			printProgress(i, calculateInertia(data, centers, labels), calculateMinWeight(labels));

			if (labels == oldLabels)
				return;
			oldLabels = labels;
		}

		// Run one last step so that predicted labels match cluster centers.
		elkanIter(
			data, centers, newCenters, weightInClusters,
			centerHalfDists, distNextCenter,
			upperBounds, lowerBounds, labels);

		printProgress(maxIter, calculateInertia(data, centers, labels), calculateMinWeight(labels));
	}

	void printProgress(uint16_t nIter, uint64_t inertia, uint32_t minWeight)
	{
		auto t = std::chrono::high_resolution_clock::now();
		auto duration = 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(t - startTime).count();
		std::cout << "restart: " << std::setw(4) << restartCount + 1 << "/" << nRestarts
			<< " | n_iter: " << std::setw(3) << nIter + 1
			<< " | inertia: " << std::setw(7) << inertia
			<< " | min_weight: " << std::setw(4) << minWeight
			<< " | " << std::setw(4) << std::round(duration) << " sec\n";
	}

	template<typename feature_t>
	void updateCenterDists(
		const std::vector<std::vector<feature_t>>& centers,
		std::vector<std::vector<uint16_t>>& centerHalfDists,
		std::vector<uint16_t>& distNextCenter)
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
			uint16_t minDist = 0;
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
		const std::vector<std::vector<feature_t>>& centers,
		const std::vector<std::vector<uint16_t>>& centerHalfDists,
		std::vector<cluSize_t>& labels,
		std::vector<uint16_t>& upperBounds,
		std::vector<std::vector<uint16_t>>& lowerBounds)
	{
		for (uint32_t i = 0; i < nSamples; ++i) {
			cluSize_t bestCluster = 0;
			uint16_t minDist = calculateDist(data[i], centers[0]);
			lowerBounds[i][0] = minDist;
			for (cluSize_t j = 1; j < nClusters; ++j) {
				if (centerHalfDists[bestCluster][j] < minDist) {
					uint16_t dist = calculateDist(data[i], centers[j]);
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
		const std::vector<std::vector<feature_t>>& centers,
		std::vector<std::vector<feature_t>>& newCenters,
		std::vector<uint32_t>& weightInClusters,
		const std::vector<std::vector<uint16_t>>& centerHalfDists,
		const std::vector<uint16_t>& distNextCenter,
		std::vector<uint16_t>& upperBounds,
		std::vector<std::vector<uint16_t>>& lowerBounds,
		std::vector<cluSize_t>& labels)
	{
		std::memset(&weightInClusters[0], 0, nClusters * sizeof(uint32_t));

		for (uint32_t i = 0; i < nSamples; ++i) {
			uint16_t upperBound = upperBounds[i];
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
						&& (upperBound > lowerBounds[i][j])
						&& (upperBound > centerHalfDists[label][j])) {

						// Recompute upper bound by calculating the actual distance
						// between the sample and its current assigned center.
						if (!boundsTight) {
							upperBound = calculateDist(data[i], centers[label]);
							lowerBounds[i][label] = upperBound;
							boundsTight = true;
						}

						// If the condition still holds, then compute the actual
						// distance between the sample and center. If this is less
						// than the previous distance, reassign label.
						if (upperBound > lowerBounds[i][j]
							|| (upperBound > centerHalfDists[label][j])) {

							uint16_t dist = calculateDist(data[i], centers[j]);
							lowerBounds[i][j] = dist;
							if (dist < upperBound) {
								label = j;
								upperBound = dist;
							}
						}
					}
				}
				labels[i] = label;
				upperBounds[i] = upperBound;
			}
			++weightInClusters[label];
		}

		// Update centers.
		relocateEmptyClusters(data, centers, weightInClusters, labels);
		for (cluSize_t j = 0; j < nClusters; ++j)
			calculateCenter(data, labels, j, weightInClusters[j], newCenters[j]);
		// Center shift.
		std::vector<uint16_t> centerShift(nClusters);
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
		const std::vector<std::vector<feature_t>>& centers,
		std::vector<uint32_t>& weightInClusters,
		std::vector<cluSize_t>& labels)
	{
		// Look for empty clusters.
		std::vector<cluSize_t> emptyClusters;
		for (cluSize_t j = 0; j < nClusters; ++j) {
			if (weightInClusters[j] == 0)
				emptyClusters.push_back(j);
		}
#pragma warning(suppress: 4267)
		cluSize_t nEmpty = emptyClusters.size();
		if (nEmpty == 0) return;

		// Find the nEmpty farthest data points from their centers.
		std::vector<uint16_t> dists(nSamples);
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

			weightInClusters[newClusterId] = 1;
			--weightInClusters[oldClusterId];
			labels[farIdx] = newClusterId;
		}
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	uint64_t calculateInertia(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		const std::vector<std::vector<feature_t>>& centers,
		const std::vector<cluSize_t>& labels)
	{
		uint64_t inertia = 0;
		for (uint32_t i = 0; i < nSamples; ++i)
			inertia += calculateDistSq(data[i], centers[labels[i]]);
#pragma warning(suppress: 4244)
		return std::round(std::sqrt(inertia));
	}

	template<typename C>
	uint32_t calculateMinWeight(const C& labels)
	{
		std::vector<uint32_t> weightInClusters(nClusters);
		for (cluSize_t label : labels)
			++weightInClusters[label];
		uint32_t minWeight = weightInClusters[0];
		for (uint32_t weight : weightInClusters)
			if (weight < minWeight) minWeight = weight;
		return minWeight;
	}

	template<typename C>
	cluSize_t countUniqueLabels(const C& labels)
	{
		std::vector<uint8_t> visited(nClusters);
		for (cluSize_t label : labels)
			visited[label] = 1;
		return std::accumulate(visited.begin(), visited.end(), uint8_t(0));
	}

	template<typename C1, typename C2>
	uint16_t calculateDist(const C1& u, const C2& v)
	{
		if (useEMD) return emd(u, v);
		else return euclidianDistance(u, v);
	}

	template<typename C1, typename C2>
	uint32_t calculateDistSq(const C1& u, const C2& v)
	{
		if (useEMD) return emdSq(u, v);
		else return euclidianDistanceSq(u, v);
	}

	template<typename feature_t, uint32_t nSamples, uint8_t nFeatures>
	void calculateCenter(
		const std::array<std::array<feature_t, nFeatures>, nSamples>& data,
		const std::vector<cluSize_t>& labels,
		cluSize_t label,
		uint32_t weight,
		std::vector<feature_t>& center)
	{
		if (useEMD) return emdCenter(data, labels, label, weight, center);
		else return euclidianCenter(data, labels, label, weight, center);
	}

	bool useEMD;
	unsigned nRestarts;
	unsigned maxIter;
	unsigned rngSeed;
	std::chrono::high_resolution_clock::time_point startTime;
	unsigned restartCount;

}; // KMeans

} // abc

#endif // ABC_KMEANS_H