/*
 * analytical_ck.c - Compute analytical clustering spectrum C(K) for STC model
 *
 * OPTIMIZED VERSION with SADDLE-POINT ASYMPTOTIC APPROXIMATION
 *
 * For ER backbone with mean degree c and triadic closure probability f:
 *
 * C(K) = (2 / K(K-1)) * sum_k P(k|K) * [f*k*(k-1)/2 + (K-k) +
 *                                        f*(K-k)*(K-k-1)/(2k)]
 *
 * where:
 *   - Type A: f*k*(k-1)/2        triangles from closing pairs of original
 * neighbors
 *   - Type B: (K-k)              triangles from new neighbors (each creates one
 * triangle)
 *   - Type C: f*(K-k)*(K-k-1)/(2k)  triangles from pairs of new neighbors in
 * same branch
 *
 * ASYMPTOTIC APPROXIMATION:
 *   The saddle-point equation for y = K/k* is:
 *     y + log(y) + log(y-1) = A,  where A = log(Kf) + fc + 1
 *
 *   This gives k* = K/y, and the asymptotic C(K) is computed by evaluating
 *   the triangle formula at k = k*:
 *     C_asymp(K) = [f*k*(k-1)/2 + (K-k) + f*(K-k)*(K-k-1)/(2k)] / [K(K-1)/2]
 *
 * K VALUES are logarithmically spaced for efficient log-log plotting.
 *
 * Compile: gcc -O3 -o analytical_ck analytical_ck.c -lm
 * Usage:   ./analytical_ck <c> <f> [Kmax] [Npoints] [PK_min]
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Log of Poisson probability: log P(n; lambda) = n*log(lambda) - lambda -
 * log(n!)
 *
 * Uses lgamma(n+1) = log(n!) which is O(1) and numerically stable for any n.
 */
static inline double log_poisson(int n, double lambda) {
  if (n < 0) {
    return -INFINITY;
  }
  if (lambda <= 0.0) {
    return (n == 0) ? 0.0 : -INFINITY;
  }

  /* lgamma(n+1) = log(n!) — fast, accurate, works for all n */
  return (double)n * log(lambda) - lambda - lgamma((double)(n + 1));
}

/*
 * Solve the saddle-point equation for y = K/k*:
 *   y + log(y) + log(y-1) = A
 * where A = log(K*f) + f*c + 1
 *
 * Uses Newton's method for fast convergence.
 * Returns y, the amplification factor (k* = K/y)
 */
static double solve_saddle_point_y(int K, double c, double f) {
  double A = log((double)K * f) + f * c + 1.0;

  /* Initial guess: y ≈ A - 2*log(A) for large A */
  double y = A - 2.0 * log(A);
  if (y < 2.0) {
    y = 2.0;
  }

  /* Newton's method: solve h(y) = y + log(y) + log(y-1) - A = 0 */
  for (int i = 0; i < 30; i++) {
    if (y <= 1.0) {
      y = 1.001; /* Ensure y > 1 */
    }

    double h = y + log(y) + log(y - 1.0) - A;
    double h_prime = 1.0 + 1.0 / y + 1.0 / (y - 1.0);

    double y_new = y - h / h_prime;

    if (fabs(y_new - y) < 1e-12) {
      break;
    }
    y = y_new;
  }

  return y;
}

/*
 * Compute the asymptotic (saddle-point) approximation for C(K)
 *
 * Given y = K/k*, compute:
 *   k* = K/y
 *   N* = K - k*
 *
 * Then evaluate the triangle formula at k = k*:
 *   E[tri] = f*k*(k-1)/2 + N + f*N*(N-1)/(2k)
 *   C(K) = E[tri] / [K(K-1)/2]
 */
static double compute_CK_asymptotic(int K, double c, double f) {
  if (K < 2) {
    return 0.0;
  }

  /* Solve for y = K/k* */
  double y = solve_saddle_point_y(K, c, f);

  /* Compute k* and N* */
  double k_star = (double)K / y;
  double N_star = (double)K - k_star;

  /* Compute expected triangles at the saddle point */
  /* Type A: f * k* * (k* - 1) / 2 */
  double type_A = f * k_star * (k_star - 1.0) / 2.0;

  /* Type B: N* */
  double type_B = N_star;

  /* Type C: f * N* * (N* - 1) / (2 * k*) */
  double type_C = 0.0;
  if (k_star > 0.0 && N_star >= 2.0) {
    type_C = f * N_star * (N_star - 1.0) / (2.0 * k_star);
  }

  double E_tri_asymp = type_A + type_B + type_C;

  /* C(K) = E[tri] / (K choose 2) */
  double n_triads = (double)K * (double)(K - 1) / 2.0;

  return E_tri_asymp / n_triads;
}

/*
 * Compute expected number of triangles E[triangles | k, K]
 */
