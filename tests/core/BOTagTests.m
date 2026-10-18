// Tests of the tags. The expected values come from Tag:apply_to_run of
// Balatro 1.0.1o with the same seed, with tools/oracle.sh score and "tag"
// lines. tools/fuzz.py --tag compares many more random tags.

#import "BOTest.h"
#include <stdlib.h>
#include <string.h>
#include "bo_run.h"
#include "bo_tag.h"

// A run at the blind select, as the oracle makes it.
static BORun *make_run(int dollars) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    bo_random_start(&run->random, "ABCDEFGH");
    run->deck_count = 0;
    run->dollars = dollars;
    run->state = BO_STATE_BLIND_SELECT;
    return run;
}

static void skip_for(BORun *run, BOCenterId tag) {
    run->skips++;
    bo_tag_add(run, tag, BO_TAG_FROM_SMALL);
    bo_tag_immediate(run);
}

static void open_shop(BORun *run) {
    run->state = BO_STATE_SHOP;
    run->voucher = BO_CENTER_NONE;
    bo_run_fill_shop(run);
}

BO_TEST(test_tag_double_copies_economy_and_uncommon_gives_a_free_joker) {
    BORun *run = make_run(10);
    skip_for(run, BO_TAG_DOUBLE);
    skip_for(run, BO_TAG_ECONOMY);
    BO_EQUAL_INT(run->dollars, 40);
    skip_for(run, BO_TAG_UNCOMMON);
    skip_for(run, BO_TAG_VOUCHER);
    open_shop(run);
    BO_EQUAL_INT(run->shop[0].item.center, BO_J_RAMEN);
    BO_EQUAL_INT(run->shop[0].item.cost, 0);
    BO_EQUAL_INT(run->shop[1].item.center, BO_J_ROCKET);
    BO_EQUAL_INT(run->shop[1].item.cost, 6);
    BO_EQUAL_INT(run->tag_voucher_count, 1);
    BO_EQUAL_INT(run->tag_vouchers[0].center, BO_V_BLANK);
    BO_EQUAL_INT(run->tag_count, 0);
    free(run);
}

BO_TEST(test_tag_coupon_foil_and_d6_change_the_shop) {
    BORun *run = make_run(10);
    skip_for(run, BO_TAG_TOP_UP);
    BO_EQUAL_INT(run->joker_count, 2);
    BO_EQUAL_INT(run->jokers[0].center, BO_J_HALLUCINATION);
    BO_EQUAL_INT(run->jokers[1].center, BO_J_SCARY_FACE);
    skip_for(run, BO_TAG_FOIL);
    skip_for(run, BO_TAG_COUPON);
    skip_for(run, BO_TAG_D_SIX);
    BO_EQUAL_INT(run->skips, 4);
    open_shop(run);
    BO_EQUAL_INT(run->shop[0].item.center, BO_J_ROCKET);
    BO_EQUAL_INT(run->shop[0].item.edition, BO_EDITION_FOIL);
    BO_EQUAL_INT(run->shop[0].item.cost, 0);
    BO_EQUAL_INT(run->shop[1].item.cost, 0);
    BO_EQUAL_INT(run->shop_packs[0].cost, 0);
    BO_EQUAL_INT(run->reroll_cost, 0);
    free(run);
}

BO_TEST(test_tag_skip_moves_to_the_next_blind) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    BO_EXPECT(bo_run_can_skip_blind(run));
    BO_EXPECT(bo_run_skip_blind(run));
    BO_EQUAL_INT(run->slot, BO_SLOT_BIG);
    BO_EQUAL_INT(run->skips, 1);
    BO_EQUAL_INT(run->skipped_slots, 1 << BO_SLOT_SMALL);
    BO_EXPECT(bo_run_skip_blind(run));
    BO_EQUAL_INT(run->slot, BO_SLOT_BOSS);
    BO_EXPECT(!bo_run_can_skip_blind(run));
    free(run);
}

BO_TEST(test_tag_each_investment_tag_gives_its_own_row) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    bo_tag_add(run, BO_TAG_INVESTMENT, BO_TAG_FROM_NONE);
    bo_tag_add(run, BO_TAG_INVESTMENT, BO_TAG_FROM_NONE);
    run->slot = BO_SLOT_BOSS;
    bo_run_select_blind(run);
    run->chips = run->blind_chips;
    bo_run_end_round(run);
    // evaluate_round adds a row for each tag, with the place of the tag.
    int rows = 0;
    for (int i = 0; i < run->eval.row_count; i++) {
        const BOEvalRow *row = &run->eval.rows[i];
        if (row->kind != BO_EVAL_TAG) continue;
        BO_EQUAL_INT(row->index, rows);
        BO_EQUAL_INT(row->dollars, 25);
        rows++;
    }
    BO_EQUAL_INT(rows, 2);
    BO_EQUAL_INT(run->tag_count, 0);
    free(run);
}

BO_TEST(test_tag_juggle_gives_hand_size_for_one_round) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    bo_tag_add(run, BO_TAG_JUGGLE, BO_TAG_FROM_NONE);
    bo_run_select_blind(run);
    BO_EQUAL_INT(run->hand_size, 11);
    BO_EQUAL_INT(run->hand_count, 11);
    BO_EQUAL_INT(run->tag_count, 0);
    // The round ends: the hand size goes back.
    run->blind_chips = 1;
    uint64_t one = 1;
    BOScore score;
    bo_run_play(run, one, &score);
    BO_EQUAL_INT(run->state, BO_STATE_ROUND_EVAL);
    BO_EQUAL_INT(run->hand_size, 8);
    free(run);
}
