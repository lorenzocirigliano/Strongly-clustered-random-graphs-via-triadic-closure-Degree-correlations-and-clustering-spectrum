#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== RNG (xoshiro256**) ========== */

static unsigned long rng_state[4];

static inline unsigned long rotl(const unsigned long x, int k) {
  return (x << k) | (x >> (64 - k));
}

static unsigned long rng_next(void) {
  const unsigned long result =
      rotl(rng_state[0] + rng_state[3], 23) + rng_state[0];
  const unsigned long t = rng_state[1] << 17;
  rng_state[2] ^= rng_state[0];
  rng_state[3] ^= rng_state[1];
  rng_state[1] ^= rng_state[2];
  rng_state[0] ^= rng_state[3];
  rng_state[2] ^= t;
  rng_state[3] = rotl(rng_state[3], 45);
  return result;
}

static void rng_seed(unsigned long seed) {
  rng_state[0] = seed;
  rng_state[1] = seed ^ 0x123456789abcdef0UL;
  rng_state[2] = seed ^ 0xfedcba9876543210UL;
  rng_state[3] = seed ^ 0x0f1e2d3c4b5a6978UL;
  for (int i = 0; i < 20; i++)
    rng_next();
}

static inline double rng_uniform(void) {
  return (rng_next() >> 11) * (1.0 / 9007199254740992.0);
}

static int rng_poisson(double lambda) {
  if (lambda <= 0)
    return 0;
  if (lambda < 30) {
    double L = exp(-lambda);
    int k = 0;
    double p = 1.0;
    do {
      k++;
      p *= rng_uniform();
    } while (p > L);
    return k - 1;
  } else {
    double u1 = rng_uniform();
    double u2 = rng_uniform();
    double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    int k = (int)(lambda + sqrt(lambda) * z + 0.5);
    return (k < 0) ? 0 : k;
  }
}

static int rng_binomial(int n, double p) {
  if (n <= 0 || p <= 0.0)
    return 0;
  if (p >= 1.0)
    return n;

  int count = 0;
  if (n * p < 20 && n < 100) {
    for (int i = 0; i < n; i++) {
      if (rng_uniform() < p)
        count++;
    }
  } else {
    double mu = n * p;
    double sigma = sqrt(n * p * (1 - p));
    double u1 = rng_uniform();
    double u2 = rng_uniform();
    double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    count = (int)(mu + sigma * z + 0.5);
    if (count < 0)
      count = 0;
    if (count > n)
      count = n;
  }
  return count;
}

/* ========== Backbone types ========== */

typedef enum { BACKBONE_ER, BACKBONE_RRN, BACKBONE_PL } BackboneType;

typedef struct {
  BackboneType type;
  double c;       /* ER: mean degree */
  int degree;     /* RRN: fixed degree */
  int kmin, kmax; /* PL: degree range */
  double gamma;   /* PL: exponent */
  double *pk_cdf; /* PL: CDF for p(k) */
  double *qr_cdf; /* PL: CDF for q(r) */
  double mean_k;
  double mean_r;
} Backbone;

static Backbone *backbone_create_er(double c) {
  Backbone *bb = calloc(1, sizeof(Backbone));
  bb->type = BACKBONE_ER;
  bb->c = c;
  bb->mean_k = c;
  bb->mean_r = c;
  return bb;
}

static Backbone *backbone_create_rrn(int degree) {
  Backbone *bb = calloc(1, sizeof(Backbone));
  bb->type = BACKBONE_RRN;
  bb->degree = degree;
  bb->mean_k = degree;
  bb->mean_r = degree - 1;
  return bb;
}

static Backbone *backbone_create_pl(int kmin, double gamma, int kmax) {
  Backbone *bb = calloc(1, sizeof(Backbone));
  bb->type = BACKBONE_PL;
  bb->kmin = kmin;
  bb->kmax = kmax;
  bb->gamma = gamma;

  int pk_len = kmax - kmin + 1;
  bb->pk_cdf = malloc(pk_len * sizeof(double));

  double Z = 0.0;
  for (int k = kmin; k <= kmax; k++)
    Z += pow(k, -gamma);

  double sum_k = 0.0, cumsum = 0.0;
  for (int i = 0; i < pk_len; i++) {
    int k = kmin + i;
    double pk = pow(k, -gamma) / Z;
    sum_k += k * pk;
    cumsum += pk;
    bb->pk_cdf[i] = cumsum;
  }
  bb->mean_k = sum_k;

  int rmin = kmin - 1;
  int rmax = kmax - 1;
  int qr_len = rmax - rmin + 1;
  bb->qr_cdf = malloc(qr_len * sizeof(double));

  double sum_r = 0.0;
  cumsum = 0.0;
  for (int i = 0; i < qr_len; i++) {
    int r = rmin + i;
    int k = r + 1;
    double qr = k * pow(k, -gamma) / (Z * sum_k);
    sum_r += r * qr;
    cumsum += qr;
    bb->qr_cdf[i] = cumsum;
  }
  bb->mean_r = sum_r;

  return bb;
}

