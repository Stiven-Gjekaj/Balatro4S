// Tests of the boss blinds. The expected values come from Blind of Balatro
// 1.0.1o with the same cards, with tools/oracle.sh score and a "blind"
// line. tools/fuzz.py --blind compares many more random plays.

#import "BOTest.h"
#include <stdlib.h>
#include "bo_boss.h"
#include "bo_eval.h"
#include "bo_pool.h"
#include "bo_rules.h"
#include "bo_run.h"

// A run in the middle of a round, with no deck, as the oracle makes it.
static BORun *make_run(void) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    bo_random_start(&run->random, "ABCDEFGH");
    run->deck_count = 0;
    run->state = BO_STATE_SELECTING;
    run->blind_chips = 300;
    run->dollars = 10;
    run->hands_left = 4;
    run->discards_left = 3;
    run->idol_rank = run->mail_rank = -999;
    run->idol_suit = run->ancient_suit = run->castle_suit = BO_SPADES;
    return run;
}

static void add_joker(BORun *run, BOCenterId center) {
    BOItem item = bo_pool_make_item(run, center);
    bo_run_add_joker(run, &item);
}

static void play_card(BORun *run, int rank, int suit) {
    run->play[run->play_count++] = bo_rules_new_playing_card(run, rank, suit);
}

static void hold(BORun *run, int rank, int suit) {
    run->hand[run->hand_count++] = bo_rules_new_playing_card(run, rank, suit);
}

// The blind starts, the play starts, and the hand scores.
static BOScore play(BORun *run, BOBlindId blind) {
    run->blind = blind;
    bo_boss_set_blind(run);
    bo_boss_press_play(run);
    run->hands_left--;
    BOScore score;
    bo_eval_play(run, &score);
    return score;
}

BO_TEST(test_boss_hook_discards_two_cards_of_the_hand) {
    BORun *run = make_run();
    add_joker(run, BO_J_CASTLE);
    run->castle_suit = BO_HEARTS;
    play_card(run, BO_ACE, BO_SPADES);
    play_card(run, BO_KING, BO_SPADES);
    hold(run, 3, BO_HEARTS);
    hold(run, 4, BO_CLUBS);
    hold(run, 5, BO_DIAMONDS);
    hold(run, 9, BO_HEARTS);
    BOScore score = play(run, BO_BL_HOOK);
    // The Hook takes the 4 of Clubs and the 9 of Hearts, and Castle sees the
    // Heart. The discard is free.
    BO_EQUAL_INT(run->hand_count, 2);
    BO_EQUAL_INT(run->hand[0].rank, 3);
    BO_EQUAL_INT(run->hand[1].rank, 5);
    BO_EQUAL_INT(run->discards_left, 3);
    BO_EQUAL_INT(run->discard_count, 2);
    BO_EQUAL_INT(score.total, 19);
    free(run);
}

BO_TEST(test_boss_flint_halves_the_base_chips_and_mult) {
    BORun *run = make_run();
    bo_run_level_up_hand(run, BO_HAND_PAIR, 2);
    play_card(run, BO_ACE, BO_SPADES);
    play_card(run, BO_ACE, BO_HEARTS);
    play_card(run, BO_KING, BO_SPADES);
    BOScore score = play(run, BO_BL_FLINT);
    BO_EQUAL_INT(score.chips, 42);
    BO_EQUAL_INT(score.mult, 2);
    BO_EQUAL_INT(score.total, 84);
    free(run);
}

BO_TEST(test_boss_psychic_refuses_a_small_hand_and_pays_matador) {
    BORun *run = make_run();
    add_joker(run, BO_J_MATADOR);
    play_card(run, BO_ACE, BO_SPADES);
    play_card(run, BO_ACE, BO_HEARTS);
    BOScore score = play(run, BO_BL_PSYCHIC);
    BO_EXPECT(score.debuffed);
    BO_EQUAL_INT(score.total, 0);
    BO_EQUAL_INT(run->dollars, 18);
    free(run);
}

BO_TEST(test_boss_club_debuffs_clubs_until_its_defeat) {
    BORun *run = make_run();
    hold(run, BO_KING, BO_CLUBS);
    hold(run, BO_KING, BO_DIAMONDS);
    run->blind = BO_BL_CLUB;
    bo_boss_set_blind(run);
    BO_EXPECT(run->hand[0].debuffed);
    BO_EXPECT(!run->hand[1].debuffed);
    bo_boss_defeat(run);
    BO_EXPECT(!run->hand[0].debuffed);
    free(run);
}

BO_TEST(test_boss_water_needle_and_manacle_come_back_when_disabled) {
    BORun *run = make_run();
    run->blind = BO_BL_WATER;
    bo_boss_set_blind(run);
    BO_EQUAL_INT(run->discards_left, 0);
    bo_boss_disable(run);
    BO_EQUAL_INT(run->discards_left, 3);
    run->blind = BO_BL_NEEDLE;
    bo_boss_set_blind(run);
    BO_EQUAL_INT(run->hands_left, 1);
    bo_boss_disable(run);
    BO_EQUAL_INT(run->hands_left, 4);
    run->blind = BO_BL_MANACLE;
    bo_boss_set_blind(run);
    BO_EQUAL_INT(run->hand_size, 7);
    bo_boss_defeat(run);
    BO_EQUAL_INT(run->hand_size, 8);
    free(run);
}

BO_TEST(test_boss_serpent_draws_three_after_a_play) {
    BORun *run = make_run();
    for (int i = 0; i < 10; i++) run->deck[run->deck_count++] = bo_rules_new_playing_card(run, 2 + i, BO_SPADES);
    run->blind = BO_BL_SERPENT;
    bo_boss_set_blind(run);
    BO_EQUAL_INT(bo_boss_draw_count(run), -1);
    run->hands_played = 1;
    BO_EQUAL_INT(bo_boss_draw_count(run), 3);
    bo_boss_disable(run);
    BO_EQUAL_INT(bo_boss_draw_count(run), -1);
    free(run);
}
