// Tests of the shop. The expected cards come from create_card_for_shop of
// Balatro 1.0.1o for the same seed, with tools/oracle.sh shop.

#import "BOTest.h"
#include <stdlib.h>
#include "bo_pool.h"
#include "bo_run.h"

static NSString *shop_keys(BORun *run) {
    NSMutableArray *keys = [NSMutableArray array];
    for (int i = 0; i < run->shop_count; i++) {
        BOItem *item = &run->shop[i].item;
        NSString *key = @(bo_center(item->center)->key);
        if (item->edition == BO_EDITION_FOIL) key = [key stringByAppendingString:@":foil"];
        if (item->edition == BO_EDITION_HOLO) key = [key stringByAppendingString:@":holo"];
        if (item->edition == BO_EDITION_POLYCHROME) key = [key stringByAppendingString:@":polychrome"];
        if (item->edition == BO_EDITION_NEGATIVE) key = [key stringByAppendingString:@":negative"];
        [keys addObject:key];
    }
    return [keys componentsJoinedByString:@" "];
}

BO_TEST(test_shop_gives_the_cards_of_balatro_for_a_seed) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    // The rerolls of the first shop, with no money limit.
    run->state = BO_STATE_SHOP;
    run->dollars = 1000;
    run->shop_count = 0;
    for (int i = 0; i < 2; i++) run->shop[run->shop_count++] = bo_pool_create_shop_card(run);
    NSArray *expected = @[@"j_rocket c_lovers", @"j_crazy j_sly", @"j_business j_hallucination",
                          @"j_half j_matador", @"c_neptune j_runner", @"j_green_joker c_justice",
                          @"j_astronomer j_supernova"];
    BO_EQUAL(shop_keys(run), expected[0]);
    for (int r = 1; r < (int)expected.count; r++) {
        bo_run_reroll(run);
        BO_EQUAL(shop_keys(run), expected[r]);
    }
    BO_EQUAL(@(bo_center(run->voucher)->key), @"v_paint_brush");
    // The first pack is a Buffoon Pack. The original picks the art of it
    // with math.random, so only the kind is the same.
    BO_EQUAL(@(bo_center(bo_pool_get_pack(run, "shop_pack", NULL))->kind), @"Buffoon");
    BO_EQUAL(@(bo_center(bo_pool_get_pack(run, "shop_pack", NULL))->key), @"p_arcana_jumbo_1");
    free(run);
}

BO_TEST(test_shop_gives_a_playing_card_one_sort_id) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    run->state = BO_STATE_SHOP;
    // Only playing cards in the shop, as with Magic Trick and no other rate.
    run->joker_rate = run->tarot_rate = run->planet_rate = run->spectral_rate = 0;
    run->playing_card_rate = 4;
    uint32_t before = run->next_sort_id;
    BOShopCard card = bo_pool_create_shop_card(run);
    BO_EXPECT(card.is_playing_card);
    BO_EQUAL_INT(run->next_sort_id, before + 1);
    BO_EQUAL_INT(card.card.uid, card.item.sort_id);
    free(run);
}
