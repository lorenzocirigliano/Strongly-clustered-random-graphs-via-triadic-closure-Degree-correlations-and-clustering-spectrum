/*
 * measurements.c - Graph measurements implementation
 */

#include "measurements.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * BinnedAccum implementation
 * ============================================================================
 */

BinnedAccum *accum_alloc(int kmax) {
  BinnedAccum *acc = malloc(sizeof(BinnedAccum));
  if (!acc)
    return NULL;

  acc->kmax = kmax;
  acc->count = calloc(kmax + 1, sizeof(long));
  acc->sum = calloc(kmax + 1, sizeof(double));
  acc->sum_sq = calloc(kmax + 1, sizeof(double));

  if (!acc->count || !acc->sum || !acc->sum_sq) {
    accum_free(acc);
    return NULL;
  }

  return acc;
}

void accum_free(BinnedAccum *acc) {
  if (acc) {
    free(acc->count);
    free(acc->sum);
    free(acc->sum_sq);
    free(acc);
  }
}

void accum_reset(BinnedAccum *acc) {
  memset(acc->count, 0, (acc->kmax + 1) * sizeof(long));
  memset(acc->sum, 0, (acc->kmax + 1) * sizeof(double));
  memset(acc->sum_sq, 0, (acc->kmax + 1) * sizeof(double));
}

void accum_add(BinnedAccum *acc, int k, double value) {
  if (k < 0 || k > acc->kmax)
    return;
  acc->count[k]++;
  acc->sum[k] += value;
  acc->sum_sq[k] += value * value;
}

double accum_mean(const BinnedAccum *acc, int k) {
  if (k < 0 || k > acc->kmax || acc->count[k] == 0)
    return 0.0;
  return acc->sum[k] / acc->count[k];
}

double accum_stderr(const BinnedAccum *acc, int k) {
  if (k < 0 || k > acc->kmax || acc->count[k] < 2)
    return 0.0;
  double mean = acc->sum[k] / acc->count[k];
  double var = acc->sum_sq[k] / acc->count[k] - mean * mean;
  if (var < 0)
    var = 0;
  return sqrt(var / acc->count[k]);
}

long accum_count(const BinnedAccum *acc, int k) {
  if (k < 0 || k > acc->kmax)
    return 0;
  return acc->count[k];
}

void accum_write(const BinnedAccum *acc, const char *filename,
                 const char *header) {
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: cannot open %s for writing\n", filename);
    return;
  }

  if (header) {
    fprintf(fp, "%s\n", header);
  }

  for (int k = 0; k <= acc->kmax; k++) {
    if (acc->count[k] > 0) {
      double mean = accum_mean(acc, k);
      double se = accum_stderr(acc, k);
      fprintf(fp, "%d %.10e %.10e %ld\n", k, mean, se, acc->count[k]);
    }
  }

  fclose(fp);
}

/* ============================================================================
 * DegreeHist implementation
 * ============================================================================
 */

DegreeHist *deghist_alloc(int kmax) {
  DegreeHist *h = malloc(sizeof(DegreeHist));
  if (!h)
    return NULL;

  h->kmax = kmax;
  h->count = calloc(kmax + 1, sizeof(long));
  h->total = 0;

  if (!h->count) {
    deghist_free(h);
    return NULL;
  }

  return h;
}

void deghist_free(DegreeHist *h) {
  if (h) {
    free(h->count);
    free(h);
  }
}

void deghist_reset(DegreeHist *h) {
  memset(h->count, 0, (h->kmax + 1) * sizeof(long));
  h->total = 0;
}

void deghist_add(DegreeHist *h, int k) {
  if (k < 0 || k > h->kmax)
    return;
  h->count[k]++;
  h->total++;
}

double deghist_pdf(const DegreeHist *h, int k) {
  if (k < 0 || k > h->kmax || h->total == 0)
    return 0.0;
  return (double)h->count[k] / h->total;
}

double deghist_cdf(const DegreeHist *h, int k) {
  if (k < 0 || h->total == 0)
    return 0.0;
  if (k > h->kmax)
    return 1.0;

  long cumsum = 0;
  for (int i = 0; i <= k; i++) {
    cumsum += h->count[i];
  }
  return (double)cumsum / h->total;
}

