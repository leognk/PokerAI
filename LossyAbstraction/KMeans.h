#ifndef ABC_KMEANS_H
#define ABC_KMEANS_H

#include <random>
#include <cstring>
#include <chrono>
#include <iostream>
#include "Metrics.h"
#include "../Utils/Random.h"

namespace abc {

enum KMeansInitMode { PlusPlus, PlusPlusMax, PlusPlusMaxMax };
enum KMeansIterMode { Elkan, SemiElkan };

// Class implementing k-means++ with earth mover's distance
// or Euclidian distance.
template<typename cluSize_t, cluSize_t nClusters>
class KMeans
{
public:

	// Set rngSeed to 0 to set a random seed.
	KMeans(bool useEMD, unsigned nRestarts,
		unsigned maxIter, unsigned rngSeed = 0,
		KMeansInitMode kmeansInitMode = KMeansInitMode::PlusPlus,
		KMeansIterMode kmeansIterMode = KMeansIterMode::Elkan) :
		useEMD(useEMD),
		nRestarts(nRestarts),
		maxIter(maxIter),
		rngSeed(rngSeed),
		kmeansInitMode(kmeansInitMode),
		kmeansIterMode(kmeansIterMode),
		startTime(std::chrono::high_resolution_clock::now()),
		restartCount(0)
	{
	}

