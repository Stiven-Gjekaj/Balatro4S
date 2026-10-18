// Tests of bo_run. The orders of the deck come from the shuffle of Balatro
// 1.0.1o in the LuaJIT of Balatro.app, with tools/oracle.sh.

#import "BOTest.h"
#include <string.h>
#include "bo_run.h"

static NSString *names(const BOCard *cards, int count, int fromTop) {
    NSMutableArray *list = [NSMutableArray array];
    for (int i = 0; i < count; i++) {
        char name[4];
        const BOCard *card = fromTop ? &cards[count - 1 - i] : &cards[i];
        // The oracle writes the rank first, then the suit, as "TS".
        bo_card_name(card, name);
        [list addObject:@(name)];
    }
    return [list componentsJoinedByString:@" "];
}

BO_TEST(test_run_shuffles_the_deck_as_balatro_does) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    BO_EQUAL([names(run->deck, run->deck_count, 1) substringToIndex:23], @"6C QC 8D KC 4D 9S TD 7D");
    BO_EQUAL_INT(run->boss, BO_BL_GOAD);

    // The blind shuffles the deck with the key "nr1", and draws 8 cards
    // from the top. The hand sorts them from high to low.
    bo_run_select_blind(run);
    BO_EQUAL_INT(run->hand_count, 8);
    BO_EQUAL_INT(run->deck_count, 44);
    BO_EQUAL(names(run->hand, run->hand_count, 0), @"KC 8H 8D 7S 7H 3H 2S 2H");
    free(run);
}

BO_TEST(test_run_beats_a_blind_and_cashes_out) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    bo_run_select_blind(run);
    BO_EQUAL_INT(run->blind_chips, 300);
    // Make the target small, so that one hand beats it.
    run->blind_chips = 10;
    BOScore score;
    BO_EXPECT(bo_run_play(run, 0x1, &score));
    BO_EQUAL_INT(run->state, BO_STATE_ROUND_EVAL);
    // $3 for the Small Blind, $3 for the 3 hands that are left, and no
    // interest on $4.
    BO_EQUAL_INT(run->eval.row_count, 2);
    BO_EQUAL_INT(run->eval.rows[0].kind, BO_EVAL_BLIND);
    BO_EQUAL_INT(run->eval.rows[0].dollars, 3);
    BO_EQUAL_INT(run->eval.rows[1].kind, BO_EVAL_HANDS);
    BO_EQUAL_INT(run->eval.rows[1].dollars, 3);
    BO_EQUAL_INT(run->eval.total, 6);
    bo_run_cash_out(run);
    BO_EQUAL_INT(run->dollars, 10);
    BO_EQUAL_INT(run->chips, 0);
    BO_EQUAL_INT(run->deck_count, 52);
    BO_EQUAL_INT(run->slot, BO_SLOT_BIG);
    BO_EQUAL_INT(run->state, BO_STATE_SHOP);
    bo_run_end_shop(run);
    BO_EQUAL_INT(run->state, BO_STATE_BLIND_SELECT);
    BO_EQUAL_INT(bo_run_slot_chips(run, BO_SLOT_BIG), 450);
    free(run);
}

BO_TEST(test_run_gives_interest_up_to_the_cap) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    run->dollars = 43;
    bo_run_select_blind(run);
    run->blind_chips = 1;
    BOScore score;
    bo_run_play(run, 0x1, &score);
    const BOEvalRow *last = &run->eval.rows[run->eval.row_count - 1];
    BO_EQUAL_INT(last->kind, BO_EVAL_INTEREST);
    BO_EQUAL_INT(last->dollars, 5);
    free(run);
}

BO_TEST(test_run_ends_when_no_hand_is_left) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    bo_run_select_blind(run);
    run->blind_chips = 1e9;
    BOScore score;
    for (int i = 0; i < 4; i++) BO_EXPECT(bo_run_play(run, 0x1, &score));
    BO_EQUAL_INT(run->state, BO_STATE_GAME_OVER);
    BO_EXPECT(!bo_run_play(run, 0x1, &score));
    free(run);
}

BO_TEST(test_run_refuses_a_play_of_six_cards_or_a_discard_with_none_left) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    bo_run_select_blind(run);
    BOScore score;
    BO_EXPECT(!bo_run_play(run, 0x3f, &score));
    BO_EXPECT(!bo_run_play(run, 0, &score));
    for (int i = 0; i < 4; i++) BO_EXPECT(bo_run_discard(run, 0x1));
    BO_EXPECT(!bo_run_discard(run, 0x1));
    BO_EQUAL_INT(run->hand_count, 8);
    BO_EQUAL_INT(run->deck_count, 40);
    free(run);
}

BO_TEST(test_run_goes_to_the_next_ante_after_the_boss) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    BOScore score;
    for (int slot = 0; slot < 3; slot++) {
        bo_run_select_blind(run);
        run->blind_chips = 1;
        bo_run_play(run, 0x1, &score);
        bo_run_cash_out(run);
        bo_run_end_shop(run);
    }
    BO_EQUAL_INT(run->ante, 2);
    BO_EQUAL_INT(run->slot, BO_SLOT_SMALL);
    BO_EQUAL_INT(run->boss, BO_BL_ARM);
    BO_EQUAL_INT(run->round, 3);
    free(run);
}

BO_TEST(test_run_sorts_the_hand_by_suit) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    bo_run_select_blind(run);
    bo_run_sort_hand(run, 1);
    BO_EQUAL(names(run->hand, run->hand_count, 0), @"7S 2S 8H 7H 3H 2H KC 8D");
    free(run);
}
