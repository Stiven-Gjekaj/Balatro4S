#include "bo_boss.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "bo_joker.h"
#include "bo_rules.h"
#include "bo_run.h"

#pragma STDC FP_CONTRACT OFF

static int is(const BORun *run, BOBlindId blind) {
    return run->blind == blind;
}

// The blind acts: it is on, and nothing disabled it.
static int acting(const BORun *run) {
    return run->blind_active && !run->blind_disabled;
}

static int suit_of_blind(const BORun *run) {
    switch (run->blind) {
    case BO_BL_CLUB: return BO_CLUBS;
    case BO_BL_GOAD: return BO_SPADES;
    case BO_BL_WINDOW: return BO_DIAMONDS;
    case BO_BL_HEAD: return BO_HEARTS;
    default: return -1;
    }
}

void bo_boss_debuff_card(BORun *run, BOCard *card) {
    if (acting(run)) {
        int suit = suit_of_blind(run);
        if (suit >= 0 && bo_rules_is_suit(run, card, suit, 1, 0)) {
            card->debuffed = 1;
            return;
        }
        if (is(run, BO_BL_PLANT) && bo_rules_is_face(run, card, 1)) {
            card->debuffed = 1;
            return;
        }
        if (is(run, BO_BL_PILLAR) && card->played_this_ante) {
            card->debuffed = 1;
            return;
        }
        if (is(run, BO_BL_FINAL_LEAF)) {
            card->debuffed = 1;
            return;
        }
    }
    card->debuffed = 0;
}

// Blind:debuff_card for a joker: Crimson Heart keeps its debuff, and each
// other joker loses its debuff.
static void debuff_joker(BORun *run, BOItem *joker) {
    if (acting(run) && is(run, BO_BL_FINAL_HEART)) return;
    bo_rules_set_debuff(run, joker, 1, 0);
}

static void debuff_playing_cards(BORun *run) {
    BOCard *areas[4] = {run->deck, run->hand, run->discard, run->play};
    int counts[4] = {run->deck_count, run->hand_count, run->discard_count, run->play_count};
    for (int a = 0; a < 4; a++) {
        for (int i = 0; i < counts[a]; i++) bo_boss_debuff_card(run, &areas[a][i]);
    }
}

void bo_boss_reset_debuffs(BORun *run) {
    debuff_playing_cards(run);
}

// CardArea:shuffle of the jokers: sort by sort_id, then pseudoshuffle.
static void shuffle_jokers(BORun *run, const char *key) {
    int count = run->joker_count;
    for (int i = 1; i < count; i++) {
        BOItem t = run->jokers[i];
        int j = i - 1;
        while (j >= 0 && run->jokers[j].sort_id > t.sort_id) {
            run->jokers[j + 1] = run->jokers[j];
            j--;
        }
        run->jokers[j + 1] = t;
    }
    int order[BO_MAX_JOKERS];
    bo_pseudoshuffle(&run->random, bo_pseudoseed(&run->random, key), order, count);
    BOItem shuffled[BO_MAX_JOKERS];
    for (int i = 0; i < count; i++) shuffled[i] = run->jokers[order[i]];
    memcpy(run->jokers, shuffled, sizeof(BOItem) * count);
}

void bo_boss_set_blind(BORun *run) {
    run->blind_active = 1;
    run->blind_disabled = 0;
    run->blind_triggered = 0;
    run->boss_prepped = 1;
    run->water_sub = 0;
    run->needle_sub = 0;
    if (is(run, BO_BL_EYE)) memset(run->eye_hands, 0, sizeof(run->eye_hands));
    if (is(run, BO_BL_MOUTH)) run->mouth_hand = BO_HAND_NONE;
    if (is(run, BO_BL_FISH)) run->boss_prepped = 0;
    if (is(run, BO_BL_WATER)) {
        run->water_sub = run->discards_left;
        bo_rules_ease_discard(run, -run->water_sub);
    }
    if (is(run, BO_BL_NEEDLE)) {
        run->needle_sub = run->hands_per_round - 1;
        run->hands_left -= run->needle_sub;
    }
    if (is(run, BO_BL_MANACLE)) bo_rules_change_hand_size(run, -1);
    if (is(run, BO_BL_FINAL_ACORN) && run->joker_count > 0) {
        for (int i = 0; i < run->joker_count; i++) run->jokers[i].face_down = 1;
        if (run->joker_count > 1) {
            for (int i = 0; i < 3; i++) shuffle_jokers(run, "aajk");
        }
    }
    debuff_playing_cards(run);
    for (int i = 0; i < run->joker_count; i++) debuff_joker(run, &run->jokers[i]);
}

