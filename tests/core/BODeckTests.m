// Tests of the 15 decks. The expected values come from Game:start_run and
// Back:apply_to_run of Balatro 1.0.1o, with tools/oracle.sh score and a
// "start" line. tools/fuzz.py --start compares many more random starts.

#import "BOTest.h"
#include <stdlib.h>
#include "bo_eval.h"
#include "bo_rules.h"
#include "bo_run.h"

static BORun *start(BOCenterId back) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start_deck(run, "ABCDEFGH", back, 1);
    return run;
}

// The card on the top of the deck, then the next ones.
static void expect_top(BORun *run, const int *ranks, const int *suits, int count) {
    for (int i = 0; i < count; i++) {
        const BOCard *card = &run->deck[run->deck_count - 1 - i];
        BO_EQUAL_INT(card->rank, ranks[i]);
        BO_EQUAL_INT(card->suit, suits[i]);
    }
}

BO_TEST(test_deck_red_starts_with_a_fourth_discard_and_any_ancient_suit) {
    BORun *run = start(BO_B_RED);
    BO_EQUAL_INT(run->hands_per_round, 4);
    BO_EQUAL_INT(run->discards_per_round, 4);
    BO_EQUAL_INT(run->deck_count, 52);
    // start_run clears the suit of Ancient Joker, so Spades can come first.
    BO_EQUAL_INT(run->ancient_suit, BO_SPADES);
    static const int ranks[] = {6, BO_QUEEN, 8, BO_KING};
    static const int suits[] = {BO_CLUBS, BO_CLUBS, BO_DIAMONDS, BO_CLUBS};
    expect_top(run, ranks, suits, 4);
    free(run);
}

BO_TEST(test_deck_erratic_takes_random_cards) {
    BORun *run = start(BO_B_ERRATIC);
    BO_EQUAL_INT(run->discards_per_round, 3);
    BO_EQUAL_INT(run->deck_count, 52);
    static const int ranks[] = {6, 9, 8, 8, 3, BO_ACE};
    static const int suits[] = {BO_CLUBS, BO_CLUBS, BO_DIAMONDS, BO_CLUBS, BO_DIAMONDS, BO_SPADES};
    expect_top(run, ranks, suits, 6);
    free(run);
}

BO_TEST(test_deck_abandoned_and_checkered_change_the_cards) {
    BORun *run = start(BO_B_ABANDONED);
    BO_EQUAL_INT(run->deck_count, 40);
    BO_EQUAL_INT(run->starting_deck_size, 40);
    for (int i = 0; i < run->deck_count; i++) BO_EXPECT(run->deck[i].rank < BO_JACK || run->deck[i].rank == BO_ACE);
    free(run);
    run = start(BO_B_CHECKERED);
    int spades = 0, hearts = 0;
    for (int i = 0; i < run->deck_count; i++) {
        spades += run->deck[i].suit == BO_SPADES;
        hearts += run->deck[i].suit == BO_HEARTS;
    }
    BO_EQUAL_INT(spades, 26);
    BO_EQUAL_INT(hearts, 26);
    free(run);
}

BO_TEST(test_deck_blue_stake_takes_a_discard) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start_deck(run, "ABCDEFGH", BO_B_RED, 5);
    BO_EQUAL_INT(run->discards_per_round, 3);
    BO_EQUAL_INT(run->discards_left, 3);
    free(run);
}

BO_TEST(test_deck_slots_money_and_hands) {
    BORun *run = start(BO_B_PAINTED);
    BO_EQUAL_INT(run->hand_size, 10);
    BO_EQUAL_INT(run->joker_slots, 4);
    free(run);
    run = start(BO_B_BLACK);
    BO_EQUAL_INT(run->hands_per_round, 3);
    BO_EQUAL_INT(run->joker_slots, 6);
    free(run);
    run = start(BO_B_YELLOW);
    BO_EQUAL_INT(run->dollars, 14);
    free(run);
    run = start(BO_B_NEBULA);
    BO_EQUAL_INT(run->consumable_slots, 1);
    BO_EXPECT(run->used_vouchers[BO_V_TELESCOPE]);
    free(run);
}

BO_TEST(test_deck_magic_and_ghost_give_consumables) {
    BORun *run = start(BO_B_MAGIC);
    BO_EQUAL_INT(run->consumable_slots, 3);
    BO_EXPECT(run->used_vouchers[BO_V_CRYSTAL_BALL]);
    BO_EQUAL_INT(run->consumable_count, 2);
    BO_EQUAL_INT(run->consumables[0].center, BO_C_FOOL);
    BO_EQUAL_INT(run->consumables[1].center, BO_C_FOOL);
    free(run);
    run = start(BO_B_GHOST);
    BO_EQUAL_INT(run->consumable_count, 1);
    BO_EQUAL_INT(run->consumables[0].center, BO_C_HEX);
    BO_EXPECT(run->spectral_rate == 2);
    free(run);
}

BO_TEST(test_deck_plasma_doubles_the_blind_and_balances_the_score) {
    BORun *run = start(BO_B_PLASMA);
    BO_EXPECT(bo_run_slot_chips(run, BO_SLOT_SMALL) == 600);
    run->deck_count = 0;
    run->state = BO_STATE_SELECTING;
    run->play[run->play_count++] = bo_rules_new_playing_card(run, BO_ACE, BO_SPADES);
    run->play[run->play_count++] = bo_rules_new_playing_card(run, BO_ACE, BO_HEARTS);
    BOScore score;
    bo_eval_play(run, &score);
    // A Pair of Aces: 10 + 11 + 11 chips and 2 mult make 34, so 17 and 17.
    BO_EXPECT(score.chips == 17);
    BO_EXPECT(score.mult == 17);
    BO_EXPECT(score.total == 289);
    free(run);
}

BO_TEST(test_deck_green_pays_for_hands_and_discards_with_no_interest) {
    BORun *run = start(BO_B_GREEN);
    bo_run_select_blind(run);
    run->dollars = 50;
    run->hands_left = 2;
    run->discards_left = 3;
    run->chips = run->blind_chips;
    bo_run_end_round(run);
    BO_EQUAL_INT(run->eval.row_count, 3);
    BO_EQUAL_INT(run->eval.rows[1].kind, BO_EVAL_HANDS);
    BO_EQUAL_INT(run->eval.rows[1].dollars, 4);
    BO_EQUAL_INT(run->eval.rows[2].kind, BO_EVAL_DISCARDS);
    BO_EQUAL_INT(run->eval.rows[2].dollars, 3);
    free(run);
}

BO_TEST(test_deck_anaglyph_gives_a_double_tag_after_a_boss) {
    BORun *run = start(BO_B_ANAGLYPH);
    run->slot = BO_SLOT_BOSS;
    bo_run_select_blind(run);
    run->chips = run->blind_chips;
    bo_run_end_round(run);
    BO_EQUAL_INT(run->tag_count, 1);
    BO_EQUAL_INT(run->tags[0].center, BO_TAG_DOUBLE);
    free(run);
}