static inline double expected_triangles(int k, int K, double f) {
  int N = K - k; /* Number of new neighbors */

  /* Type A: pairs of original neighbors that close */
  double type_A = f * (double)k * (double)(k - 1) / 2.0;

  /* Type B: each new neighbor forms one triangle with node and intermediate */
  double type_B = (double)N;

  /* Type C: pairs of new neighbors in same branch that close */
  double type_C = 0.0;
  if (k > 0 && N >= 2) {
    type_C = f * (double)N * (double)(N - 1) / (2.0 * (double)k);
  }

  return type_A + type_B + type_C;
}

/*
 * Structure to hold all computed statistics for a given K
 */
typedef struct {
  double PK;       /* P(K) - degree probability */
  double CK;       /* C(K) - exact clustering coefficient */
  double CK_asymp; /* C(K) - saddle-point asymptotic approximation */
  double E_k;      /* E[k|K] - expected backbone degree */
  double E_tri;    /* E[triangles|K] - expected triangle count */
  bool valid;      /* Whether computation succeeded */
} KStats;

/*
 * Compute all statistics for given K in a single pass
 * This avoids redundant computation of P(k|K)
 */
static KStats compute_all_stats(int K, double c, double f) {
  KStats stats = {0.0, 0.0, 0.0, 0.0, 0.0, false};

  if (K < 2) {
    stats.valid = true; /* Valid but C(K) undefined */
    return stats;
  }

  /* Compute P(K) using log-sum-exp */
  double max_log = -INFINITY;
  double *log_terms = malloc((size_t)(K + 1) * sizeof(double));
  if (!log_terms) {
    return stats;
  }

  for (int k = 0; k <= K; k++) {
    int N = K - k;
    double log_P_k = log_poisson(k, c);
    double log_P_K_given_k = log_poisson(N, (double)k * f * c);
    log_terms[k] = log_P_k + log_P_K_given_k;

    if (log_terms[k] > max_log) {
      max_log = log_terms[k];
    }
  }

  /* Compute P(K) */
  double sum_for_PK = 0.0;
  for (int k = 0; k <= K; k++) {
    if (isfinite(log_terms[k])) {
      sum_for_PK += exp(log_terms[k] - max_log);
    }
  }
  stats.PK = exp(max_log) * sum_for_PK;

  /* Get normalized P(k|K) distribution */
  double *P_k_K = malloc((size_t)(K + 1) * sizeof(double));
  if (!P_k_K) {
    free(log_terms);
    return stats;
  }

  for (int k = 0; k <= K; k++) {
    if (isfinite(log_terms[k]) && sum_for_PK > 0.0) {
      P_k_K[k] = exp(log_terms[k] - max_log) / sum_for_PK;
    } else {
      P_k_K[k] = 0.0;
    }
  }

  free(log_terms);

  /* Compute E[triangles|K] and E[k|K] in single pass */
  double E_triangles = 0.0;
  double E_k = 0.0;

  for (int k = 0; k <= K; k++) {
    if (P_k_K[k] > 1e-300) { /* Skip truly negligible terms */
      E_triangles += P_k_K[k] * expected_triangles(k, K, f);
      E_k += P_k_K[k] * (double)k;
    }
  }

  free(P_k_K);

  /* C(K) = E[triangles] / (K choose 2) */
  double n_triads = (double)K * (double)(K - 1) / 2.0;

  stats.CK = E_triangles / n_triads;
  stats.E_k = E_k;
  stats.E_tri = E_triangles;

  /* Compute asymptotic approximation */
  stats.CK_asymp = compute_CK_asymptotic(K, c, f);

  stats.valid = true;

  return stats;
}

/*
 * Generate logarithmically spaced unique integer K values in [Kmin, Kmax].
 *
 * Produces Npoints values evenly spaced in log-space, then rounds to integers
 * and deduplicates. Returns the actual count via *out_count.
 * Caller must free the returned array.
 */
static int *generate_log_spaced_K(int Kmin, int Kmax, int Npoints,
                                  int *out_count) {
  if (Kmin < 2)
    Kmin = 2;
  if (Kmax <= Kmin || Npoints < 1) {
    *out_count = 0;
    return NULL;
  }

  int *vals = malloc((size_t)Npoints * sizeof(int));
  if (!vals) {
    *out_count = 0;
    return NULL;
  }

  double log_min = log((double)Kmin);
  double log_max = log((double)Kmax);

  int count = 0;
  int prev = -1;

  for (int i = 0; i < Npoints; i++) {
    double t = (Npoints > 1) ? (double)i / (Npoints - 1) : 0.0;
    int K = (int)round(exp(log_min + t * (log_max - log_min)));
    if (K < Kmin)
      K = Kmin;
    if (K > Kmax)
      K = Kmax;
    if (K != prev) {
      vals[count++] = K;
      prev = K;
    }
  }

  *out_count = count;
  return vals;
}

/*
 * Print usage information
 */