	// Run k-means and modify bestLabels and bestCenters in-place.
	// Return inertia and minWeight.
	template<typename CData, typename CLabels, typename CCenters>
	std::pair<uint64_t, uint32_t> buildClusters(
		const CData& data,
		CLabels& bestLabels,
		CCenters& bestCenters)
	{
		typedef CData::value_type::value_type feature_t;
#pragma warning(suppress: 4267)
		nSamples = data.size();
#pragma warning(suppress: 4267)
		nFeatures = data[0].size();

		Rng rng{ (!rngSeed) ? std::random_device{}() : rngSeed };

		uint64_t bestInertia = 0;
		uint32_t bestMinWeight = 0;

		std::vector<std::vector<feature_t>> centers(
			nClusters, std::vector<feature_t>(nFeatures));
		std::vector<cluSize_t> labels(nSamples);
		
		while (restartCount < nRestarts) {

			// Initialize centers.
			switch (kmeansInitMode) {
			case KMeansInitMode::PlusPlus:
				kMeansPlusPlusInit(data, centers, rng);
				break;
			case KMeansInitMode::PlusPlusMax:
				kMeansPlusPlusMaxInit(data, centers, rng);
				break;
			case KMeansInitMode::PlusPlusMaxMax:
				kMeansPlusPlusMaxMaxInit(data, centers);
				break;
			default:
				throw std::runtime_error("Specified k-means init mode does not exist.");
			}

			// Run k-means once.
			switch (kmeansIterMode) {
			case KMeansIterMode::Elkan:
				kMeansSingleElkan(data, centers, labels);
				break;
			case KMeansIterMode::SemiElkan:
				kMeansSingleSemiElkan(data, centers, labels);
				break;
			default:
				throw std::runtime_error("Specified k-means iter mode does not exist.");
			}

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
	template<typename C, typename feature_t>
	void kMeansPlusPlusInit(
		const C& data,
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

	// Initialize the centers with k-means++ but deterministically
	// by choosing the farthest data point as the new center at each step.
	template<typename C, typename feature_t>
	void kMeansPlusPlusMaxInit(
		const C& data,
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

		// Initialize the farthest data point from the centers
		// that have already been chosen.
		uint32_t maxMinSqDist = 0;
		uint32_t maxMinIdx = 0;
		for (uint32_t i = 0; i < nSamples; ++i) {
			if (minSqDists[i] > maxMinSqDist) {
				maxMinSqDist = minSqDists[i];
				maxMinIdx = i;
			}
		}

		// Choose the remaining nClusters - 1 centers
		// among the data points.
		for (cluSize_t c = 1; c < nClusters; ++c) {

			// Assign the farthest data point from already chosen centers
			// to a new center.
#pragma warning(suppress: 28020)
			std::copy(data[maxMinIdx].begin(), data[maxMinIdx].end(), centers[c].begin());

			// Update minSqDists, maxMinSqDist and maxMinIdx.
			maxMinSqDist = 0;
			for (uint32_t i = 0; i < nSamples; ++i) {
				// Skip if point i has already been chosen as a center.
				if (minSqDists[i] == 0) continue;
				uint32_t newSqDist = calculateDistSq(data[i], centers[c]);
				if (newSqDist < minSqDists[i])
					minSqDists[i] = newSqDist;
				if (minSqDists[i] > maxMinSqDist) {
					maxMinSqDist = minSqDists[i];
					maxMinIdx = i;
				}
			}
		}
	}

	// Initialize the centers with k-means++ max but
	// by choosing the farthest data point from the center of all
	// the data points as the first center.
	template<typename C, typename feature_t>
	void kMeansPlusPlusMaxMaxInit(
		const C& data,
		std::vector<std::vector<feature_t>>& centers)
	{
		// Choose the first center.
		std::vector<feature_t> globalCenter(nFeatures);
		calculateCenter(data, globalCenter);
		uint32_t maxSqDist = 0;
		uint32_t maxIdx = 0;
		for (uint32_t i = 0; i < nSamples; ++i) {
			uint32_t sqDist = calculateDistSq(data[i], globalCenter);
			if (sqDist > maxSqDist) {
				maxSqDist = sqDist;
				maxIdx = i;
			}
		}
#pragma warning(suppress: 28020)
		std::copy(data[maxIdx].begin(), data[maxIdx].end(), centers[0].begin());

		// Initialize the squared distances between each data point
		// and the nearest center that has already been chosen.
		std::vector<uint32_t> minSqDists(nSamples);
		for (uint32_t i = 0; i < nSamples; ++i)
			minSqDists[i] = calculateDistSq(data[i], centers[0]);

		// Initialize the farthest data point from the centers
		// that have already been chosen.
		uint32_t maxMinSqDist = 0;
		uint32_t maxMinIdx = 0;
		for (uint32_t i = 0; i < nSamples; ++i) {
			if (minSqDists[i] > maxMinSqDist) {
				maxMinSqDist = minSqDists[i];
				maxMinIdx = i;
			}
		}

		// Choose the remaining nClusters - 1 centers
		// among the data points.
		for (cluSize_t c = 1; c < nClusters; ++c) {

			// Assign the farthest data point from already chosen centers
			// to a new center.
			std::copy(data[maxMinIdx].begin(), data[maxMinIdx].end(), centers[c].begin());

			// Update minSqDists, maxMinSqDist and maxMinIdx.
			maxMinSqDist = 0;
			for (uint32_t i = 0; i < nSamples; ++i) {
				// Skip if point i has already been chosen as a center.
				if (minSqDists[i] == 0) continue;
				uint32_t newSqDist = calculateDistSq(data[i], centers[c]);
				if (newSqDist < minSqDists[i])
					minSqDists[i] = newSqDist;
				if (minSqDists[i] > maxMinSqDist) {
					maxMinSqDist = minSqDists[i];
					maxMinIdx = i;
				}
			}
		}
	}

	template<typename C, typename feature_t>
	void kMeansSingleElkan(
		const C& data,
		std::vector<std::vector<feature_t>>& centers,
		std::vector<cluSize_t>& labels)
	{
		std::vector<std::vector<feature_t>> newCenters(
			nClusters, std::vector<feature_t>(nFeatures));
		std::vector<uint32_t> weightInClusters(nClusters);
		uint64_t oldInertia = 0xffffffffffffffff;

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

			uint64_t inertia = calculateInertia(data, centers, labels);
			printProgress(i, inertia, calculateMinWeight(labels));

			if (inertia >= oldInertia)
				return;
			oldInertia = inertia;
		}

		// Run one last step so that predicted labels match cluster centers.
		elkanIter(
			data, centers, newCenters, weightInClusters,
			centerHalfDists, distNextCenter,
			upperBounds, lowerBounds, labels);

		printProgress(maxIter, calculateInertia(data, centers, labels), calculateMinWeight(labels));
	}

	template<typename C, typename feature_t>
	void kMeansSingleSemiElkan(
		const C& data,
		std::vector<std::vector<feature_t>>& centers,
		std::vector<cluSize_t>& labels)
	{
		std::vector<std::vector<feature_t>> newCenters(
			nClusters, std::vector<feature_t>(nFeatures));
		std::vector<uint32_t> weightInClusters(nClusters);
		uint64_t oldInertia = 0xffffffffffffffff;

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
		initSemiBounds(data, centers, centerHalfDists, labels, upperBounds);

		// Proceed to the iterations of k-means.
		for (unsigned i = 0; i < maxIter; ++i) {

			semiElkanIter(
				data, centers, newCenters, weightInClusters,
				centerHalfDists, distNextCenter, upperBounds, labels);

			updateCenterDists(newCenters, centerHalfDists, distNextCenter);
			std::swap(centers, newCenters);

			uint64_t inertia = calculateInertia(data, centers, labels);
			printProgress(i, inertia, calculateMinWeight(labels));

			if (inertia >= oldInertia)
				return;
			oldInertia = inertia;
		}

		// Run one last step so that predicted labels match cluster centers.
		semiElkanIter(
			data, centers, newCenters, weightInClusters,
			centerHalfDists, distNextCenter, upperBounds, labels);

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

	template<typename C, typename feature_t>
	void initBounds(
		const C& data,
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

	template<typename C, typename feature_t>
	void initSemiBounds(
		const C& data,
		const std::vector<std::vector<feature_t>>& centers,
		const std::vector<std::vector<uint16_t>>& centerHalfDists,
		std::vector<cluSize_t>& labels,
		std::vector<uint16_t>& upperBounds)
	{
		for (uint32_t i = 0; i < nSamples; ++i) {
			cluSize_t bestCluster = 0;
			uint16_t minDist = calculateDist(data[i], centers[0]);
			for (cluSize_t j = 1; j < nClusters; ++j) {
				if (centerHalfDists[bestCluster][j] < minDist) {
					uint16_t dist = calculateDist(data[i], centers[j]);
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

	template<typename C, typename feature_t>
	void elkanIter(
		const C& data,
		const std::vector<std::vector<feature_t>>& centers,
		std::vector<std::vector<feature_t>>& newCenters,
		std::vector<uint32_t>& weightInClusters,
		const std::vector<std::vector<uint16_t>>& centerHalfDists,
		const std::vector<uint16_t>& distNextCenter,
		std::vector<uint16_t>& upperBounds,
		std::vector<std::vector<uint16_t>>& lowerBounds,
		std::vector<cluSize_t>& labels)
	{
		for (std::vector<feature_t>& center : newCenters)
			std::memset(&center[0], 0, nFeatures * sizeof(feature_t));
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
		calculateCenters(data, labels, weightInClusters, newCenters);
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

	template<typename C, typename feature_t>
	void semiElkanIter(
		const C& data,
		const std::vector<std::vector<feature_t>>& centers,
		std::vector<std::vector<feature_t>>& newCenters,
		std::vector<uint32_t>& weightInClusters,
		const std::vector<std::vector<uint16_t>>& centerHalfDists,
		const std::vector<uint16_t>& distNextCenter,
		std::vector<uint16_t>& upperBounds,
		std::vector<cluSize_t>& labels)
	{
		for (std::vector<feature_t>& center : newCenters)
			std::memset(&center[0], 0, nFeatures * sizeof(feature_t));
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
					// recomputing the upper bound.
					if (j != label
						&& (upperBound > centerHalfDists[label][j])) {

						// Recompute upper bound by calculating the actual distance
						// between the sample and its current assigned center.
						if (!boundsTight) {
							upperBound = calculateDist(data[i], centers[label]);
							boundsTight = true;
						}

						// Compute the actual distance between the sample and center.
						// If this is less than the previous distance, reassign label.
						uint16_t dist = calculateDist(data[i], centers[j]);
						if (dist < upperBound) {
							label = j;
							upperBound = dist;
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
		calculateCenters(data, labels, weightInClusters, newCenters);
		// Center shift.
		std::vector<uint16_t> centerShift(nClusters);
		for (cluSize_t j = 0; j < nClusters; ++j)
			centerShift[j] = calculateDist(newCenters[j], centers[j]);
		// Update upper bounds.
		for (uint32_t i = 0; i < nSamples; ++i)
			upperBounds[i] += centerShift[labels[i]];
	}

	template<typename C, typename feature_t>
	void relocateEmptyClusters(
		const C& data,
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

	template<typename C, typename feature_t>
	uint64_t calculateInertia(
		const C& data,
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
		return std::accumulate(visited.begin(), visited.end(), cluSize_t(0));
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

	template<typename C, typename feature_t>
	void calculateCenter(const C& data, std::vector<feature_t>& center)
	{
		if (useEMD) emdCenter(data, center);
		else euclidianCenter(data, center);
	}

	template<typename C, typename feature_t>
	void calculateCenters(
		const C& data,
		const std::vector<cluSize_t>& labels,
		std::vector<uint32_t>& weights,
		std::vector<std::vector<feature_t>>& centers)
	{
		if (useEMD) emdCenters(data, labels, weights, centers);
		else euclidianCenters(data, labels, weights, centers);
	}

	bool useEMD;
	unsigned nRestarts;
	unsigned maxIter;
	unsigned rngSeed;
	KMeansInitMode kmeansInitMode;
	KMeansIterMode kmeansIterMode;
	std::chrono::high_resolution_clock::time_point startTime;
	unsigned restartCount;

	uint32_t nSamples = 0;
	uint8_t nFeatures = 0;

}; // KMeans

} // abc

#endif // ABC_KMEANS_H