/*
 * triadic_closure.h - Static Triadic Closure procedure
 *
 * Given a backbone graph G0, produces a new graph Gf by closing
 * each triad with probability f.
 */

#ifndef TRIADIC_CLOSURE_H
#define TRIADIC_CLOSURE_H

#include "graph.h"

/*
 * Perform static triadic closure on backbone graph.
 *
 * For each triad (i, j, k) where edges (i,j) and (j,k) exist in backbone,
 * add edge (i,k) with probability f.
 *
 * Parameters:
 *   backbone - the original graph G0 (must be finalized)
 *   f        - triadic closure probability
 *
 * Returns:
 *   A new Graph Gf containing all edges from backbone plus new edges.
 *   Returns NULL on failure.
 *
 * Note: The backbone graph is not modified.
 */
Graph *triadic_closure(const Graph *backbone, double f);

/*
 * Perform triadic closure in-place, reusing an existing graph structure.
 * The output graph is cleared and refilled.
 *
 * Parameters:
 *   backbone - the original graph G0 (must be finalized)
 *   f        - triadic closure probability  
 *   output   - graph to store result (will be cleared first)
 *
 * Returns:
 *   0 on success, -1 on failure
 */
int triadic_closure_inplace(const Graph *backbone, double f, Graph *output);

#endif /* TRIADIC_CLOSURE_H */
