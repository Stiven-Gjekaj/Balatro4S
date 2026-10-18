// Tests of the vouchers. The expected values come from Card:redeem of
// Balatro 1.0.1o, with tools/oracle.sh score and a "redeem" line.
// tools/fuzz.py --voucher compares many more random purchases.

#import "BOTest.h"
#include <stdlib.h>
#include "bo_pool.h"
#include "bo_run.h"
#include "bo_voucher.h"

// A new shop of a run, with the voucher center for sale.
static BORun *make_shop(const char *seed, int dollars, BOCenterId voucher) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, seed);
    run->dollars = dollars;
    run->state = BO_STATE_SHOP;
    for (int i = 0; i < 2; i++) run->shop[run->shop_count++] = bo_pool_create_shop_card(run);
    run->shop_voucher = bo_pool_make_item(run, voucher);
    run->has_shop_voucher = 1;
    return run;
}

BO_TEST(test_voucher_overstock_adds_a_card_to_the_shop) {
    BORun *run = make_shop("ABCDEFGH", 30, BO_V_OVERSTOCK_NORM);
    BO_EXPECT(bo_voucher_redeem(run));
    BO_EQUAL_INT(run->dollars, 20);
    BO_EQUAL_INT(run->shop_joker_max, 3);
    BO_EQUAL_INT(run->shop_count, 3);
    BO_EQUAL_INT(run->shop[2].item.center, BO_J_CRAZY);
    BO_EXPECT(run->used_vouchers[BO_V_OVERSTOCK_NORM]);
    BO_EXPECT(!run->has_shop_voucher);
    free(run);
}

BO_TEST(test_voucher_petroglyph_lowers_the_ante_and_keeps_discards_at_zero) {
    BORun *run = make_shop("ABCDEFGH", 30, BO_V_PETROGLYPH);
    run->discards_left = 0;
    BO_EXPECT(bo_voucher_redeem(run));
    BO_EQUAL_INT(run->ante, 0);
    BO_EQUAL_INT(run->discards_per_round, 3);
    BO_EQUAL_INT(run->discards_left, 0);
    free(run);
}

BO_TEST(test_voucher_needs_the_money) {
    BORun *run = make_shop("ABCDEFGH", 9, BO_V_GRABBER);
    BO_EXPECT(!bo_voucher_can_redeem(run));
    BO_EXPECT(!bo_voucher_redeem(run));
    BO_EQUAL_INT(run->hands_per_round, 4);
    free(run);
}

BO_TEST(test_voucher_directors_cut_rerolls_the_boss_once_an_ante) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    run->dollars = 30;
    BO_EXPECT(!bo_voucher_can_reroll_boss(run));
    run->used_vouchers[BO_V_DIRECTORS_CUT] = 1;
    int used = 0;
    for (int i = 0; i < BO_BLIND_COUNT; i++) used += run->bosses_used[i];
    BO_EXPECT(bo_voucher_reroll_boss(run));
    // get_new_boss counts the new boss as used.
    int after = 0;
    for (int i = 0; i < BO_BLIND_COUNT; i++) after += run->bosses_used[i];
    BO_EQUAL_INT(after, used + 1);
    BO_EQUAL_INT(run->dollars, 20);
    BO_EXPECT(!bo_voucher_can_reroll_boss(run));
    // Retcon gives a reroll each time.
    run->used_vouchers[BO_V_RETCON] = 1;
    BO_EXPECT(bo_voucher_reroll_boss(run));
    BO_EQUAL_INT(run->dollars, 10);
    free(run);
}

BO_TEST(test_voucher_of_a_tag_can_be_redeemed_too) {
    BORun *run = make_shop("ABCDEFGH", 30, BO_V_GRABBER);
    run->tag_vouchers[run->tag_voucher_count++] = bo_pool_make_item(run, BO_V_WASTEFUL);
    BO_EXPECT(bo_voucher_redeem_tag(run, 0));
    BO_EQUAL_INT(run->tag_voucher_count, 0);
    BO_EXPECT(run->used_vouchers[BO_V_WASTEFUL]);
    BO_EQUAL_INT(run->discards_per_round, 5);
    // The redeem clears current_round.voucher, but the shop keeps its card.
    BO_EQUAL_INT(run->voucher, BO_CENTER_NONE);
    BO_EXPECT(run->has_shop_voucher);
    free(run);
}
