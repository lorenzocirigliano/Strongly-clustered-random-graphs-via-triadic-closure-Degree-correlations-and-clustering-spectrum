/*
 * config_model.h - Configuration model graph generation
 *
 * Generates random graphs with prescribed degree sequence using
 * the configuration model (random matching of half-edges).
 */

#ifndef CONFIG_MODEL_H
#define CONFIG_MODEL_H

#include "graph.h"

/* 
 * Generate a graph using the configuration model.
 * 
 * Parameters:
 *   n         - number of nodes
 *   degrees   - array of length n with desired degree for each node
 *   max_attempts - maximum matching attempts before giving up on multi-edges
 *
 * Returns:
 *   A new Graph, or NULL on failure.
 *   The graph may have slightly different degrees if multi-edges were removed.
 *
 * Note: This implementation removes self-loops and multi-edges, which may
 * result in a graph that doesn't exactly match the degree sequence.
 * For sparse graphs with finite mean degree, this effect vanishes as n -> infinity.
 */
Graph *config_model_generate(int n, const int *degrees, int max_attempts);

/*
 * Generate degree sequence from Poisson distribution (Erdos-Renyi)
 * Returns array of length n (caller must free)
 */
int *degree_seq_poisson(int n, double c);

/*
 * Generate degree sequence for random regular graph
 * Returns array of length n (caller must free), or NULL if n*c is odd
 */
int *degree_seq_regular(int n, int c);

/*
 * Generate degree sequence from power-law distribution p(k) ~ k^{-gamma}
 * Returns array of length n (caller must free)
 */
int *degree_seq_powerlaw(int n, int kmin, double gamma, int kmax);

#endif /* CONFIG_MODEL_H */
