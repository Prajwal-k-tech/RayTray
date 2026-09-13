#ifndef RANDOM_H
#define RANDOM_H

#include <cstdint>

// Phase 3: Fast PCG32 random number generator (Thread-local state)
// Standard std::mt19937 and rand() cause severe contention under OpenMP.
inline thread_local uint64_t pcg_state = 0x853c49e6748fea9bULL;
inline thread_local uint64_t pcg_inc = 0xda3e39cb94b95bdbULL;

inline void seed_random(uint64_t initstate, uint64_t initseq) {
    pcg_state = 0U;
    pcg_inc = (initseq << 1u) | 1u;
    uint64_t oldstate = pcg_state;
    pcg_state = oldstate * 6364136223846793005ULL + pcg_inc;
    pcg_state += initstate;
    oldstate = pcg_state;
    pcg_state = oldstate * 6364136223846793005ULL + pcg_inc;
}

inline uint32_t random_u32() {
    uint64_t oldstate = pcg_state;
    pcg_state = oldstate * 6364136223846793005ULL + pcg_inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

inline double random_double() {
    // Returns a random real in [0,1).
    return (random_u32() >> 8) * (1.0 / 16777216.0);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

#endif
