// Tests of bo_random. The expected numbers come from the functions of
// Balatro 1.0.1o in LuaJIT 2.1, with tools/oracle.lua.

#import "BOTest.h"
#include <math.h>
#include "bo_random.h"

#define BO_EQUAL_DOUBLE(actual, expected)                                                       \
    do {                                                                                      \
        double bo_a = (actual), bo_e = (expected);                                            \
        if (bo_a != bo_e)                                                                     \
            BOTestFail(__FILE__, __LINE__,                                                    \
                       [NSString stringWithFormat:@"%s is %.17g, expected %.17g", #actual, bo_a, bo_e]); \
    } while (0)

BO_TEST(test_random_is_the_math_random_of_luajit) {
    BOLuaRandom state;
    bo_lua_randomseed(&state, 0.5);
    BO_EQUAL_DOUBLE(bo_lua_random(&state), 0.09657393438653461);
    BO_EQUAL_DOUBLE(bo_lua_random(&state), 0.96226945770684003);
    BO_EQUAL_DOUBLE(bo_lua_random(&state), 0.52150907845154126);
}

BO_TEST(test_random_hashes_a_string_as_balatro_does) {
    BO_EQUAL_DOUBLE(bo_pseudohash("ABCDEFGH"), 0.074964212416716691);
}

BO_TEST(test_random_steps_the_seed_of_a_key) {
    BORandom random;
    bo_random_start(&random, "ABCDEFGH");
    BO_EQUAL_DOUBLE(bo_pseudoseed(&random, "shuffle"), 0.090643430007258344);
    BO_EQUAL_DOUBLE(bo_pseudoseed(&random, "shuffle"), 0.19637554741605834);
    BO_EQUAL_DOUBLE(bo_pseudoseed(&random, "shuffle"), 0.37869074219835835);
}

BO_TEST(test_random_shuffles_as_pseudoshuffle) {
    BORandom random;
    bo_random_start(&random, "ABCDEFGH");
    int order[10];
    bo_pseudoshuffle(&random, 0.123456, order, 10);
    // Balatro gives 10 9 7 8 1 4 3 5 2 6 for the values 1 to 10.
    int expected[10] = {9, 8, 6, 7, 0, 3, 2, 4, 1, 5};
    for (int i = 0; i < 10; i++) BO_EQUAL_INT(order[i], expected[i]);
}
