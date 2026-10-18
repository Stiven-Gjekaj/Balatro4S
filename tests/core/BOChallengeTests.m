// Tests of the challenges. The expected values come from challenges.lua
// and Game:start_run of Balatro 1.0.1o.

#import "BOTest.h"
#include <stdlib.h>
#include <string.h>
#include "bo_challenge_data.h"
#include "bo_pool.h"
#include "bo_run.h"

BO_TEST(test_challenge_data_has_the_20_challenges) {
    BO_EQUAL_INT(BO_CHALLENGE_COUNT, 20);
    const BOChallengeInfo *omelette = bo_challenge(0);
    BO_EQUAL(@(omelette->id), @"c_omelette_1");
    BO_EQUAL_INT(omelette->joker_count, 5);
    BO_EQUAL(@(omelette->jokers[0].key), @"j_egg");
    BO_EQUAL_INT(omelette->custom_count, 3);
    BO_EQUAL(@(omelette->custom[0].id), @"no_reward");
    // Fragile bans the Standard packs with the ids of the group.
    const BOChallengeInfo *fragile = bo_challenge(13);
    BO_EQUAL(@(fragile->name), @"Fragile");
    BO_EQUAL_INT(fragile->banned_count, 26);
    BO_EQUAL_INT(fragile->deck_card_count, 52);
    BO_EXPECT(fragile->jokers[0].eternal);
    BO_EQUAL(@(fragile->jokers[0].edition), @"negative");
    const BOChallengeInfo *luxury = bo_challenge(6);
    BO_EQUAL(@(luxury->custom[0].id), @"minus_hand_size_per_X_dollar");
    BO_EXPECT(luxury->custom[0].number == 5);
    BO_EXPECT(bo_challenge(20) == NULL);
}

static BORun *start(int index) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start_challenge(run, "ABCDEFGH", index);
    return run;
}

BO_TEST(test_challenge_omelette_starts_with_five_eggs_and_no_money) {
    BORun *run = start(0);
    BO_EQUAL_INT(run->challenge, 1);
    BO_EQUAL_INT(run->back, BO_B_CHALLENGE);
    BO_EQUAL_INT(run->joker_count, 5);
    for (int i = 0; i < 5; i++) BO_EQUAL_INT(run->jokers[i].center, BO_J_EGG);
    BO_EQUAL_INT(run->no_blind_reward, 7);
    BO_EXPECT(run->no_interest && run->no_extra_hand_money);
    BO_EXPECT(run->banned[BO_J_ROCKET] && run->banned[BO_V_SEED_MONEY]);
    bo_run_select_blind(run);
    BO_EQUAL_INT(run->blind_dollars, 0);
    free(run);
}

BO_TEST(test_challenge_luxury_tax_takes_a_card_for_each_five_dollars) {
    BORun *run = start(6);
    BO_EQUAL_INT(run->hand_size, 10);
    bo_run_ease_dollars(run, 11);
    // $15: three times $5, so three cards less.
    BO_EQUAL_INT(run->hand_size, 7);
    bo_run_ease_dollars(run, -10);
    BO_EQUAL_INT(run->hand_size, 9);
    free(run);
}

BO_TEST(test_challenge_typecast_makes_the_jokers_eternal_at_ante_4) {
    BORun *run = start(10);
    BOItem joker = bo_pool_make_item(run, BO_J_JOKER);
    bo_run_add_joker(run, &joker);
    run->ante = 4;
    run->slot = BO_SLOT_BOSS;
    bo_run_select_blind(run);
    run->chips = run->blind_chips;
    bo_run_end_round(run);
    BO_EXPECT(run->jokers[0].eternal);
    BO_EQUAL_INT(run->joker_slots, 0);
    BO_EQUAL_INT(run->ante, 5);
    free(run);
}

BO_TEST(test_challenge_inflation_raises_the_prices_after_a_purchase) {
    BORun *run = start(11);
    run->state = BO_STATE_SHOP;
    run->dollars = 100;
    bo_run_fill_shop(run);
    int before = run->shop_packs[0].cost;
    BO_EXPECT(bo_run_buy(run, 0));
    BO_EQUAL_INT(run->inflation, 1);
    BO_EQUAL_INT(run->shop_packs[0].cost, before + 1);
    free(run);
}

BO_TEST(test_challenge_golden_needle_charges_each_discard) {
    BORun *run = start(17);
    BO_EQUAL_INT(run->hands_per_round, 1);
    BO_EQUAL_INT(run->discards_per_round, 6);
    BO_EQUAL_INT(run->dollars, 10);
    bo_run_select_blind(run);
    BO_EXPECT(bo_run_discard(run, 1));
    BO_EQUAL_INT(run->dollars, 9);
    free(run);
}

BO_TEST(test_challenge_jokerless_has_no_joker_and_no_banned_card_in_the_shop) {
    BORun *run = start(19);
    BO_EQUAL_INT(run->joker_slots, 0);
    for (int i = 0; i < 300; i++) {
        BOShopCard card = bo_pool_create_shop_card(run);
        if (card.is_playing_card) continue;
        BO_EXPECT(bo_center(card.item.center)->set != BO_SET_JOKER);
        BO_EXPECT(!run->banned[card.item.center]);
    }
    for (int i = 0; i < 100; i++) {
        BOCenterId pack = bo_pool_get_pack(run, "shop_pack", NULL);
        BO_EXPECT(!run->banned[pack]);
    }
    free(run);
}
