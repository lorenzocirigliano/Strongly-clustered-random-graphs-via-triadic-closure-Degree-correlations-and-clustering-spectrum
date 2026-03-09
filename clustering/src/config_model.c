/*
 * config_model.c - Configuration model implementation
 */

#include "config_model.h"
#include "rng.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
 * Generate graph using configuration model with repeated matching.
 * Uses the "erased" configuration model: multi-edges and self-loops are removed.
 */
Graph *config_model_generate(int n, const int *degrees, int max_attempts) {
    /* Count total stubs (half-edges) */
    long total_stubs = 0;
    for (int i = 0; i < n; i++) {
        total_stubs += degrees[i];
    }
    
    if (total_stubs % 2 != 0) {
        fprintf(stderr, "Warning: odd total degree, adding 1 to node 0\n");
        total_stubs++;
    }
    
    /* Create stub array: stub[s] = node that owns stub s */
    int *stubs = malloc(total_stubs * sizeof(int));
    if (!stubs) return NULL;
    
    int idx = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < degrees[i]; j++) {
            stubs[idx++] = i;
        }
    }
    /* Handle odd degree case */
    while (idx < total_stubs) {
        stubs[idx++] = 0;
    }
    
    /* Allocate graph */
    Graph *g = graph_alloc(n, total_stubs / 2);
    if (!g) {
        free(stubs);
        return NULL;
    }
    
    /* Randomly match stubs */
    /* Fisher-Yates shuffle and pair consecutive elements */
    for (long i = total_stubs - 1; i > 0; i--) {
        long j = (long)(rng_uniform() * (i + 1));
        int tmp = stubs[i];
        stubs[i] = stubs[j];
        stubs[j] = tmp;
    }
    
    /* Create edges from consecutive pairs */
    for (long i = 0; i < total_stubs; i += 2) {
        int u = stubs[i];
        int v = stubs[i + 1];
        
        /* Skip self-loops */
        if (u == v) continue;
        
        /* Skip multi-edges (expensive check, but necessary for correctness) */
        /* For efficiency, we only check in the edge list during construction */
        int is_multi = 0;
        for (int e = 0; e < g->m; e++) {
            if ((g->edge_u[e] == u && g->edge_v[e] == v) ||
                (g->edge_u[e] == v && g->edge_v[e] == u)) {
                is_multi = 1;
                break;
            }
        }
        
        if (!is_multi) {
            graph_add_edge(g, u, v);
        }
    }
    
    free(stubs);
    graph_finalize(g);
    
    return g;
}

int *degree_seq_poisson(int n, double c) {
    int *degrees = malloc(n * sizeof(int));
    if (!degrees) return NULL;
    
    long total = 0;
    for (int i = 0; i < n; i++) {
        degrees[i] = rng_poisson(c);
        total += degrees[i];
    }
    
    /* Ensure even total */
    if (total % 2 == 1) {
        degrees[rng_int(n)]++;
    }
    
    return degrees;
}

int *degree_seq_regular(int n, int c) {
    if ((long)n * c % 2 != 0) {
        fprintf(stderr, "Error: n*c must be even for regular graph\n");
        return NULL;
    }
    
    int *degrees = malloc(n * sizeof(int));
    if (!degrees) return NULL;
    
    for (int i = 0; i < n; i++) {
        degrees[i] = c;
    }
    
    return degrees;
}

int *degree_seq_powerlaw(int n, int kmin, double gamma, int kmax) {
    if (kmin <= 0 || gamma <= 2.0 || kmax < kmin) {
        fprintf(stderr, "Error: invalid power-law parameters\n");
        return NULL;
    }
    
    /* Precompute CDF for power-law distribution */
    int range = kmax - kmin + 1;
    double *cdf = malloc(range * sizeof(double));
    if (!cdf) return NULL;
    
    double Z = 0.0;
    for (int k = kmin; k <= kmax; k++) {
        Z += pow(k, -gamma);
    }
    
    double cumsum = 0.0;
    for (int i = 0; i < range; i++) {
        cumsum += pow(kmin + i, -gamma) / Z;
        cdf[i] = cumsum;
    }
    
    /* Sample degrees */
    int *degrees = malloc(n * sizeof(int));
    if (!degrees) {
        free(cdf);
        return NULL;
    }
    
    long total = 0;
    for (int i = 0; i < n; i++) {
        double u = rng_uniform();
        /* Binary search for inverse CDF */
        int lo = 0, hi = range - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (cdf[mid] < u) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        degrees[i] = kmin + lo;
        total += degrees[i];
    }
    
    /* Ensure even total */
    if (total % 2 == 1) {
        /* Increase a random node's degree */
        int idx = rng_int(n);
        if (degrees[idx] < kmax) {
            degrees[idx]++;
        } else {
            /* Find a node that can be increased */
            for (int i = 0; i < n; i++) {
                if (degrees[i] < kmax) {
                    degrees[i]++;
                    break;
                }
            }
        }
    }
    
    free(cdf);
    return degrees;
}
