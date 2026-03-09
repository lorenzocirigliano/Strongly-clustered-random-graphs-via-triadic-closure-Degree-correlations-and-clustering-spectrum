/*
 * measurements.h - Graph measurements for STC model
 *
 * Compute degree distribution, clustering spectrum, knn, etc.
 */

#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include "graph.h"

/*
 * Statistics accumulator for degree-binned quantities
 */
typedef struct {
  int kmax;       /* Maximum degree tracked */
  long *count;    /* count[k] = number of samples at degree k */
  double *sum;    /* sum[k] = sum of values at degree k */
  double *sum_sq; /* sum_sq[k] = sum of squared values at degree k */
} BinnedAccum;

BinnedAccum *accum_alloc(int kmax);
void accum_free(BinnedAccum *acc);
void accum_reset(BinnedAccum *acc);
void accum_add(BinnedAccum *acc, int k, double value);
double accum_mean(const BinnedAccum *acc, int k);
double accum_stderr(const BinnedAccum *acc, int k);
long accum_count(const BinnedAccum *acc, int k);

/*
 * Histogram for degree distribution P(K)
 */
typedef struct {
  int kmax;    /* Maximum degree tracked */
  long *count; /* count[k] = number of nodes with degree k */
  long total;  /* Total number of nodes counted */
} DegreeHist;

DegreeHist *deghist_alloc(int kmax);
void deghist_free(DegreeHist *h);
void deghist_reset(DegreeHist *h);
void deghist_add(DegreeHist *h, int k);
double deghist_pdf(const DegreeHist *h, int k);
double deghist_cdf(const DegreeHist *h, int k);
long deghist_count(const DegreeHist *h, int k);
void deghist_write(const DegreeHist *h, const char *filename,
                   const char *header);

/*
 * Measure degree distribution P(K).
 * Adds one count per node to histogram.
 */
void measure_degree_distribution(const Graph *g, DegreeHist *hist);

/*
 * Measure local clustering coefficient C_i for each node.
 * Adds (K_i, C_i) pairs to accumulator.
 *
 * C_i = (number of triangles at i) / (K_i choose 2)
 *     = 2 * triangles_i / (K_i * (K_i - 1))
 */
void measure_clustering(const Graph *g, BinnedAccum *acc);

/*
 * Measure average nearest neighbor degree knn(K).
 * For each node i with degree K_i, computes mean degree of neighbors.
 * Adds (K_i, knn_i) pairs to accumulator.
 */
void measure_knn(const Graph *g, BinnedAccum *acc);

/*
 * Measure degree-degree correlations (for Pearson coefficient).
 * Returns covariance and variance through output pointers.
 */
void measure_degree_correlations(const Graph *g, double *mean_k, double *var_k,
                                 double *cov_k1k2);

/*
 * Count total triangles in graph.
 * Returns 3 * (number of triangles), i.e., sum over nodes of triangles at node.
 */
long count_triangles(const Graph *g);

/*
 * Write accumulator to file.
 * Format: k mean(k) stderr(k) count(k)
 */
void accum_write(const BinnedAccum *acc, const char *filename,
                 const char *header);

#endif /* MEASUREMENTS_H */