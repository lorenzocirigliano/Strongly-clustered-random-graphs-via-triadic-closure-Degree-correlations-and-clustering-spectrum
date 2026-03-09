/*
 * graph.c - Simple sparse graph implementation
 */

#include "graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Graph *graph_alloc(int n, int expected_edges) {
    Graph *g = malloc(sizeof(Graph));
    if (!g) return NULL;
    
    g->n = n;
    g->m = 0;
    g->m_alloc = (expected_edges > 0) ? expected_edges : n;
    
    g->degree = calloc(n, sizeof(int));
    g->adj_ptr = calloc(n + 1, sizeof(int));
    g->adj_list = NULL;
    g->adj_alloc = 0;
    
    g->edge_u = malloc(g->m_alloc * sizeof(int));
    g->edge_v = malloc(g->m_alloc * sizeof(int));
    
    if (!g->degree || !g->adj_ptr || !g->edge_u || !g->edge_v) {
        graph_free(g);
        return NULL;
    }
    
    return g;
}

void graph_free(Graph *g) {
    if (g) {
        free(g->degree);
        free(g->adj_ptr);
        free(g->adj_list);
        free(g->edge_u);
        free(g->edge_v);
        free(g);
    }
}

void graph_clear(Graph *g) {
    g->m = 0;
    memset(g->degree, 0, g->n * sizeof(int));
}

static void ensure_edge_capacity(Graph *g, int needed) {
    if (needed <= g->m_alloc) return;
    
    int new_alloc = g->m_alloc * 2;
    if (new_alloc < needed) new_alloc = needed;
    
    g->edge_u = realloc(g->edge_u, new_alloc * sizeof(int));
    g->edge_v = realloc(g->edge_v, new_alloc * sizeof(int));
    g->m_alloc = new_alloc;
}

int graph_has_edge(const Graph *g, int u, int v) {
    /* If adjacency list is finalized, use it for O(degree) lookup */
    if (g->adj_list) {
        const int *neighbors = graph_neighbors(g, u);
        int deg = g->degree[u];
        for (int i = 0; i < deg; i++) {
            if (neighbors[i] == v) return 1;
        }
        return 0;
    }
    
    /* Otherwise scan edge list - O(m) */
    for (int e = 0; e < g->m; e++) {
        if ((g->edge_u[e] == u && g->edge_v[e] == v) ||
            (g->edge_u[e] == v && g->edge_v[e] == u)) {
            return 1;
        }
    }
    return 0;
}

int graph_add_edge(Graph *g, int u, int v) {
    if (u == v) return 0;  /* No self-loops */
    if (u < 0 || u >= g->n || v < 0 || v >= g->n) return 0;
    
    ensure_edge_capacity(g, g->m + 1);
    
    g->edge_u[g->m] = u;
    g->edge_v[g->m] = v;
    g->m++;
    
    g->degree[u]++;
    g->degree[v]++;
    
    return 1;
}

void graph_finalize(Graph *g) {
    /* Build adjacency list from edge list */
    int total_adj = 2 * g->m;
    
    if (g->adj_alloc < total_adj) {
        free(g->adj_list);
        g->adj_list = malloc(total_adj * sizeof(int));
        g->adj_alloc = total_adj;
    }
    
    /* Compute adj_ptr (prefix sum of degrees) */
    g->adj_ptr[0] = 0;
    for (int i = 0; i < g->n; i++) {
        g->adj_ptr[i + 1] = g->adj_ptr[i] + g->degree[i];
    }
    
    /* Temporary array to track fill position */
    int *fill = calloc(g->n, sizeof(int));
    
    /* Fill adjacency list */
    for (int e = 0; e < g->m; e++) {
        int u = g->edge_u[e];
        int v = g->edge_v[e];
        
        g->adj_list[g->adj_ptr[u] + fill[u]++] = v;
        g->adj_list[g->adj_ptr[v] + fill[v]++] = u;
    }
    
    free(fill);
}

void graph_print_stats(const Graph *g) {
    printf("Graph: n=%d, m=%d\n", g->n, g->m);
    
    /* Compute degree statistics */
    int min_deg = g->n, max_deg = 0;
    long sum_deg = 0;
    
    for (int i = 0; i < g->n; i++) {
        int d = g->degree[i];
        if (d < min_deg) min_deg = d;
        if (d > max_deg) max_deg = d;
        sum_deg += d;
    }
    
    printf("  Degrees: min=%d, max=%d, mean=%.2f\n", 
           min_deg, max_deg, (double)sum_deg / g->n);
}