int bo_boss_press_play(BORun *run) {
    if (!acting(run)) return 0;
    if (is(run, BO_BL_HOOK)) {
        // Two random cards of the hand, as pseudorandom_element picks them.
        int left[BO_MAX_HAND];
        int count = run->hand_count;
        for (int i = 0; i < count; i++) left[i] = i;
        uint64_t mask = 0;
        for (int pick = 0; pick < 2 && pick < run->hand_count; pick++) {
            // The cards sort by sort_id; the pick removes the card by its key.
            int sorted[BO_MAX_HAND];
            memcpy(sorted, left, sizeof(int) * count);
            for (int i = 1; i < count; i++) {
                int t = sorted[i], j = i - 1;
                while (j >= 0 && run->hand[sorted[j]].uid > run->hand[t].uid) {
                    sorted[j + 1] = sorted[j];
                    j--;
                }
                sorted[j + 1] = t;
            }
            int chosen = sorted[bo_random_index(&run->random, bo_pseudoseed(&run->random, "hook"), count)];
            mask |= (uint64_t)1 << chosen;
            for (int i = 0; i < count; i++) {
                if (left[i] == chosen) {
                    memmove(&left[i], &left[i + 1], sizeof(int) * (count - i - 1));
                    break;
                }
            }
            count--;
        }
        if (mask) bo_run_discard_cards(run, mask, 1);
        run->blind_triggered = 1;
        return 1;
    }
    if (is(run, BO_BL_FINAL_HEART) && run->joker_count > 0) {
        run->blind_triggered = 1;
        run->boss_prepped = 1;
    }
    if (is(run, BO_BL_FISH)) run->boss_prepped = 1;
    if (is(run, BO_BL_TOOTH)) {
        run->dollars -= run->play_count;
        run->blind_triggered = 1;
        return 1;
    }
    return 0;
}

int bo_boss_debuff_hand(BORun *run, int count, const BOHandResult *hands, BOHandType text, int check) {
    if (!acting(run)) return 0;
    run->blind_triggered = 0;
    if (is(run, BO_BL_PSYCHIC) && count < 5) {
        run->blind_triggered = 1;
        return 1;
    }
    if (is(run, BO_BL_EYE)) {
        if (run->eye_hands[text]) {
            run->blind_triggered = 1;
            return 1;
        }
        if (!check) run->eye_hands[text] = 1;
    }
    if (is(run, BO_BL_MOUTH)) {
        if (run->mouth_hand != BO_HAND_NONE && run->mouth_hand != (int)text) {
            run->blind_triggered = 1;
            return 1;
        }
        if (!check) run->mouth_hand = text;
    }
    if (is(run, BO_BL_ARM)) {
        run->blind_triggered = 0;
        if (run->hands[text].level > 1) {
            run->blind_triggered = 1;
            if (!check) bo_run_level_up_hand(run, text, -1);
        }
    }
    if (is(run, BO_BL_OX)) {
        run->blind_triggered = 0;
        if (text == run->most_played_hand) {
            run->blind_triggered = 1;
            // ease_dollars with instant: the money goes to 0 at once.
            if (!check) run->dollars -= run->dollars;
        }
    }
    (void)hands;
    return 0;
}

int bo_boss_modify_hand(BORun *run, double *mult, double *chips) {
    if (!acting(run) || !is(run, BO_BL_FLINT)) return 0;
    run->blind_triggered = 1;
    double m = floor(*mult * 0.5 + 0.5), c = floor(*chips * 0.5 + 0.5);
    *mult = m > 1 ? m : 1;
    *chips = c > 0 ? c : 0;
    return 1;
}

int bo_boss_stay_flipped(BORun *run, const BOCard *card) {
    if (!acting(run)) return 0;
    if (is(run, BO_BL_WHEEL) && bo_pseudorandom(&run->random, "wheel") < run->probabilities / 7) return 1;
    if (is(run, BO_BL_HOUSE) && run->hands_played == 0 && run->discards_used == 0) return 1;
    if (is(run, BO_BL_MARK) && bo_rules_is_face(run, card, 1)) return 1;
    if (is(run, BO_BL_FISH) && run->boss_prepped) return 1;
    return 0;
}