static void print_usage(const char *progname) {
  fprintf(stderr, "Usage: %s <c> <f> [Kmax] [Npoints] [PK_min]\n", progname);
  fprintf(stderr, "\n");
  fprintf(stderr,
          "Compute analytical clustering spectrum C(K) for STC model.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Arguments:\n");
  fprintf(stderr, "  c       - mean degree of ER backbone (must be > 0)\n");
  fprintf(stderr,
          "  f       - triadic closure probability (must be in [0, 1])\n");
  fprintf(stderr,
          "  Kmax    - maximum degree to compute (default: auto-determined)\n");
  fprintf(stderr, "  Npoints - number of log-spaced K values (default: 200)\n");
  fprintf(
      stderr,
      "  PK_min  - minimum P(K) threshold to print (default: 0, print all)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Output columns:\n");
  fprintf(stderr, "  K        - degree\n");
  fprintf(stderr, "  P(K)     - degree probability\n");
  fprintf(stderr, "  C(K)     - exact clustering coefficient (numerical)\n");
  fprintf(stderr, "  C_asymp  - saddle-point asymptotic approximation\n");
  fprintf(stderr, "  E[k|K]   - expected backbone degree\n");
  fprintf(stderr, "  E[tri|K] - expected number of triangles\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "The asymptotic approximation solves:\n");
  fprintf(stderr, "  y + log(y) + log(y-1) = log(Kf) + fc + 1\n");
  fprintf(stderr, "for y = K/k*, then evaluates the triangle formula at k*.\n");
  fprintf(stderr, "\n");
  fprintf(stderr,
          "K values are logarithmically spaced for log-log plotting.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Examples:\n");
  fprintf(stderr, "  %s 5 0.3                # Auto Kmax, 200 points\n",
          progname);
  fprintf(stderr, "  %s 5 0.3 10000          # Kmax=10000, 200 points\n",
          progname);
  fprintf(stderr, "  %s 5 0.3 10000 500      # Kmax=10000, 500 points\n",
          progname);
  fprintf(stderr, "  %s 5 0.3 10000 500 1e-20 # Also skip P(K) < 1e-20\n",
          progname);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    print_usage(argv[0]);
    return 1;
  }

  double c = atof(argv[1]);
  double f = atof(argv[2]);

  if (c <= 0.0) {
    fprintf(stderr, "Error: c must be > 0 (got %g)\n", c);
    return 1;
  }
  if (f < 0.0 || f > 1.0) {
    fprintf(stderr, "Error: f must be in [0, 1] (got %g)\n", f);
    return 1;
  }

  /* Expected mean degree in Gf */
  double mean_K = c + f * c * c;

  /* Default Kmax based on expected distribution (covers ~6 sigma) */
  int Kmax =
      (argc > 3) ? atoi(argv[3]) : (int)(mean_K + 6.0 * sqrt(mean_K)) + 10;

  /* Number of log-spaced sample points */
  int Npoints = (argc > 4) ? atoi(argv[4]) : 200;

  /* Minimum P(K) threshold (default: 0, meaning print all) */
  double PK_min = (argc > 5) ? atof(argv[5]) : 0.0;

  if (Kmax < 2) {
    fprintf(stderr, "Error: Kmax must be >= 2 (got %d)\n", Kmax);
    return 1;
  }
  if (Npoints < 2) {
    Npoints = 2;
  }

  /* Asymptotic C(K -> infinity) */
  double alpha = 1.0 + f * c;
  double C_asymp_inf = f / (alpha * alpha);

  /* Print header */
  printf("# Analytical C(K) for STC model with ER backbone\n");
  printf("# c = %.6f\n", c);
  printf("# f = %.6f\n", f);
  printf("# <K> = %.6f\n", mean_K);
  printf("# Kmax = %d\n", Kmax);
  printf("# Npoints = %d (log-spaced)\n", Npoints);
  printf("# PK_min = %.2e\n", PK_min);
  printf("# C(K->inf) = f/alpha^2 = %.6f\n", C_asymp_inf);
  printf("# Saddle-point equation: y + log(y) + log(y-1) = log(Kf) + fc + 1\n");
  printf("# C_asymp computed by evaluating triangle formula at k* = K/y\n");
  printf("#\n");
  printf("# K      P(K)             C(K)           C_asymp        E[k|K]       "
         "  E[tri|K]\n");

  /* Generate log-spaced K values */
  int count;
  int *K_values = generate_log_spaced_K(2, Kmax, Npoints, &count);
  if (!K_values) {
    fprintf(stderr, "Error: failed to generate K values\n");
    return 1;
  }

  for (int i = 0; i < count; i++) {
    int K = K_values[i];
    KStats stats = compute_all_stats(K, c, f);

    if (!stats.valid) {
      fprintf(stderr, "Warning: computation failed for K=%d\n", K);
      continue;
    }

    /* Apply threshold if specified */
    if (stats.PK < PK_min) {
      continue;
    }

    printf("%6d   %.8e   %.8f   %.8f   %.8f   %.8f\n", K, stats.PK, stats.CK,
           stats.CK_asymp, stats.E_k, stats.E_tri);
  }

  free(K_values);
  return 0;
}