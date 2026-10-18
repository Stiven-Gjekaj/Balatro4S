// Tests of the profile: the unlocks of check_for_unlock and the wins of
// each deck, from common_events.lua and misc_functions.lua of Balatro
// 1.0.1o.

#import "BOTest.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bo_pool.h"
#include "bo_profile.h"
#include "bo_rules.h"

static BOProfile *new_profile(void) {
    BOProfile *p = calloc(1, sizeof(BOProfile));
    bo_profile_init(p);
    bo_profile_set_active(p);
    return p;
}

static BORun *new_run(BOCenterId back, int stake) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start_deck(run, "ABCDEFGH", back, stake);
    return run;
}

static void done(BOProfile *p, BORun *run) {
    bo_profile_set_active(NULL);
    free(p);
    free(run);
}

BO_TEST(test_profile_starts_with_the_flags_of_game_lua) {
    BOProfile *p = new_profile();
    BO_EXPECT(bo_profile_unlocked(BO_J_JOKER));
    BO_EXPECT(bo_profile_discovered(BO_J_JOKER));
    BO_EXPECT(!bo_profile_unlocked(BO_B_BLUE));
    BO_EXPECT(!bo_profile_unlocked(BO_J_BURNT));
    // A consumable cannot be locked.
    BO_EXPECT(bo_profile_unlocked(BO_C_FOOL));
    BO_EXPECT(!bo_profile_discovered(BO_C_FOOL));
    BO_EQUAL_INT(bo_profile_max_stake(p, BO_B_RED), 1);
    bo_profile_set_active(NULL);
    // With no active profile, all cards are unlocked and discovered.
    BO_EXPECT(bo_profile_unlocked(BO_B_BLUE));
    BO_EXPECT(bo_profile_discovered(BO_C_FOOL));
    free(p);
}

BO_TEST(test_profile_win_unlocks_the_next_deck_and_stake) {
    BOProfile *p = new_profile();
    BORun *run = new_run(BO_B_RED, 2);
    run->round = 20;
    bo_profile_win(run);
    // set_deck_win counts the stake and each stake under it.
    BO_EQUAL_INT(p->decks[0].wins[1], 1);
    BO_EQUAL_INT(p->decks[0].wins[0], 1);
    BO_EQUAL_INT(bo_profile_max_stake(p, BO_B_RED), 3);
    BO_EQUAL_INT(bo_profile_max_stake(p, BO_B_BLUE), 1);
    // win_deck b_red unlocks the Magic Deck, win_stake 2 the Zodiac Deck.
    BO_EXPECT(p->unlocked[BO_B_MAGIC]);
    BO_EXPECT(p->unlocked[BO_B_ZODIAC]);
    BO_EXPECT(!p->unlocked[BO_B_PAINTED]);
    // win with 18 rounds or fewer: Wee Joker. The run has 20.
    BO_EXPECT(!p->unlocked[BO_J_WEE]);
    // Blueprint unlocks with any win.
    BO_EXPECT(p->unlocked[BO_J_BLUEPRINT]);
    BO_EQUAL_INT((int)p->stats[BO_STAT_WINS], 1);
    BO_EQUAL_INT((int)p->high_scores[BO_HIGH_WIN_STREAK], 1);
    BO_EXPECT(bo_profile_take_notice(p) != BO_CENTER_NONE);
    done(p, run);
}

BO_TEST(test_profile_loss_breaks_the_streak) {
    BOProfile *p = new_profile();
    BORun *run = new_run(BO_B_BLUE, 1);
    p->high_scores[BO_HIGH_CURRENT_STREAK] = 3;
    bo_profile_lose(run);
    BO_EQUAL_INT((int)p->high_scores[BO_HIGH_CURRENT_STREAK], 0);
    BO_EQUAL_INT(p->decks[1].losses[0], 1);
    BO_EQUAL_INT((int)p->stats[BO_STAT_LOSSES], 1);
    done(p, run);
}

BO_TEST(test_profile_career_stat_and_chip_score_unlock_jokers) {
    BOProfile *p = new_profile();
    BORun *run = new_run(BO_B_RED, 1);
    bo_profile_stat(run, BO_STAT_CARDS_SOLD, 49);
    BO_EXPECT(!p->unlocked[BO_J_BURNT]);
    bo_profile_stat(run, BO_STAT_CARDS_SOLD, 1);
    BO_EXPECT(p->unlocked[BO_J_BURNT]);
    BOUnlockArgs score = {.type = BO_UNLOCK_CHIP_SCORE, .chips = 1000000};
    bo_profile_check(run, &score);
    BO_EXPECT(p->unlocked[BO_J_OOPS]);
    BO_EXPECT(p->unlocked[BO_J_IDOL]);
    BO_EXPECT(!p->unlocked[BO_J_STUNTMAN]);
    done(p, run);
}

