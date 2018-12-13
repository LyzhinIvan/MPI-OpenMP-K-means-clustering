#include "common.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <ctime>

int main(int argc, char** argv)
{
    if (argc != 4) {
        fprintf(stderr,
            "Usage: kmeans num_points num_clusters num_dimensions\n");
        exit(1);
    }

    // Get args from command line:
    int n = atoi(argv[1]); // number of vectors.
    int k = atoi(argv[2]); // number of clusters.
    int d = atoi(argv[3]); // dimension of data.

    // Seed the random number generator to get different results each time
    // srand(time(NULL));
    // No, we'd like the same results.
    srand(31359);

    auto startTime = get_time();

    float* points;
    points = create_rand_nums(d * n);
    // The sum of points assigned to each cluster by this process.
    // k vectors of d elements.
    float* sums;
    assert(sums = (float*)malloc(k * d * sizeof(float)));
    // The number of points assigned to each cluster by this process. k integers.
    int* counts;
    assert(counts = (int*)malloc(k * sizeof(int)));
    // The current centroids against which points are being compared.
    // These are shipped to the process by the root process.
    float* centroids;
    assert(centroids = (float*)malloc(k * d * sizeof(float)));
    // The cluster assignments for each site.
    int* labels;
    assert(labels = (int*)malloc(n * sizeof(int)));

    // Take the first k points as the initial cluster centroids.
    memcpy(centroids, points, k * d * sizeof(float));
    print_centroids(centroids, k, d);

    float norm = 1.0; // Will tell us if centroids have moved.

    while (norm > 0.00001) { // While they've moved...

        // Each process reinitializes its cluster accumulators.
        for (int i = 0; i < k * d; i++)
            sums[i] = 0.0;
        for (int i = 0; i < k; i++)
            counts[i] = 0;

        // Find the closest centroid to each site and assign to cluster.
        #pragma omp parallel for reduction(+:counts[:k]) reduction(+:sums[:k*d])
        for (int i = 0; i < n; i++) {
            float* point = points + i * d;
            int cluster = assign_point(point, centroids, k, d);
            // Record the assignment of the site to the cluster.
            counts[cluster]++;
            add_point(point, &sums[cluster * d], d);
        }

        for (int i = 0; i < k; i++) {
            for (int j = 0; j < d; j++) {
                int dij = d * i + j;
                sums[dij] /= counts[i];
            }
        }
        // Have the centroids changed much?
        norm = distance2(sums, centroids, d * k);
        printf("norm: %f\r", norm);
        // Copy new centroids from grand_sums into centroids.
        memcpy(centroids, sums, k * d * sizeof(float));
        print_centroids(centroids, k, d);
    }

    // Now centroids are fixed, so compute a final label for each site.
    float* site = points;
    for (int i = 0; i < n; i++, site += d) {
        labels[i] = assign_point(site, centroids, k, d);
    }

    auto endTime = get_time();
    printf("Passed time: %.3lf\n", getSeconds(startTime, endTime));

    return 0;
}
