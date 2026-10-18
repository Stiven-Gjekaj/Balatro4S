#include "bo_random.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LuaJIT does each multiply and each add on its own. A fused multiply-add
// rounds one time less and gives other numbers, so turn it off here.
#pragma STDC FP_CONTRACT OFF

typedef union {
    uint64_t u64;
    double d;
} BOBits;

// One step of generator i of TW223, as the macro TW223_GEN of lib_math.c.
#define BO_TW223_GEN(i, k, q, s)                                                         \
    z = state->gen[i];                                                                 \
    z = (((z << q) ^ z) >> (k - s)) ^ ((z & ((uint64_t)(int64_t)-1 << (64 - k))) << s); \
    r ^= z;                                                                            \
    state->gen[i] = z;

// Returns the bits of a double in [1, 2).
static uint64_t random_step(BOLuaRandom *state) {
    uint64_t z, r = 0;
    BO_TW223_GEN(0, 63, 31, 18)
    BO_TW223_GEN(1, 58, 19, 28)
    BO_TW223_GEN(2, 55, 24, 7)
    BO_TW223_GEN(3, 47, 21, 8)
    return (r & 0x000fffffffffffffULL) | 0x3ff0000000000000ULL;
}

void bo_lua_randomseed(BOLuaRandom *state, double d) {
    uint32_t r = 0x11090601;  // 64 - k of each generator, as four bytes.
    for (int i = 0; i < 4; i++) {
        BOBits bits;
        uint32_t m = 1u << (r & 255);
        r >>= 8;
        bits.d = d = d * 3.14159265358979323846 + 2.7182818284590452354;
        if (bits.u64 < m) bits.u64 += m;
        state->gen[i] = bits.u64;
    }
    for (int i = 0; i < 10; i++) random_step(state);
}

double bo_lua_random(BOLuaRandom *state) {
    BOBits bits;
    bits.u64 = random_step(state);
    return bits.d - 1.0;
}

int bo_lua_random_int(BOLuaRandom *state, int lo, int hi) {
    double d = bo_lua_random(state);
    return (int)(floor(d * ((double)hi - lo + 1.0)) + lo);
}

// The % of Lua for numbers: a - floor(a / b) * b.
static double lua_mod(double a, double b) {
    return a - floor(a / b) * b;
}

double bo_pseudohash(const char *text) {
    double num = 1;
    for (size_t i = strlen(text); i >= 1; i--) {
        num = lua_mod((1.1239285023 / num) * (unsigned char)text[i - 1] * M_PI + M_PI * (double)i, 1);
    }
    return num;
}

void bo_random_start(BORandom *random, const char *seed) {
    memset(random, 0, sizeof(*random));
    snprintf(random->seed, sizeof(random->seed), "%s", seed);
    random->hashed_seed = bo_pseudohash(random->seed);
}

// tonumber(string.format("%.13f", value)).
static double round13(double value) {
    char text[64];
    snprintf(text, sizeof(text), "%.13f", value);
    return strtod(text, NULL);
}

double bo_pseudoseed(BORandom *random, const char *key) {
    int index = -1;
    for (int i = 0; i < random->count; i++) {
        if (strcmp(random->entries[i].key, key) == 0) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        // A new key starts at the hash of the key and the seed.
        char joined[BO_RANDOM_KEY_LENGTH + 16];
        snprintf(joined, sizeof(joined), "%s%s", key, random->seed);
        if (random->count < BO_RANDOM_KEYS) {
            index = random->count++;
        } else {
            index = BO_RANDOM_KEYS - 1;
        }
        snprintf(random->entries[index].key, BO_RANDOM_KEY_LENGTH, "%s", key);
        random->entries[index].value = bo_pseudohash(joined);
    }
    double value = fabs(round13(lua_mod(2.134453429141 + random->entries[index].value * 1.72431234, 1)));
    random->entries[index].value = value;
    return (value + random->hashed_seed) / 2;
}

double bo_pseudorandom(BORandom *random, const char *key) {
    bo_lua_randomseed(&random->lua, bo_pseudoseed(random, key));
    return bo_lua_random(&random->lua);
}

int bo_pseudorandom_int(BORandom *random, const char *key, int min, int max) {
    bo_lua_randomseed(&random->lua, bo_pseudoseed(random, key));
    return bo_lua_random_int(&random->lua, min, max);
}

int bo_random_index(BORandom *random, double seed, int count) {
    bo_lua_randomseed(&random->lua, seed);
    return bo_lua_random_int(&random->lua, 1, count) - 1;
}

void bo_pseudoshuffle(BORandom *random, double seed, int *order, int count) {
    bo_lua_randomseed(&random->lua, seed);
    for (int i = 0; i < count; i++) order[i] = i;
    for (int i = count; i >= 2; i--) {
        int j = bo_lua_random_int(&random->lua, 1, i);
        int t = order[i - 1];
        order[i - 1] = order[j - 1];
        order[j - 1] = t;
    }
}

void bo_random_string(double seed, char out[9]) {
    BOLuaRandom state;
    bo_lua_randomseed(&state, seed);
    for (int i = 0; i < 8; i++) {
        // The order of the calls is the order of the "and" and "or" of Lua.
        if (bo_lua_random(&state) > 0.7) {
            out[i] = (char)bo_lua_random_int(&state, '1', '9');
        } else if (bo_lua_random(&state) > 0.45) {
            out[i] = (char)bo_lua_random_int(&state, 'A', 'N');
        } else {
            out[i] = (char)bo_lua_random_int(&state, 'P', 'Z');
        }
    }
    out[8] = 0;
}
