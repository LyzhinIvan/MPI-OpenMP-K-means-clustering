#include "common.h"
#include <cstdio>
#include <cstdlib>
#include <mpi.h>
#include <cassert>
#include <cstring>

int main(int argc, char** argv)
{
    if (argc != 4) {
        fprintf(stderr,
            "Usage: kmeans num_points num_clusters num_dimensions\n");
        exit(1);
    }

    // Get args from command line:
    int n = atoi(argv[1]); // number of points.
    int k = atoi(argv[2]); // number of clusters.
    int d = atoi(argv[3]); // dimension of data.

    // Seed the random number generator to get different results each time
    // srand(time(NULL));
    // No, we'd like the same results.
    srand(31359);

    auto startTime = get_time();

    // Initial MPI and find process rank and number of processes.
    MPI_Init(NULL, NULL);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int points_per_proc = n / nprocs;

    //
    // Data structures in all processes.
    //
    // The points assigned to this process.
    float* points;
    assert(points = (float*)malloc(points_per_proc * d * sizeof(float)));
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
    // The cluster assignments for each point.
    int* labels;
    assert(labels = (int*)malloc(points_per_proc * sizeof(int)));

    //
    // Data structures maintained only in root process.
    //
    // All the points for all the processes.
    // point_per_proc * nprocs vectors of d floats.
    float* all_points = NULL;
    // Sum of points assigned to each cluster by all processes.
    float* grand_sums = NULL;
    // Number of points assigned to each cluster by all processes.
    int* grand_counts = NULL;
    // Result of program: a cluster label for each point.
    int* all_labels;
    if (rank == 0) {
        all_points = create_rand_nums(d * points_per_proc * nprocs);
        // Take the first k points as the initial cluster centroids.
        for (int i = 0; i < k * d; i++) {
            centroids[i] = all_points[i];
        }
        print_centroids(centroids, k, d);
        assert(grand_sums = (float*)malloc(k * d * sizeof(float)));
        assert(grand_counts = (int*)malloc(k * sizeof(int)));
        assert(all_labels = (int*)malloc(nprocs * points_per_proc * sizeof(int)));
    }

    // Root sends each process its share of points.
    MPI_Scatter(all_points, d * points_per_proc, MPI_FLOAT, points,
        d * points_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

    float norm = 1.0; // Will tell us if centroids have moved.

    while (norm > 0.00001) { // While they've moved...

        // Broadcast the current cluster centroids to all processes.
        MPI_Bcast(centroids, k * d, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Each process reinitializes its cluster accumulators.
        for (int i = 0; i < k * d; i++)
            sums[i] = 0.0;
        for (int i = 0; i < k; i++)
            counts[i] = 0;

		// Find the closest centroid to each point and assign to cluster.
		#pragma omp parallel for reduction(+:counts[:k]) reduction(+:sums[:k*d])
        for (int i = 0; i < points_per_proc; i++) {
            float* point = points + i * d;
            int cluster = assign_point(point, centroids, k, d);
			// Record the assignment of the point to the cluster.
			counts[cluster]++;
			add_point(point, &sums[cluster * d], d);
        }

        // Gather and sum at root all cluster sums for individual processes.
        MPI_Reduce(sums, grand_sums, k * d, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(counts, grand_counts, k, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            // Root process computes new centroids by dividing sums per cluster
            // by count per cluster.
            for (int i = 0; i < k; i++) {
                for (int j = 0; j < d; j++) {
                    int dij = d * i + j;
                    grand_sums[dij] /= grand_counts[i];
                }
            }
            // Have the centroids changed much?
            norm = distance2(grand_sums, centroids, d * k);
            printf("norm: %f\r", norm);
            // Copy new centroids from grand_sums into centroids.
            memcpy(centroids, grand_sums, k * d * sizeof(float));
            print_centroids(centroids, k, d);
        }
        // Broadcast the norm.    All processes will use this in the loop test.
        MPI_Bcast(&norm, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }

    // Now centroids are fixed, so compute a final label for each point.
    float* point = points;
    for (int i = 0; i < points_per_proc; i++, point += d) {
        labels[i] = assign_point(point, centroids, k, d);
    }

    // Gather all labels into root process.
    MPI_Gather(labels, points_per_proc, MPI_INT,
    	   all_labels, points_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    auto endTime = get_time();
    if (rank == 0) {
        printf("Passed time: %.3lf\n", getSeconds(startTime, endTime));
    }

    MPI_Finalize();
    return 0;
}
