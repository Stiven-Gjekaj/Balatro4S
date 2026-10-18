// Tests of bo_blind. The bosses come from get_new_boss of Balatro 1.0.1o
// in LuaJIT, with tools/oracle.lua.

#import "BOTest.h"
#include "bo_blind.h"

static void expect_bosses(const char *seed, const BOBlindId *expected, int antes) {
    BORandom random;
    bo_random_start(&random, seed);
    int used[BO_BLIND_COUNT] = {0};
    for (int ante = 1; ante <= antes; ante++) {
        BOBlindId boss = bo_blind_new_boss(&random, ante, 8, used, NULL);
        if (boss != expected[ante - 1]) {
            BOTestFail(__FILE__, __LINE__, [NSString stringWithFormat:@"%s ante %d: %s, expected %s", seed, ante,
                                                                       bo_blind_info(boss)->key,
                                                                       bo_blind_info(expected[ante - 1])->key]);
        }
    }
}

BO_TEST(test_blind_picks_the_bosses_of_balatro_for_a_seed) {
    BOBlindId first[] = {BO_BL_GOAD, BO_BL_ARM, BO_BL_HOOK, BO_BL_MARK, BO_BL_WHEEL,
                         BO_BL_FLINT, BO_BL_WALL, BO_BL_FINAL_VESSEL, BO_BL_PLANT, BO_BL_FISH};
    expect_bosses("ABCDEFGH", first, 10);
    BOBlindId second[] = {BO_BL_CLUB, BO_BL_WHEEL, BO_BL_HOOK, BO_BL_NEEDLE, BO_BL_MARK, BO_BL_WALL,
                          BO_BL_PLANT, BO_BL_FINAL_ACORN, BO_BL_FISH, BO_BL_HOUSE, BO_BL_OX, BO_BL_WATER,
                          BO_BL_GOAD, BO_BL_FLINT, BO_BL_HEAD, BO_BL_FINAL_VESSEL};
    expect_bosses("7LB2WVPK", second, 16);
}

BO_TEST(test_blind_gives_the_chip_amount_of_each_ante) {
    BO_EQUAL_INT(bo_blind_amount(1, 1), 300);
    BO_EQUAL_INT(bo_blind_amount(8, 1), 50000);
    BO_EQUAL_INT(bo_blind_amount(4, 2), 8000);
    BO_EQUAL_INT(bo_blind_amount(6, 3), 60000);
    BO_EQUAL_INT(bo_blind_amount(0, 1), 100);
    // The endless antes, from get_blind_amount in LuaJIT.
    BO_EQUAL_INT(bo_blind_amount(9, 1), 110000);
    BO_EQUAL_INT(bo_blind_amount(12, 1), 300000000);
}

BO_TEST(test_blind_has_the_values_of_the_table) {
    BO_EQUAL_INT(bo_blind_info(BO_BL_SMALL)->dollars, 3);
    BO_EQUAL_INT(bo_blind_info(BO_BL_BIG)->mult * 2, 3);
    BO_EQUAL_INT(bo_blind_info(BO_BL_WALL)->mult, 4);
    BO_EQUAL_INT(bo_blind_info(BO_BL_FINAL_VESSEL)->mult, 6);
    BO_EQUAL_INT(bo_blind_info(BO_BL_NEEDLE)->mult, 1);
    BO_EXPECT(bo_blind_info(BO_BL_FINAL_BELL)->showdown);
    BO_EXPECT(!bo_blind_info(BO_BL_BIG)->boss);
}
