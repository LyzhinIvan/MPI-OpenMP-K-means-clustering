#include "common.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <chrono>

// Creates an array of random floats. Each number has a value from 0 - 1
float* create_rand_nums(const int num_elements)
{
    float* rand_nums = (float*)malloc(sizeof(float) * num_elements);
    assert(rand_nums != NULL);
    for (int i = 0; i < num_elements; i++) {
        rand_nums[i] = (rand() / (float)RAND_MAX);
    }
    return rand_nums;
}

// Distance**2 between d-vectors pointed to by v1, v2.
float distance2(const float* v1, const float* v2, const int d)
{
    float dist = 0.0;
    for (int i = 0; i < d; i++) {
        float diff = v1[i] - v2[i];
        dist += diff * diff;
    }
    return dist;
}

// Assign a point to the correct cluster by computing its distances to
// each cluster centroid.
int assign_point(const float* point, float* centroids,
    const int k, const int d)
{
    int best_cluster = 0;
    float best_dist = distance2(point, centroids, d);
    float* centroid = centroids + d;
    for (int c = 1; c < k; c++, centroid += d) {
        float dist = distance2(point, centroid, d);
        if (dist < best_dist) {
            best_cluster = c;
            best_dist = dist;
        }
    }
    return best_cluster;
}

// Add a point (vector) into a sum of points (vector).
void add_point(const float* point, float* sum, const int d)
{
    for (int i = 0; i < d; i++) {
        sum[i] += point[i];
    }
}

// Print the centroids one per line.
void print_centroids(float* centroids, const int k, const int d)
{
    return;
    float* p = centroids;
    printf("Centroids:\n");
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < d; j++, p++) {
            printf("%f ", *p);
        }
        printf("\n");
    }
}

// Print points with labels.
void print_points_with_labels(float* points, const int n, const int d, int* labels)
{
	float *point = points;
	for (int i = 0; i < n; i++, point += d)
	{
		for (int j = 0; j < d; j++)
			printf("%f ", point[j]);
		printf("%4d\n", labels[i]);
	}
}

using namespace std;
using namespace std::chrono;

// Get time
high_resolution_clock::time_point get_time()
{
    return high_resolution_clock::now();
}

double getSeconds(high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
    return duration_cast<milliseconds>(t2 - t1).count() / 1e3;
}