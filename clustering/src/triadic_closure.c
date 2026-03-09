/*
 * triadic_closure.c - Static Triadic Closure implementation
 */

#include "triadic_closure.h"
#include "rng.h"
#include <stdlib.h>
#include <string.h>

/*
 * Helper: Check if edge exists using hash set for O(1) lookup.
 * For simplicity, we use a sorted adjacency list with binary search.
 */
static int has_edge_sorted(const int *neighbors, int deg, int target) {
    int lo = 0, hi = deg - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (neighbors[mid] == target) return 1;
        if (neighbors[mid] < target) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return 0;
}

static int cmp_int(const void *a, const void *b) {
    return (*(const int *)a) - (*(const int *)b);
}

Graph *triadic_closure(const Graph *backbone, double f) {
    int n = backbone->n;
    
    /* Estimate number of edges: original + expected new */
    /* Expected new edges ≈ f * (number of triads) / 2 */
    /* Number of triads ≈ sum of C(d_i, 2) = sum of d_i(d_i-1)/2 */
    long expected_new = 0;
    for (int i = 0; i < n; i++) {
        int d = backbone->degree[i];
        expected_new += (long)d * (d - 1) / 2;
    }
    expected_new = (long)(f * expected_new) + backbone->m;
    
    Graph *gf = graph_alloc(n, (int)expected_new);
    if (!gf) return NULL;
    
    if (triadic_closure_inplace(backbone, f, gf) < 0) {
        graph_free(gf);
        return NULL;
    }
    
    return gf;
}

int triadic_closure_inplace(const Graph *backbone, double f, Graph *output) {
    int n = backbone->n;
    
    if (output->n != n) {
        return -1;  /* Size mismatch */
    }
    
    graph_clear(output);
    
    /* Copy all backbone edges */
    for (int e = 0; e < backbone->m; e++) {
        graph_add_edge(output, backbone->edge_u[e], backbone->edge_v[e]);
    }
    
    /* 
     * For each node j, consider all pairs of its neighbors (i, k).
     * The triad (i, j, k) can be closed by adding edge (i, k).
     * 
     * To avoid double-counting triads, we only process when j < i and j < k,
     * or equivalently, we process each triad exactly once by requiring
     * that j is the "center" with smallest index.
     * 
     * Actually, simpler: for each node j, for each pair of neighbors (i, k)
     * with i < k, attempt to close. But we need to check the triad hasn't
     * been closed from another center. Since the backbone is treelike,
     * each open triad has a unique center, so this is fine.
     *
     * For non-treelike backbones, we need to be careful not to double-process.
     * Here we take the approach: for each triad (i, j, k) centered at j,
     * we only attempt closure if the edge (i, k) doesn't already exist.
     */
    
    /* First, finalize output so we can check for existing edges */
    graph_finalize(output);
    
    /* Create sorted adjacency lists for fast edge lookup in output */
    /* We'll sort the adjacency list of output */
    for (int i = 0; i < n; i++) {
        int deg = output->degree[i];
        if (deg > 1) {
            qsort(&output->adj_list[output->adj_ptr[i]], deg, sizeof(int), cmp_int);
        }
    }
    
    /* Collect new edges to add (can't add while iterating) */
    int new_capacity = 1000;
    int new_count = 0;
    int *new_u = malloc(new_capacity * sizeof(int));
    int *new_v = malloc(new_capacity * sizeof(int));
    
    if (!new_u || !new_v) {
        free(new_u);
        free(new_v);
        return -1;
    }
    
    /* Iterate over all triads in backbone */
    for (int j = 0; j < n; j++) {
        const int *neighbors_j = graph_neighbors(backbone, j);
        int deg_j = backbone->degree[j];
        
        /* For each pair of neighbors (i, k) with i < k */
        for (int a = 0; a < deg_j; a++) {
            int i = neighbors_j[a];
            for (int b = a + 1; b < deg_j; b++) {
                int k = neighbors_j[b];
                
                /* Check if edge (i, k) already exists in output */
                int deg_i = output->degree[i];
                const int *neighbors_i = graph_neighbors(output, i);
                
                if (has_edge_sorted(neighbors_i, deg_i, k)) {
                    continue;  /* Edge already exists */
                }
                
                /* Attempt to close triad with probability f */
                if (rng_uniform() < f) {
                    /* Add to list of new edges */
                    if (new_count >= new_capacity) {
                        new_capacity *= 2;
                        new_u = realloc(new_u, new_capacity * sizeof(int));
                        new_v = realloc(new_v, new_capacity * sizeof(int));
                    }
                    new_u[new_count] = i;
                    new_v[new_count] = k;
                    new_count++;
                }
            }
        }
    }
    
    /* Add all new edges */
    for (int e = 0; e < new_count; e++) {
        graph_add_edge(output, new_u[e], new_v[e]);
    }
    
    free(new_u);
    free(new_v);
    
    /* Finalize the output graph */
    graph_finalize(output);
    
    return 0;
}