static void backbone_free(Backbone *bb) {
  if (bb) {
    free(bb->pk_cdf);
    free(bb->qr_cdf);
    free(bb);
  }
}

static int sample_k(const Backbone *bb) {
  switch (bb->type) {
  case BACKBONE_ER:
    return rng_poisson(bb->c);
  case BACKBONE_RRN:
    return bb->degree;
  case BACKBONE_PL: {
    double u = rng_uniform();
    int lo = 0, hi = bb->kmax - bb->kmin;
    while (lo < hi) {
      int mid = (lo + hi) / 2;
      if (bb->pk_cdf[mid] < u)
        lo = mid + 1;
      else
        hi = mid;
    }
    return bb->kmin + lo;
  }
  }
  return 0;
}

static int sample_r(const Backbone *bb) {
  switch (bb->type) {
  case BACKBONE_ER:
    return rng_poisson(bb->c);
  case BACKBONE_RRN:
    return bb->degree - 1;
  case BACKBONE_PL: {
    double u = rng_uniform();
    int lo = 0, hi = bb->kmax - bb->kmin;
    while (lo < hi) {
      int mid = (lo + hi) / 2;
      if (bb->qr_cdf[mid] < u)
        lo = mid + 1;
      else
        hi = mid;
    }
    return (bb->kmin - 1) + lo;
  }
  }
  return 0;
}

/* ========== Logarithmic binning ========== */
/*
 * Bin edges: K_lo(b) = floor(ratio^b) + K_offset, K_hi(b) = K_lo(b+1) - 1
 * For small K (below ~50), bins are still width-1 (exact integer binning).
 * Above that, bins widen geometrically.
 *
 * We store per-bin accumulators and report the geometric mean of K_lo, K_hi
 * as the representative K for that bin.
 */

typedef struct {
  int n_bins;
  double ratio;  /* geometric growth factor */
  long *lo;      /* bin lower edge (inclusive) */
  long *hi;      /* bin upper edge (inclusive) */
  double *K_rep; /* representative K for output */

  long *count;
  double *sum_tri;
  double *sum_k;
  double *sum_N;
} LogBinnedStats;

static LogBinnedStats *logstats_create(int n_bins_hint, long K_max_est) {
  /*
   * Build bin edges: exact bins for K=2..49, then log bins above that.
   * n_bins_hint controls how many log-spaced bins we use for K >= 50.
   */
  LogBinnedStats *s = malloc(sizeof(LogBinnedStats));

  int exact_start = 2;
  int exact_end = 49; /* exact integer bins for K in [2, 49] */
  int n_exact = exact_end - exact_start + 1; /* 48 bins */

  /* Log bins from 50 to K_max_est */
  if (K_max_est < exact_end + 1)
    K_max_est = exact_end + 1;

  double log_lo = log((double)(exact_end + 1));
  double log_hi = log((double)K_max_est);
  if (log_hi <= log_lo)
    log_hi = log_lo + 1.0;

  int n_log = n_bins_hint;
  if (n_log < 20)
    n_log = 20;

  int n_total = n_exact + n_log;
  s->n_bins = n_total;
  s->lo = calloc(n_total, sizeof(long));
  s->hi = calloc(n_total, sizeof(long));
  s->K_rep = calloc(n_total, sizeof(double));
  s->count = calloc(n_total, sizeof(long));
  s->sum_tri = calloc(n_total, sizeof(double));
  s->sum_k = calloc(n_total, sizeof(double));
  s->sum_N = calloc(n_total, sizeof(double));

  /* Exact bins */
  for (int i = 0; i < n_exact; i++) {
    s->lo[i] = exact_start + i;
    s->hi[i] = exact_start + i;
    s->K_rep[i] = exact_start + i;
  }

  /* Log bins */
  double dlog = (log_hi - log_lo) / n_log;
  s->ratio = exp(dlog);
  for (int i = 0; i < n_log; i++) {
    long edge_lo = (long)floor(exp(log_lo + i * dlog));
    long edge_hi = (long)floor(exp(log_lo + (i + 1) * dlog)) - 1;
    if (edge_lo < exact_end + 1)
      edge_lo = exact_end + 1;
    if (edge_hi < edge_lo)
      edge_hi = edge_lo;

    int idx = n_exact + i;
    s->lo[idx] = edge_lo;
    s->hi[idx] = edge_hi;
    s->K_rep[idx] =
        sqrt((double)edge_lo * (double)edge_hi); /* geometric mean */
  }

  /* Fix overlaps: ensure lo[i] = hi[i-1]+1 for the log section */
  for (int i = n_exact + 1; i < n_total; i++) {
    if (s->lo[i] <= s->hi[i - 1])
      s->lo[i] = s->hi[i - 1] + 1;
    if (s->hi[i] < s->lo[i])
      s->hi[i] = s->lo[i];
    s->K_rep[i] = sqrt((double)s->lo[i] * (double)s->hi[i]);
  }

  return s;
}