BO_TEST(test_profile_modify_deck_counts_the_cards_of_the_run) {
    BOProfile *p = new_profile();
    BORun *run = new_run(BO_B_RED, 1);
    BOUnlockArgs args = {.type = BO_UNLOCK_MODIFY_DECK};
    bo_profile_check(run, &args);
    BO_EXPECT(!p->unlocked[BO_J_SMEARED]);
    for (int i = 0; i < 3; i++) run->deck[i].enhancement = BO_ENH_WILD;
    bo_profile_check(run, &args);
    BO_EXPECT(p->unlocked[BO_J_SMEARED]);
    BO_EXPECT(!p->unlocked[BO_J_DRIVERS_LICENSE]);
    done(p, run);
}

BO_TEST(test_profile_discoveries_unlock_the_blue_deck) {
    BOProfile *p = new_profile();
    BORun *run = new_run(BO_B_RED, 1);
    // The run start discovers the deck and its two blind tags. Each Tarot
    // card then adds one, and the 20th discovery unlocks the Blue Deck.
    int total;
    bo_profile_discover_tallies(p, &total, NULL, NULL, NULL);
    for (int i = 0; i < BO_CENTER_COUNT && total < 20; i++) {
        if (bo_center((BOCenterId)i)->set != BO_SET_TAROT || p->discovered[i]) continue;
        BO_EXPECT(!p->unlocked[BO_B_BLUE]);
        bo_profile_discover(run, (BOCenterId)i);
        bo_profile_discover_tallies(p, &total, NULL, NULL, NULL);
    }
    BO_EXPECT(p->unlocked[BO_B_BLUE]);
    // unlock_card discovers a deck, so the tally is 21.
    BO_EXPECT(p->discovered[BO_B_BLUE]);
    bo_profile_discover_tallies(p, &total, NULL, NULL, NULL);
    BO_EQUAL_INT(total, 21);
    BO_EXPECT(!p->unlocked[BO_J_CARTOMANCER]);
    done(p, run);
}

BO_TEST(test_profile_file_reads_back_and_refuses_a_bad_file) {
    BOProfile *p = new_profile();
    bo_profile_set_active(NULL);
    p->stats[BO_STAT_ROUNDS] = 12;
    p->decks[3].wins[4] = 2;
    char path[256];
    snprintf(path, sizeof(path), "%s/bo-profile-%d.b4p", NSTemporaryDirectory().fileSystemRepresentation, getpid());
    BO_EXPECT(bo_profile_write(p, path));
    BOProfile *loaded = calloc(1, sizeof(BOProfile));
    BO_EXPECT(bo_profile_read(loaded, path));
    BO_EXPECT(memcmp(p, loaded, sizeof(BOProfile)) == 0);
    BO_EXPECT(truncate(path, 100) == 0);
    loaded->stats[BO_STAT_ROUNDS] = 1;
    BO_EXPECT(!bo_profile_read(loaded, path));
    BO_EQUAL_INT((int)loaded->stats[BO_STAT_ROUNDS], 1);
    remove(path);
    free(p);
    free(loaded);
}

BO_TEST(test_profile_shop_has_no_locked_joker) {
    BOProfile *p = new_profile();
    BORun *run = new_run(BO_B_RED, 1);
    int jokers = 0;
    for (int i = 0; i < 200; i++) {
        BOShopCard card = bo_pool_create_shop_card(run);
        if (card.is_playing_card || bo_center(card.item.center)->set != BO_SET_JOKER) continue;
        jokers++;
        BO_EXPECT(p->unlocked[card.item.center]);
    }
    BO_EXPECT(jokers > 50);
    done(p, run);
}

BO_TEST(test_profile_play_and_round_count_the_stats) {
    BOProfile *p = new_profile();
    BORun *run = new_run(BO_B_RED, 1);
    bo_run_select_blind(run);
    BO_EQUAL_INT((int)p->high_scores[BO_HIGH_FURTHEST_ROUND], 1);
    run->blind_chips = 1;
    BOScore score;
    BO_EXPECT(bo_run_play(run, 3, &score));
    BO_EQUAL_INT((int)p->stats[BO_STAT_HANDS_PLAYED], 1);
    BO_EQUAL_INT((int)p->stats[BO_STAT_CARDS_PLAYED], 2);
    BO_EQUAL_INT(p->hand_usage[score.type], 1);
    BO_EXPECT(p->high_scores[BO_HIGH_HAND] == score.total);
    // The round is won with one hand: the streak of single hand rounds.
    BO_EQUAL_INT(run->state, BO_STATE_ROUND_EVAL);
    BO_EQUAL_INT((int)p->stats[BO_STAT_SINGLE_HAND_ROUND_STREAK], 1);
    BO_EXPECT(p->blind_discovered[BO_BL_SMALL]);
    bo_run_cash_out(run);
    BO_EQUAL_INT((int)p->stats[BO_STAT_ROUNDS], 1);
    BO_EXPECT(p->stats[BO_STAT_DOLLARS_EARNED] > 0);
    done(p, run);
}
