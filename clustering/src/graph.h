/*
 * graph.h - Simple sparse graph representation
 *
 * Uses adjacency list representation with edge list for efficient
 * neighbor iteration and edge enumeration.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>

typedef struct {
    int n;              /* Number of nodes */
    int m;              /* Number of edges */
    int m_alloc;        /* Allocated edge capacity */
    
    /* Adjacency list representation */
    int *degree;        /* degree[i] = degree of node i */
    int *adj_ptr;       /* adj_ptr[i] = start index in adj_list for node i */
    int *adj_list;      /* Concatenated adjacency lists */
    int adj_alloc;      /* Allocated size of adj_list */
    
    /* Edge list representation (for iteration) */
    int *edge_u;        /* edge_u[e] = first endpoint of edge e */
    int *edge_v;        /* edge_v[e] = second endpoint of edge e */
} Graph;

/* Allocate an empty graph with n nodes */
Graph *graph_alloc(int n, int expected_edges);

/* Free a graph */
void graph_free(Graph *g);

/* Clear all edges (keep node count) */
void graph_clear(Graph *g);

/* Add an undirected edge (u, v). Returns 1 if added, 0 if already exists */
int graph_add_edge(Graph *g, int u, int v);

/* Check if edge (u, v) exists */
int graph_has_edge(const Graph *g, int u, int v);

/* Finalize adjacency list structure after all edges added */
void graph_finalize(Graph *g);

/* Get degree of node i */
static inline int graph_degree(const Graph *g, int i) {
    return g->degree[i];
}

/* Get pointer to neighbors of node i (after finalize) */
static inline const int *graph_neighbors(const Graph *g, int i) {
    return &g->adj_list[g->adj_ptr[i]];
}

/* Print graph statistics */
void graph_print_stats(const Graph *g);

#endif /* GRAPH_H */