static void logstats_free(LogBinnedStats *s) {
  if (s) {
    free(s->lo);
    free(s->hi);
    free(s->K_rep);
    free(s->count);
    free(s->sum_tri);
    free(s->sum_k);
    free(s->sum_N);
    free(s);
  }
}

/* Binary search for the bin containing K */
static int logstats_find_bin(const LogBinnedStats *s, long K) {
  if (K < s->lo[0] || K > s->hi[s->n_bins - 1])
    return -1;
  int lo = 0, hi = s->n_bins - 1;
  while (lo <= hi) {
    int mid = (lo + hi) / 2;
    if (K < s->lo[mid])
      hi = mid - 1;
    else if (K > s->hi[mid])
      lo = mid + 1;
    else
      return mid;
  }
  return -1; /* in a gap between bins (shouldn't happen with proper
                construction) */
}

static void logstats_add(LogBinnedStats *s, long K, double triangles, int k) {
  int b = logstats_find_bin(s, K);
  if (b < 0)
    return;
  s->count[b]++;
  s->sum_tri[b] += triangles;
  s->sum_k[b] += k;
  s->sum_N[b] += (K - k);
}

/* ========== Core simulation ========== */

static double expected_triangles(int k, const int *r, const int *n, double f) {
  (void)r; /* r_i values not needed; triangle count depends on k, n_i, f */
  int N = 0;
  for (int i = 0; i < k; i++)
    N += n[i];

  double type_A = f * k * (k - 1) / 2.0;
  double type_B = N;
  double type_C = 0.0;
  for (int i = 0; i < k; i++)
    type_C += f * n[i] * (n[i] - 1) / 2.0;

  return type_A + type_B + type_C;
}

/*
 * Sample one node's neighborhood and compute expected triangles.
 * Returns triangle count; sets *k_out and *K_out.
 *
 * Uses stack allocation for small k to avoid malloc in the hot loop.
 */
#define STACK_ALLOC_LIMIT 1024

static double sample_node_expected(const Backbone *bb, double f, int *k_out,
                                   long *K_out) {
  int k = sample_k(bb);
  *k_out = k;

  if (k == 0) {
    *K_out = 0;
    return 0.0;
  }

  int r_stack[STACK_ALLOC_LIMIT], n_stack[STACK_ALLOC_LIMIT];
  int *r, *n;
  int heap_alloc = 0;

  if (k <= STACK_ALLOC_LIMIT) {
    r = r_stack;
    n = n_stack;
  } else {
    r = malloc(k * sizeof(int));
    n = malloc(k * sizeof(int));
    heap_alloc = 1;
  }

  long N = 0;
  for (int i = 0; i < k; i++) {
    r[i] = sample_r(bb);
    n[i] = rng_binomial(r[i], f);
    N += n[i];
  }

  long K = k + N;
  *K_out = K;

  double tri = expected_triangles(k, r, n, f);

  if (heap_alloc) {
    free(r);
    free(n);
  }

  return tri;
}

/* ========== Main ========== */

void print_usage(const char *prog) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s ER <c> <f> [n_samples] [n_log_bins]\n", prog);
  fprintf(stderr, "  %s RRN <degree> <f> [n_samples] [n_log_bins]\n", prog);
  fprintf(stderr,
          "  %s PL <kmin> <gamma> <kmax> <f> [n_samples] [n_log_bins]\n", prog);
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr,
          "  n_samples   Number of Monte Carlo samples (default: 10000000)\n");
  fprintf(
      stderr,
      "  n_log_bins  Number of logarithmic bins for K >= 50 (default: 200)\n");
  fprintf(stderr, "\n");
  fprintf(stderr,
          "Output: Results saved to file with parameters in filename.\n");
}