// pseudorandom_element of cards: sorted by sort_id.
static int random_hand_card(BORun *run, const char *key) {
    int sorted[BO_MAX_HAND];
    int count = run->hand_count;
    for (int i = 0; i < count; i++) sorted[i] = i;
    for (int i = 1; i < count; i++) {
        int t = sorted[i], j = i - 1;
        while (j >= 0 && run->hand[sorted[j]].uid > run->hand[t].uid) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = t;
    }
    return sorted[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), count)];
}

void bo_boss_drawn_to_hand(BORun *run) {
    if (acting(run)) {
        if (is(run, BO_BL_FINAL_BELL) && run->hand_count > 0) {
            int any = 0;
            for (int i = 0; i < run->hand_count; i++) any |= run->hand[i].forced_selection;
            if (!any) run->hand[random_hand_card(run, "cerulean_bell")].forced_selection = 1;
        }
        if (is(run, BO_BL_FINAL_HEART) && run->boss_prepped && run->joker_count > 0) {
            int list[BO_MAX_JOKERS], count = 0;
            for (int i = 0; i < run->joker_count; i++) {
                if (!run->jokers[i].debuffed || run->joker_count < 2) list[count++] = i;
                bo_rules_set_debuff(run, &run->jokers[i], 1, 0);
            }
            if (count > 0) {
                // pseudorandom_element of jokers: sorted by sort_id.
                for (int i = 1; i < count; i++) {
                    int t = list[i], j = i - 1;
                    while (j >= 0 && run->jokers[list[j]].sort_id > run->jokers[t].sort_id) {
                        list[j + 1] = list[j];
                        j--;
                    }
                    list[j + 1] = t;
                }
                int chosen = list[bo_random_index(&run->random, bo_pseudoseed(&run->random, "crimson_heart"), count)];
                bo_rules_set_debuff(run, &run->jokers[chosen], 1, 1);
            }
        }
    }
    run->boss_prepped = 0;
}

int bo_boss_draw_count(const BORun *run) {
    if (acting(run) && is(run, BO_BL_SERPENT) && (run->hands_played > 0 || run->discards_used > 0)) {
        return run->deck_count < 3 ? run->deck_count : 3;
    }
    return -1;
}

void bo_boss_disable(BORun *run) {
    if (!run->blind_active) return;
    run->blind_disabled = 1;
    for (int i = 0; i < run->joker_count; i++) run->jokers[i].face_down = 0;
    if (is(run, BO_BL_WATER)) bo_rules_ease_discard(run, run->water_sub);
    if (is(run, BO_BL_WHEEL) || is(run, BO_BL_HOUSE) || is(run, BO_BL_MARK) || is(run, BO_BL_FISH)) {
        for (int i = 0; i < run->hand_count; i++) run->hand[i].face_down = 0;
    }
    if (is(run, BO_BL_NEEDLE)) run->hands_left += run->needle_sub;
    if (is(run, BO_BL_WALL)) run->blind_chips = run->blind_chips / 2;
    if (is(run, BO_BL_FINAL_BELL)) {
        for (int i = 0; i < run->hand_count; i++) run->hand[i].forced_selection = 0;
        for (int i = 0; i < run->deck_count; i++) run->deck[i].forced_selection = 0;
        for (int i = 0; i < run->discard_count; i++) run->discard[i].forced_selection = 0;
    }
    if (is(run, BO_BL_MANACLE)) {
        bo_rules_change_hand_size(run, 1);
        bo_run_draw_cards(run, 1);
    }
    if (is(run, BO_BL_FINAL_VESSEL)) run->blind_chips = run->blind_chips / 3;
    debuff_playing_cards(run);
    for (int i = 0; i < run->joker_count; i++) debuff_joker(run, &run->jokers[i]);
}

void bo_boss_defeat(BORun *run) {
    for (int i = 0; i < run->joker_count; i++) run->jokers[i].face_down = 0;
    if (is(run, BO_BL_MANACLE) && !run->blind_disabled && run->blind_active) bo_rules_change_hand_size(run, 1);
    // set_blind(nil): no blind, so no card stays debuffed.
    run->blind_active = 0;
    debuff_playing_cards(run);
    for (int i = 0; i < run->joker_count; i++) debuff_joker(run, &run->jokers[i]);
}
