/*
 * rng.c - Fast random number generation using xoshiro256**
 */

#define _GNU_SOURCE
#include "rng.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Internal state for xoshiro256** */
static uint64_t rng_state[4];

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t rng_next(void) {
    const uint64_t result = rotl(rng_state[1] * 5, 7) * 9;
    const uint64_t t = rng_state[1] << 17;
    
    rng_state[2] ^= rng_state[0];
    rng_state[3] ^= rng_state[1];
    rng_state[1] ^= rng_state[2];
    rng_state[0] ^= rng_state[3];
    rng_state[2] ^= t;
    rng_state[3] = rotl(rng_state[3], 45);
    
    return result;
}

void rng_seed(uint64_t seed) {
    /* Initialize with splitmix64 */
    for (int i = 0; i < 4; i++) {
        uint64_t z = (seed += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        rng_state[i] = z ^ (z >> 31);
    }
}

double rng_uniform(void) {
    return (rng_next() >> 11) * 0x1.0p-53;
}

int rng_int(int n) {
    if (n <= 0) return 0;
    return (int)(rng_uniform() * n);
}

int rng_poisson(double lambda) {
    if (lambda <= 0) return 0;
    
    if (lambda < 30) {
        /* Direct method for small lambda */
        double L = exp(-lambda);
        int k = 0;
        double p = 1.0;
        
        do {
            k++;
            p *= rng_uniform();
        } while (p > L);
        
        return k - 1;
    } else {
        /* Normal approximation for large lambda */
        double u1 = rng_uniform();
        double u2 = rng_uniform();
        double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        int result = (int)round(lambda + sqrt(lambda) * z);
        return (result < 0) ? 0 : result;
    }
}

int rng_binomial(int n, double p) {
    if (n <= 0 || p <= 0.0) return 0;
    if (p >= 1.0) return n;
    
    /* For small n, use direct method */
    if (n < 25) {
        int count = 0;
        for (int i = 0; i < n; i++) {
            if (rng_uniform() < p) count++;
        }
        return count;
    }
    
    /* For larger n, use normal approximation */
    double mean = n * p;
    double std = sqrt(n * p * (1.0 - p));
    
    double u1 = rng_uniform();
    double u2 = rng_uniform();
    double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    
    int result = (int)round(mean + std * z);
    if (result < 0) result = 0;
    if (result > n) result = n;
    
    return result;
}