static void generate_filename(char *filename, size_t size,
                              const char *backbone_type, double c, int degree,
                              int kmin, double gamma, int kmax, double f) {
  if (strcmp(backbone_type, "ER") == 0)
    snprintf(filename, size, "CK_ER_c%.2f_f%.2f.dat", c, f);
  else if (strcmp(backbone_type, "RRN") == 0)
    snprintf(filename, size, "CK_RRN_d%d_f%.2f.dat", degree, f);
  else if (strcmp(backbone_type, "PL") == 0)
    snprintf(filename, size, "CK_PL_kmin%d_gamma%.2f_kmax%d_f%.2f.dat", kmin,
             gamma, kmax, f);
  else
    snprintf(filename, size, "CK_output.dat");
}

int main(int argc, char **argv) {
  if (argc < 4) {
    print_usage(argv[0]);
    return 1;
  }

  rng_seed(time(NULL) ^ (long)&argc);

  Backbone *bb = NULL;
  double f = 0.0;
  long n_samples = 10000000;
  int n_log_bins = 200;

  char filename[256];
  double param_c = 0.0;
  int param_degree = 0;
  int param_kmin = 0, param_kmax = 0;
  double param_gamma = 0.0;

  if (strcmp(argv[1], "ER") == 0) {
    if (argc < 4) {
      print_usage(argv[0]);
      return 1;
    }
    double c = atof(argv[2]);
    f = atof(argv[3]);
    if (argc > 4)
      n_samples = atol(argv[4]);
    if (argc > 5)
      n_log_bins = atoi(argv[5]);
    bb = backbone_create_er(c);
    param_c = c;
    fprintf(stderr, "# ER backbone: c = %.4f\n", c);

  } else if (strcmp(argv[1], "RRN") == 0) {
    if (argc < 4) {
      print_usage(argv[0]);
      return 1;
    }
    int degree = atoi(argv[2]);
    f = atof(argv[3]);
    if (argc > 4)
      n_samples = atol(argv[4]);
    if (argc > 5)
      n_log_bins = atoi(argv[5]);
    bb = backbone_create_rrn(degree);
    param_degree = degree;
    fprintf(stderr, "# RRN backbone: degree = %d\n", degree);

  } else if (strcmp(argv[1], "PL") == 0) {
    if (argc < 6) {
      print_usage(argv[0]);
      return 1;
    }
    int kmin = atoi(argv[2]);
    double gamma = atof(argv[3]);
    int kmax = atoi(argv[4]);
    f = atof(argv[5]);
    if (argc > 6)
      n_samples = atol(argv[6]);
    if (argc > 7)
      n_log_bins = atoi(argv[7]);
    bb = backbone_create_pl(kmin, gamma, kmax);
    param_kmin = kmin;
    param_gamma = gamma;
    param_kmax = kmax;
    fprintf(stderr, "# PL backbone: kmin = %d, gamma = %.2f, kmax = %d\n", kmin,
            gamma, kmax);
    fprintf(stderr, "# <k> = %.4f, <r> = %.4f\n", bb->mean_k, bb->mean_r);

  } else {
    fprintf(stderr, "Unknown backbone: %s\n", argv[1]);
    print_usage(argv[0]);
    return 1;
  }

  if (!bb) {
    fprintf(stderr, "Failed to create backbone\n");
    return 1;
  }

  fprintf(stderr, "# f = %.4f\n", f);
  fprintf(stderr, "# n_samples = %ld, n_log_bins = %d\n", n_samples,
          n_log_bins);

  /*
   * Pre-sampling pass: draw a small fraction of samples to discover the
   * actual K range empirically.  This avoids any fragile analytical estimate
   * of K_max, which fails for both gamma < 3 (divergent <r>) and gamma > 3
   * with large kmax (rare but reachable high-K samples).
   *
   * We use 1% of n_samples (at least 100k) for the pre-pass.  The RNG state
   * is saved and restored so the main pass is statistically independent.
   */
  long n_presample = n_samples / 20;
  if (n_presample < 200000)
    n_presample = 200000;
  if (n_presample > n_samples)
    n_presample = n_samples;

  /* Save RNG state */
  unsigned long rng_save[4];
  memcpy(rng_save, rng_state, sizeof(rng_state));

  /* Seed a separate stream for pre-sampling */
  rng_seed(12345678UL ^ (unsigned long)n_samples);

  long K_max_seen_pre = 0;
  fprintf(stderr, "# Pre-sampling %ld nodes to discover K range...\n",
          n_presample);
  for (long s = 0; s < n_presample; s++) {
    int k;
    long K;
    sample_node_expected(bb, f, &k, &K);
    if (K > K_max_seen_pre)
      K_max_seen_pre = K;
  }
  /* Add 2x headroom so the main pass (which draws more samples) is unlikely
     to exceed the bin range */
  long K_max_est = (long)(K_max_seen_pre * 2.0) + 100;

  /* Restore RNG state for the main pass */
  memcpy(rng_state, rng_save, sizeof(rng_state));

  fprintf(stderr, "# Pre-sample K_max_seen = %ld, using K_max_est = %ld\n",
          K_max_seen_pre, K_max_est);
  fprintf(stderr, "# Running Monte Carlo...\n");

  LogBinnedStats *stats = logstats_create(n_log_bins, K_max_est);

  long total_samples = 0;
  long K_max_seen = 0;

  for (long s = 0; s < n_samples; s++) {
    int k;
    long K;
    double tri = sample_node_expected(bb, f, &k, &K);

    logstats_add(stats, K, tri, k);
    total_samples++;
    if (K > K_max_seen)
      K_max_seen = K;

    if ((s + 1) % 1000000 == 0)
      fprintf(stderr, "\r# Progress: %.1f%%", 100.0 * (s + 1) / n_samples);
  }
  fprintf(stderr, "\r# Done: %ld samples, K_max_seen = %ld       \n",
          total_samples, K_max_seen);

  /* Total count for P(K) */
  long total_count = 0;
  for (int b = 0; b < stats->n_bins; b++)
    total_count += stats->count[b];

  /* Generate filename and open output file */
  generate_filename(filename, sizeof(filename), argv[1], param_c, param_degree,
                    param_kmin, param_gamma, param_kmax, f);

  FILE *outfile = fopen(filename, "w");
  if (!outfile) {
    fprintf(stderr, "Error: could not open %s for writing\n", filename);
    logstats_free(stats);
    backbone_free(bb);
    return 1;
  }

  fprintf(stderr, "# Writing results to: %s\n", filename);

  /* Header */
  fprintf(outfile,
          "# STC Clustering Spectrum C(K) via Monte Carlo (log-binned)\n");
  fprintf(outfile, "# Backbone: %s\n", argv[1]);
  fprintf(outfile, "# f = %.6f\n", f);
  fprintf(outfile, "# n_samples = %ld\n", n_samples);
  fprintf(outfile, "#\n");
  fprintf(outfile, "# K_rep      K_lo     K_hi      count       P(K)*dK        "
                   "C(K)           C(K)*K         E[k|K]         E[N|K]\n");

  /* Also print to stdout */
  printf("# STC Clustering Spectrum C(K) via Monte Carlo (log-binned)\n");
  printf("# Backbone: %s\n", argv[1]);
  printf("# f = %.6f\n", f);
  printf("# n_samples = %ld\n", n_samples);
  printf("#\n");
  printf("# K_rep      K_lo     K_hi      count       P(K)*dK        C(K)      "
         "     C(K)*K         E[k|K]         E[N|K]\n");

  int min_count = 10;

  for (int b = 0; b < stats->n_bins; b++) {
    if (stats->count[b] < min_count)
      continue;

    double K_rep = stats->K_rep[b];
    long K_lo = stats->lo[b];
    long K_hi = stats->hi[b];

    double PK_dK =
        (double)stats->count[b] / total_count; /* integrated P(K) over bin */
    double E_tri = stats->sum_tri[b] / stats->count[b];
    double n_triads = K_rep * (K_rep - 1) / 2.0;
    double CK = (n_triads > 0) ? E_tri / n_triads : 0.0;
    double E_k = stats->sum_k[b] / stats->count[b];
    double E_N = stats->sum_N[b] / stats->count[b];

    fprintf(outfile,
            "%10.1f %8ld %8ld %10ld   %.6e   %.6e   %.6e   %12.4f   %12.4f\n",
            K_rep, K_lo, K_hi, stats->count[b], PK_dK, CK, CK * K_rep, E_k,
            E_N);
    printf("%10.1f %8ld %8ld %10ld   %.6e   %.6e   %.6e   %12.4f   %12.4f\n",
           K_rep, K_lo, K_hi, stats->count[b], PK_dK, CK, CK * K_rep, E_k, E_N);
  }

  fclose(outfile);
  logstats_free(stats);
  backbone_free(bb);

  return 0;
}