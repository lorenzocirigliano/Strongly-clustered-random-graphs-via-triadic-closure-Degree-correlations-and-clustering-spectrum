/*============================================================
 * triad_omp.c
 *
 * OpenMP-parallelized triad closure simulation.
 * Each gamma value runs independently on its own core.
 *
 * Structural cutoff: kmax = min(N^{1/(gamma-1)}, N^{1/2})
 *
 * Compile:
 *   gcc -O3 -march=native -fopenmp -o triad_omp triad_omp.c -lm
 *
 * Run:
 *   ./triad_omp              (uses 6 cores, one per gamma)
 *   OMP_NUM_THREADS=4 ./triad_omp   (limit to 4 cores)
 *============================================================*/

#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================
 * Per-thread xoshiro256** PRNG
 *
 * Each thread owns its own state, seeded independently.
 * No shared mutable state, no false sharing.
 * ========================================================= */
typedef struct {
  uint64_t s[4];
} RNG;

static inline uint64_t rotl(uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

static inline uint64_t rng_next(RNG *r) {
  uint64_t result = rotl(r->s[1] * 5, 7) * 9;
  uint64_t t = r->s[1] << 17;
  r->s[2] ^= r->s[0];
  r->s[3] ^= r->s[1];
  r->s[1] ^= r->s[2];
  r->s[0] ^= r->s[3];
  r->s[2] ^= t;
  r->s[3] = rotl(r->s[3], 45);
  return result;
}

static void rng_seed(RNG *r, uint64_t seed) {
  for (int i = 0; i < 4; i++) {
    seed += 0x9e3779b97f4a7c15ULL;
    uint64_t z = seed;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    r->s[i] = z ^ (z >> 31);
  }
}

static inline double U01(RNG *r) { return (rng_next(r) >> 11) * 0x1.0p-53; }

static double randn(RNG *r) {
  double u1, u2;
  do {
    u1 = U01(r);
  } while (u1 < 1e-300);
  u2 = U01(r);
  return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

/* =========================================================
 * Binomial sampler (thread-safe: takes RNG pointer)
 * ========================================================= */
static inline int binom(RNG *r, int n, double p) {
  if (n <= 0 || p <= 0.0)
    return 0;
  if (p >= 1.0)
    return n;
  if (n < 80) {
    int c = 0;
    for (int i = 0; i < n; i++)
      c += (U01(r) < p);
    return c;
  }
  double mu = n * p, sig = sqrt(mu * (1.0 - p));
  int v = (int)(mu + sig * randn(r) + 0.5);
  return v < 0 ? 0 : (v > n ? n : v);
}

/* =========================================================
 * Discrete power law with CDF table
 * ========================================================= */
typedef struct {
  double *cdf;
  int kmin, kmax, len;
} PL;

static PL build_pl(double alpha, int kmin, int kmax) {
  PL d;
  d.kmin = kmin;
  d.kmax = kmax;
  d.len = kmax - kmin + 1;
  d.cdf = (double *)malloc(d.len * sizeof(double));
  if (!d.cdf) {
    fprintf(stderr, "OOM\n");
    exit(1);
  }
  double s = 0;
  for (int i = 0; i < d.len; i++) {
    s += pow(kmin + i, -alpha);
    d.cdf[i] = s;
  }
  for (int i = 0; i < d.len; i++)
    d.cdf[i] /= s;
  return d;
}

static inline int spl(RNG *r, const PL *d) {
  double u = U01(r);
  int lo = 0, hi = d->len - 1;
  while (lo < hi) {
    int m = (lo + hi) >> 1;
    if (d->cdf[m] < u)
      lo = m + 1;
    else
      hi = m;
  }
  return d->kmin + lo;
}

static void free_pl(PL *d) { free(d->cdf); }

/* =========================================================
 * Log-binned histogram
 * ========================================================= */
typedef struct {
  double *cnt, *blo, *bhi;
  int nb;
  double ratio, b0;
} LH;

static LH lh_new(double b0, double bmax, double ratio) {
  LH h;
  h.ratio = ratio;
  h.b0 = b0;
  h.nb = (int)(log(bmax / b0) / log(ratio)) + 2;
  h.cnt = (double *)calloc(h.nb, sizeof(double));
  h.blo = (double *)malloc(h.nb * sizeof(double));
  h.bhi = (double *)malloc(h.nb * sizeof(double));
  if (!h.cnt || !h.blo || !h.bhi) {
    fprintf(stderr, "OOM\n");
    exit(1);
  }
  for (int i = 0; i < h.nb; i++) {
    h.blo[i] = b0 * pow(ratio, i);
    h.bhi[i] = b0 * pow(ratio, i + 1);
  }
  return h;
}

static inline void lh_add(LH *h, long v) {
  if (v < (long)h->b0)
    return;
  int b = (int)(log((double)v / h->b0) / log(h->ratio));
  if (b >= 0 && b < h->nb)
    h->cnt[b] += 1.0;
}

static void lh_free(LH *h) {
  free(h->cnt);
  free(h->blo);
  free(h->bhi);
}

/* =========================================================
 * Structural cutoff
 * ========================================================= */
static int compute_kmax(int N, double gamma, int kmin) {
  double km_nat = pow((double)N, 1.0 / (gamma - 1.0));
  double km_str = sqrt((double)N);
  int km = (int)fmin(km_nat, km_str);
  if (km < kmin + 1)
    km = kmin + 1;
  return km;
}

/* =========================================================
 * Moments of p(k)
 * ========================================================= */
static void compute_moments(double gamma, int kmin, int kmax, double *mean_k,
                            double *mean_kk1) {
  double Z = 0, s1 = 0, s2 = 0;
  for (int k = kmin; k <= kmax; k++) {
    double w = pow((double)k, -gamma);
    Z += w;
    s1 += k * w;
    s2 += (double)k * (k - 1) * w;
  }
  *mean_k = s1 / Z;
  *mean_kk1 = s2 / Z;
}

/* =========================================================
 * Per-gamma work: histogram + Kmax scaling
 *
 * Completely self-contained. Each call writes its own files
 * and returns a results buffer for kmax_scaling.dat.
 * ========================================================= */
typedef struct {
  double gamma;
  int N;
  int kmax;
  double Kmax_mean;
  double Kmax_std;
  double exp_pred;
  const char *cutoff;
} KmaxEntry;

static void run_gamma(double gamma, double f, int kmin, double bin_ratio,
                      int N_hist, int R_hist, const int *Nv, int nN, int R_km,
                      KmaxEntry *km_out, int *km_count) {
  /* Thread-local RNG seeded from gamma to ensure reproducibility
   * and independence across threads. */
  RNG rng;
  uint64_t seed = (uint64_t)(gamma * 1e9) ^ 0xDEADBEEF12345ULL;
  rng_seed(&rng, seed);

  int tid = omp_get_thread_num();

  const char *cutoff = (gamma >= 3.0) ? "natural" : "structural";
  double ep = (gamma >= 3.0) ? 1.0 / (gamma - 1.0) : (4.0 - gamma) / 2.0;

  printf("[thread %d] gamma=%.2f  pred_exp=%.4f (%s cutoff)\n", tid, gamma, ep,
         cutoff);

  /* ----------------------------------------------------------
   * PART 1: P(K) histogram
   * ---------------------------------------------------------- */
  {
    int N = N_hist;
    int km = compute_kmax(N, gamma, kmin);
    PL pk = build_pl(gamma, kmin, km);
    PL qj = build_pl(gamma - 1.0, kmin, km);

    double mk, mkk1;
    compute_moments(gamma, kmin, km, &mk, &mkk1);
    double cN = 1.0 + f * mkk1 / mk;
    long ub = (long)(cN * km * 2.5) + 100;

    double km_nat = pow((double)N, 1.0 / (gamma - 1.0));
    double km_str = sqrt((double)N);

    printf("[thread %d] gamma=%.2f  N=%d  kmax=%d  "
           "(nat=%.0f, str=%.0f)  c(N)=%.2f\n",
           tid, gamma, N, km, km_nat, km_str, cN);

    LH hist = lh_new(1.0, (double)ub, bin_ratio);

    for (int r = 0; r < R_hist; r++) {
      for (int i = 0; i < N; i++) {
        int k = spl(&rng, &pk);
        long K = k;
        for (int nb = 0; nb < k; nb++) {
          int j = spl(&rng, &qj);
          K += binom(&rng, j - 1, f);
        }
        lh_add(&hist, K);
      }
    }

    /* Write P(K) file */
    char fn[256];
    snprintf(fn, 256, "pk_gamma%.2f_f%.2f.dat", gamma, f);
    FILE *fp = fopen(fn, "w");
    fprintf(fp, "# P(K) — structural cutoff (OpenMP)\n");
    fprintf(fp, "# gamma=%.2f  f=%.3f  N=%d  R=%d\n", gamma, f, N, R_hist);
    fprintf(fp,
            "# kmax=%d  (natural=%.0f, structural=%.0f)  "
            "cutoff=%s\n",
            km, km_nat, km_str, cutoff);
    fprintf(fp, "# <k>=%.4f  <k(k-1)>=%.4f  c(N)=%.4f\n", mk, mkk1, cN);
    fprintf(fp, "# f*kmax=%.0f  c*kmax=%.0f\n", f * km, cN * km);
    fprintf(fp, "#\n");
    fprintf(fp, "# K_center  P(K)  count\n");

    double tot = (double)N * R_hist;
    for (int b = 0; b < hist.nb; b++) {
      if (hist.cnt[b] > 0) {
        double Kc = sqrt(hist.blo[b] * hist.bhi[b]);
        double dK = hist.bhi[b] - hist.blo[b];
        fprintf(fp, "%.6e  %.6e  %.0f\n", Kc, hist.cnt[b] / (tot * dK),
                hist.cnt[b]);
      }
    }
    fclose(fp);
    printf("[thread %d] gamma=%.2f  -> %s\n", tid, gamma, fn);

    lh_free(&hist);
    free_pl(&pk);
    free_pl(&qj);
  }

  /* ----------------------------------------------------------
   * PART 2: K_max scaling
   * ---------------------------------------------------------- */
  int idx = 0;
  for (int ni = 0; ni < nN; ni++) {
    int N = Nv[ni];
    int km = compute_kmax(N, gamma, kmin);
    PL pk = build_pl(gamma, kmin, km);
    PL qj = build_pl(gamma - 1.0, kmin, km);

    double sK = 0, sK2 = 0;
    for (int r = 0; r < R_km; r++) {
      long Mx = 0;
      for (int i = 0; i < N; i++) {
        int k = spl(&rng, &pk);
        long K = k;
        for (int nb = 0; nb < k; nb++) {
          int j = spl(&rng, &qj);
          K += binom(&rng, j - 1, f);
        }
        if (K > Mx)
          Mx = K;
      }
      sK += (double)Mx;
      sK2 += (double)Mx * Mx;
    }
    double mK = sK / R_km;
    double v = sK2 / R_km - mK * mK;
    double sd = v > 0 ? sqrt(v) : 0;

    km_out[idx].gamma = gamma;
    km_out[idx].N = Nv[ni];
    km_out[idx].kmax = km;
    km_out[idx].Kmax_mean = mK;
    km_out[idx].Kmax_std = sd;
    km_out[idx].exp_pred = ep;
    km_out[idx].cutoff = cutoff;
    idx++;

    printf("[thread %d] gamma=%.2f  N=%-8d  km=%-6d  "
           "Kmax=%10.1f +/- %.1f\n",
           tid, gamma, N, km, mK, sd);

    free_pl(&pk);
    free_pl(&qj);
  }
  *km_count = idx;
}

/* =========================================================
 * Main
 * ========================================================= */
int main(void) {
  /* ---------- Parameters (edit these) ---------- */
  double gammas[] = {2.25, 2.50, 2.75, 3.00, 3.50, 4.50};
  int ng = 6;
  double f = 1;
  int kmin = 3;
  double bin_ratio = 1.15;

  /* Histogram */
  int N_hist = 100000000;
  int R_hist = 5;

  /* K_max scaling */
  int Nv[] = {10000, 100000, 1000000, 10000000, 100000000};
  int nN = 5;
  int R_km = 10;

  /* ---------- Allocate per-gamma result buffers ---------- */
  /* Each gamma produces nN entries for kmax_scaling */
  KmaxEntry *all_km = (KmaxEntry *)calloc(ng * nN, sizeof(KmaxEntry));
  int *counts = (int *)calloc(ng, sizeof(int));

  double t0 = omp_get_wtime();

  printf("Running with up to %d OpenMP threads\n\n", omp_get_max_threads());

/* ---------- Parallel loop: one gamma per thread ---------- */
#pragma omp parallel for schedule(dynamic, 1) num_threads(ng)
  for (int gi = 0; gi < ng; gi++) {
    run_gamma(gammas[gi], f, kmin, bin_ratio, N_hist, R_hist, Nv, nN, R_km,
              &all_km[gi * nN], &counts[gi]);
  }

  /* ---------- Write combined kmax_scaling.dat (serial) ---------- */
  FILE *fkm = fopen("kmax_scaling.dat", "w");
  fprintf(fkm,
          "# K_max scaling — structural cutoff "
          "(OpenMP, %d threads)\n",
          omp_get_max_threads());
  fprintf(fkm, "# kmax = min(N^{1/(gamma-1)}, N^{1/2})\n");
  fprintf(fkm, "# f=%.3f  kmin=%d  R=%d\n", f, kmin, R_km);
  fprintf(fkm, "#\n");
  fprintf(fkm, "# gamma    N          kmax      Kmax_mean"
               "       Kmax_std      exp_pred  cutoff\n");

  for (int gi = 0; gi < ng; gi++) {
    fprintf(fkm, "\n");
    for (int j = 0; j < counts[gi]; j++) {
      KmaxEntry *e = &all_km[gi * nN + j];
      fprintf(fkm,
              "%-8.2f  %-10d  %-8d  %-14.2f  "
              "%-12.2f  %.4f  %s\n",
              e->gamma, e->N, e->kmax, e->Kmax_mean, e->Kmax_std, e->exp_pred,
              e->cutoff);
    }
  }
  fclose(fkm);

  double t1 = omp_get_wtime();
  printf("\nDone. Wall time: %.1f s\n", t1 - t0);
  printf("Wrote kmax_scaling.dat + pk_gamma*.dat\n");

  free(all_km);
  free(counts);
  return 0;
}