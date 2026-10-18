// bo_random: the random numbers of Balatro 1.0.1o, so that a seed gives the
// same run as in the original.
//
// Balatro runs on LuaJIT 2.1. Its math.random is the Tausworthe generator
// TW223 of LuaJIT, and math.randomseed sets it from a double. Balatro makes
// that double with pseudoseed, from a hash of a key and the seed of the run.

#ifndef BO_RANDOM_H
#define BO_RANDOM_H

#include <stdint.h>

// The state of math.random in LuaJIT.
typedef struct {
    uint64_t gen[4];
} BOLuaRandom;

void bo_lua_randomseed(BOLuaRandom *state, double seed);
// math.random(): a double in [0, 1).
double bo_lua_random(BOLuaRandom *state);
// math.random(lo, hi): an integer in [lo, hi]. math.random(n) is (1, n).
int bo_lua_random_int(BOLuaRandom *state, int lo, int hi);

#define BO_RANDOM_KEYS 192
#define BO_RANDOM_KEY_LENGTH 40

typedef struct {
    char seed[16];
    double hashed_seed;
    int count;
    struct {
        char key[BO_RANDOM_KEY_LENGTH];
        double value;
    } entries[BO_RANDOM_KEYS];
    BOLuaRandom lua;  // The one global math.random state of Lua.
} BORandom;

// pseudohash of Balatro: a number in [0, 1) from a string.
double bo_pseudohash(const char *text);

// Starts the random state of a run with seed, for example "ABCDEFGH".
void bo_random_start(BORandom *random, const char *seed);

// pseudoseed(key): moves the state of key one step and gives a number to
// seed math.random with.
double bo_pseudoseed(BORandom *random, const char *key);

// pseudorandom(key): a number in [0, 1).
double bo_pseudorandom(BORandom *random, const char *key);

// pseudorandom(key, min, max): an integer in [min, max].
int bo_pseudorandom_int(BORandom *random, const char *key, int min, int max);

// The part of pseudoshuffle and pseudorandom_element after the sort:
// math.randomseed(seed), then math.random(count). Returns an index from 0.
int bo_random_index(BORandom *random, double seed, int count);

// pseudoshuffle of a list that is already in the order of its sort_id.
// order gets the new order as indices from 0.
void bo_pseudoshuffle(BORandom *random, double seed, int *order, int count);

// random_string(8, seed): a seed for a new run, for example "7LB2WVPK".
// It has the digits 1 to 9 and the letters A to Z without O.
void bo_random_string(double seed, char out[9]);

#endif