long deghist_count(const DegreeHist *h, int k) {
  if (k < 0 || k > h->kmax)
    return 0;
  return h->count[k];
}

void deghist_write(const DegreeHist *h, const char *filename,
                   const char *header) {
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: cannot open %s for writing\n", filename);
    return;
  }

  if (header) {
    fprintf(fp, "%s\n", header);
  }

  for (int k = 0; k <= h->kmax; k++) {
    if (h->count[k] > 0) {
      double pdf = deghist_pdf(h, k);
      double cdf = deghist_cdf(h, k);
      fprintf(fp, "%d %.10e %.10e %ld\n", k, pdf, cdf, h->count[k]);
    }
  }

  fclose(fp);
}

/* ============================================================================
 * Measurement functions
 * ============================================================================
 */

void measure_degree_distribution(const Graph *g, DegreeHist *hist) {
  for (int i = 0; i < g->n; i++) {
    int k = g->degree[i];
    deghist_add(hist, k);
  }
}

/*
 * Helper: count triangles at node i using adjacency list intersection
 */
static int count_triangles_at_node(const Graph *g, int i) {
  const int *neighbors_i = graph_neighbors(g, i);
  int deg_i = g->degree[i];

  int count = 0;

  /* For each pair of neighbors (j, k), check if edge (j, k) exists */
  for (int a = 0; a < deg_i; a++) {
    int j = neighbors_i[a];
    const int *neighbors_j = graph_neighbors(g, j);
    int deg_j = g->degree[j];

    for (int b = a + 1; b < deg_i; b++) {
      int k = neighbors_i[b];

      /* Check if k is in neighbors of j */
      /* Linear scan (could be optimized with sorted lists) */
      for (int c = 0; c < deg_j; c++) {
        if (neighbors_j[c] == k) {
          count++;
          break;
        }
      }
    }
  }

  return count;
}

void measure_clustering(const Graph *g, BinnedAccum *acc) {
  for (int i = 0; i < g->n; i++) {
    int k = g->degree[i];

    if (k < 2) {
      /* C_i undefined or 0 for degree < 2 */
      /* We can either skip or add 0 */
      continue;
    }

    int triangles = count_triangles_at_node(g, i);
    double C_i = 2.0 * triangles / ((double)k * (k - 1));

    accum_add(acc, k, C_i);
  }
}

void measure_knn(const Graph *g, BinnedAccum *acc) {
  for (int i = 0; i < g->n; i++) {
    int k = g->degree[i];

    if (k == 0)
      continue;

    const int *neighbors = graph_neighbors(g, i);
    double sum_neighbor_deg = 0;

    for (int a = 0; a < k; a++) {
      sum_neighbor_deg += g->degree[neighbors[a]];
    }

    double knn_i = sum_neighbor_deg / k;
    accum_add(acc, k, knn_i);
  }
}

void measure_degree_correlations(const Graph *g, double *mean_k, double *var_k,
                                 double *cov_k1k2) {
  /* Compute statistics over edges */
  long n_edges = 0;
  double sum_k = 0;
  double sum_k_sq = 0;
  double sum_k1k2 = 0;

  for (int e = 0; e < g->m; e++) {
    int k1 = g->degree[g->edge_u[e]];
    int k2 = g->degree[g->edge_v[e]];

    /* Each edge contributes twice (once for each direction) */
    sum_k += k1 + k2;
    sum_k_sq += (double)k1 * k1 + (double)k2 * k2;
    sum_k1k2 += 2.0 * k1 * k2; /* Both (k1,k2) and (k2,k1) */
    n_edges += 2;
  }

  if (n_edges == 0) {
    *mean_k = *var_k = *cov_k1k2 = 0;
    return;
  }

  *mean_k = sum_k / n_edges;
  *var_k = sum_k_sq / n_edges - (*mean_k) * (*mean_k);
  *cov_k1k2 = sum_k1k2 / n_edges - (*mean_k) * (*mean_k);
}

long count_triangles(const Graph *g) {
  long total = 0;

  for (int i = 0; i < g->n; i++) {
    total += count_triangles_at_node(g, i);
  }

  /* Each triangle is counted 3 times (once at each vertex) */
  return total;
}