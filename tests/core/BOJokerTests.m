// Tests of the joker engine and the score. Each expected number comes from
// evaluate_play of Balatro 1.0.1o with the same cards, with
// tools/oracle.sh score. tools/fuzz.py compares many more random hands.

#import "BOTest.h"
#include <stdlib.h>
#include <string.h>
#include "bo_eval.h"
#include "bo_pool.h"
#include "bo_rules.h"
#include "bo_run.h"

// A run in the middle of a Small Blind, with no deck, as the oracle makes it.
static BORun *make_run(const char *seed, int dollars, int hands_left) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, seed);
    bo_random_start(&run->random, seed);
    run->deck_count = 0;
    run->state = BO_STATE_SELECTING;
    run->blind = BO_BL_SMALL;
    run->blind_chips = 300;
    run->dollars = dollars;
    run->hands_left = hands_left;
    run->discards_left = 3;
    run->idol_rank = run->mail_rank = -999;
    run->idol_suit = run->ancient_suit = run->castle_suit = BO_SPADES;
    return run;
}

static void add_joker(BORun *run, BOCenterId center, int edition) {
    BOItem item = bo_pool_make_item(run, center);
    item.edition = (uint8_t)edition;
    bo_run_add_joker(run, &item);
}

static BOCard card(BORun *run, int rank, int suit, int enhancement, int edition, int seal) {
    BOCard c = bo_rules_new_playing_card(run, rank, suit);
    c.enhancement = (uint8_t)enhancement;
    c.edition = (uint8_t)edition;
    c.seal = (uint8_t)seal;
    return c;
}

static BOScore play(BORun *run) {
    BOScore score;
    run->hands_left--;
    bo_eval_play(run, &score);
    return score;
}

BO_TEST(test_joker_scores_suit_and_type_jokers_with_editions_and_glass) {
    BORun *run = make_run("ABCDEFGH", 10, 4);
    add_joker(run, BO_J_GREEDY_JOKER, BO_EDITION_NONE);
    add_joker(run, BO_J_JOLLY, BO_EDITION_NONE);
    add_joker(run, BO_J_JOKER, BO_EDITION_HOLO);
    run->play[run->play_count++] = card(run, 9, BO_DIAMONDS, 0, 0, 0);
    run->play[run->play_count++] = card(run, 9, BO_DIAMONDS, BO_ENH_GLASS, BO_EDITION_POLYCHROME, 0);
    run->play[run->play_count++] = card(run, 4, BO_HEARTS, 0, 0, 0);
    run->hand[run->hand_count++] = card(run, BO_KING, BO_SPADES, BO_ENH_STEEL, 0, 0);
    run->hand[run->hand_count++] = card(run, BO_QUEEN, BO_SPADES, BO_ENH_STEEL, BO_EDITION_HOLO, BO_SEAL_RED);
    BOScore score = play(run);
    BO_EQUAL_INT(score.type, BO_HAND_PAIR);
    BO_EQUAL_INT(score.chips, 28);
    BO_EXPECT(score.mult == 82.75);
    BO_EQUAL_INT(run->chips, 2317);
    free(run);
}

BO_TEST(test_joker_copies_with_blueprint_and_multiplies_with_baseball) {
    BORun *run = make_run("ABCDEFGH", 10, 4);
    add_joker(run, BO_J_BLUEPRINT, BO_EDITION_NONE);
    add_joker(run, BO_J_DUO, BO_EDITION_NONE);
    add_joker(run, BO_J_BASEBALL, BO_EDITION_NONE);
    add_joker(run, BO_J_MISPRINT, BO_EDITION_NONE);
    run->play[run->play_count++] = card(run, BO_KING, BO_SPADES, 0, 0, 0);
    run->play[run->play_count++] = card(run, BO_KING, BO_HEARTS, 0, 0, 0);
    run->play[run->play_count++] = card(run, 3, BO_DIAMONDS, 0, 0, 0);
    run->hand[run->hand_count++] = card(run, BO_KING, BO_CLUBS, BO_ENH_STEEL, 0, BO_SEAL_RED);
    BOScore score = play(run);
    BO_EQUAL_INT(score.chips, 30);
    BO_EQUAL_INT(score.mult, 37);
    BO_EQUAL_INT(run->chips, 1110);
    free(run);
}

BO_TEST(test_joker_counts_steel_before_vampire_and_waits_with_the_money) {
    BORun *run = make_run("ZZZZZZZZ", 3, 2);
    add_joker(run, BO_J_VAMPIRE, BO_EDITION_NONE);
    add_joker(run, BO_J_STEEL_JOKER, BO_EDITION_NONE);
    add_joker(run, BO_J_BULL, BO_EDITION_NONE);
    add_joker(run, BO_J_VAGABOND, BO_EDITION_NONE);
    run->play[run->play_count++] = card(run, 7, BO_HEARTS, BO_ENH_STEEL, 0, 0);
    run->play[run->play_count++] = card(run, 7, BO_HEARTS, 0, 0, BO_SEAL_GOLD);
    BOScore score = play(run);
    // Steel Joker still counts the Steel Card that Vampire takes, as the
    // tally of Card:update does. Bull sees the $3 of the Gold Seal in the
    // buffer, and Vagabond sees only the $3 that was there before.
    BO_EQUAL_INT(score.chips, 36);
    BO_EXPECT(score.mult > 2.6399 && score.mult < 2.6401);
    BO_EQUAL_INT(run->chips, 95);
    BO_EQUAL_INT(run->dollars, 6);
    BO_EQUAL_INT(run->consumable_count, 1);
    free(run);
}

BO_TEST(test_joker_leaves_no_negative_discards) {
    BORun *run = make_run("ABCDEFGH", 10, 4);
    add_joker(run, BO_J_DRUNKARD, BO_EDITION_NONE);
    BO_EQUAL_INT(run->discards_left, 4);
    run->discards_left = 0;
    // remove_from_deck uses ease_discard, which stops at 0.
    bo_run_remove_joker(run, 0);
    BO_EQUAL_INT(run->discards_left, 0);
    free(run);
}
