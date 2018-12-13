// Creates an array of random floats. Each number has a value from 0 - 1
float *create_rand_nums(const int num_elements);

// Distance**2 between d-vectors pointed to by v1, v2.
float distance2(const float *v1, const float *v2, const int d);

// Assign a site to the correct cluster by computing its distances to
// each cluster centroid.
int assign_point(const float *point, float *centroids, const int k, const int d);

// Add a site (vector) into a sum of sites (vector).
void add_point(const float *point, float *sum, const int d);

// Print the centroids one per line.
void print_centroids(float *centroids, const int k, const int d);

// Print points with labels.
void print_points_with_labels(float *points, const int n, const int d, int *labels);

// Get time
#include <chrono>
using std::chrono::high_resolution_clock;
high_resolution_clock::time_point get_time();

double getSeconds(high_resolution_clock::time_point t1, high_resolution_clock::time_point t2);