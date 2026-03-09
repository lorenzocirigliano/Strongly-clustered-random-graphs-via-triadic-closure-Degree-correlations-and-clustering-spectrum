/*
 * rng.h - Fast random number generation using xoshiro256**
 */

#ifndef RNG_H
#define RNG_H

#include <stdint.h>

/* Initialize the RNG with a seed */
void rng_seed(uint64_t seed);

/* Generate a uniform random double in [0, 1) */
double rng_uniform(void);

/* Generate a uniform random integer in [0, n-1] */
int rng_int(int n);

/* Sample from Poisson distribution with mean lambda */
int rng_poisson(double lambda);

/* Sample from Binomial(n, p) distribution */
int rng_binomial(int n, double p);

#endif /* RNG_H */
