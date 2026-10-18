#include "bo_profile.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bo_challenge_data.h"
#include "bo_eval.h"
#include "bo_hand.h"
#include "bo_rules.h"

static BOProfile *gActive;

// A challenge counts no stat, unlocks no card, and discovers no card, as
// inc_career_stat, check_for_unlock, and discover_card.
static int counting(const BORun *run) {
    return gActive && !(run && run->challenge);
}

// The names of career_stats, as the types of the unlock conditions.
static const char *kStatNames[BO_STAT_COUNT] = {
    "c_round_interest_cap_streak", "c_dollars_earned", "c_shop_dollars_spent", "c_tarots_bought",
    "c_planets_bought", "c_playing_cards_bought", "c_vouchers_bought", "c_tarot_reading_used",
    "c_planetarium_used", "c_shop_rerolls", "c_cards_played", "c_cards_discarded", "c_losses", "c_wins",
    "c_rounds", "c_hands_played", "c_face_cards_played", "c_jokers_sold", "c_cards_sold",
    "c_single_hand_round_streak",
};

// Only the jokers, the decks, and the vouchers can be locked, as P_LOCKED.
static int can_lock(BOCenterId center) {
    int set = bo_center(center)->set;
    return set == BO_SET_JOKER || set == BO_SET_BACK || set == BO_SET_VOUCHER;
}

void bo_profile_init(BOProfile *profile) {
    memset(profile, 0, sizeof(*profile));
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        const BOCenterInfo *c = bo_center((BOCenterId)i);
        profile->unlocked[i] = c->unlocked || !can_lock((BOCenterId)i);
        profile->discovered[i] = c->discovered;
    }
}

void bo_profile_set_active(BOProfile *profile) {
    gActive = profile;
}

BOProfile *bo_profile_active(void) {
    return gActive;
}

void bo_profile_unlock_all(BOProfile *profile) {
    profile->all_unlocked = 1;
    for (int i = 0; i < BO_CENTER_COUNT; i++) profile->unlocked[i] = profile->discovered[i] = 1;
    for (int i = 0; i < BO_BLIND_COUNT; i++) profile->blind_discovered[i] = 1;
}

int bo_profile_unlocked(BOCenterId center) {
    return !gActive || center < 0 || center >= BO_CENTER_COUNT || gActive->unlocked[center];
}

int bo_profile_discovered(BOCenterId center) {
    return !gActive || center < 0 || center >= BO_CENTER_COUNT || gActive->discovered[center];
}

int bo_profile_deck_win_stake(const BOProfile *profile, BOCenterId back) {
    int best = 0;
    for (int d = 0; d < BO_DECK_COUNT; d++) {
        if (back != BO_CENTER_NONE && d != back - BO_B_RED) continue;
        for (int s = 0; s < BO_STAKE_COUNT; s++) {
            if (profile->decks[d].wins[s] > 0 && s + 1 > best) best = s + 1;
        }
    }
    return best;
}

int bo_profile_max_stake(const BOProfile *profile, BOCenterId back) {
    if (!profile || profile->all_unlocked) return BO_STAKE_COUNT;
    int max = bo_profile_deck_win_stake(profile, back) + 1;
    return max > BO_STAKE_COUNT ? BO_STAKE_COUNT : max;
}

// unlock_card, with a notice for the screen.
static void unlock(BOProfile *profile, BOCenterId center) {
    if (profile->unlocked[center]) return;
    profile->unlocked[center] = 1;
    if (bo_center(center)->set == BO_SET_BACK) profile->discovered[center] = 1;
    if (profile->notice_count < BO_PROFILE_NOTICES) profile->notices[profile->notice_count++] = center;
}

BOCenterId bo_profile_take_notice(BOProfile *profile) {
    if (profile->notice_count == 0) return BO_CENTER_NONE;
    BOCenterId first = profile->notices[0];
    memmove(profile->notices, profile->notices + 1, sizeof(BOCenterId) * (size_t)(profile->notice_count - 1));
    profile->notice_count--;
    return first;
}

void bo_profile_discover_tallies(const BOProfile *profile, int *total, int *of, int *tarots, int *planets) {
    int t = 0, o = 0, ta = 0, pl = 0;
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        const BOCenterInfo *c = bo_center((BOCenterId)i);
        if (c->omit && c->set != BO_SET_TAG) continue;
        int counted = c->set == BO_SET_JOKER || c->consumeable || c->set == BO_SET_EDITION ||
                      c->set == BO_SET_VOUCHER || c->set == BO_SET_BACK || c->set == BO_SET_BOOSTER ||
                      c->set == BO_SET_TAG;
        if (!counted) continue;
        o++;
        if (profile->discovered[i]) t++;
        if (c->set == BO_SET_TAROT && profile->discovered[i]) ta++;
        if (c->set == BO_SET_PLANET && profile->discovered[i]) pl++;
    }
    if (total) *total = t;
    if (of) *of = o;
    if (tarots) *tarots = ta;
    if (planets) *planets = pl;
}

static int count_jokers(const BORun *run, int (*test)(const BOItem *)) {
    int count = 0;
    for (int i = 0; i < run->joker_count; i++) {
        if (bo_item_center(&run->jokers[i])->set == BO_SET_JOKER && test(&run->jokers[i])) count++;
    }
    return count;
}

static int has_edition(const BOItem *item) {
    return item->edition != 0;
}

static int is_polychrome(const BOItem *item) {
    return item->edition == BO_EDITION_POLYCHROME;
}

// All the playing cards of the run: the deck, the hand, the play, and the
// discard pile.
static int playing_cards(const BORun *run, const BOCard **out, int max) {
    const BOCard *areas[4] = {run->deck, run->hand, run->play, run->discard};
    int counts[4] = {run->deck_count, run->hand_count, run->play_count, run->discard_count};
    int n = 0;
    for (int a = 0; a < 4; a++) {
        for (int i = 0; i < counts[a] && n < max; i++) {
            if (areas[a][i].shattered || areas[a][i].destroyed) continue;
            out[n++] = &areas[a][i];
        }
    }
    return n;
}

static int is_named(BOCenterId center, BOCenterId named) {
    return center == named;
}

// One unlock condition of check_for_unlock.
static int condition_true(BORun *run, BOProfile *p, BOCenterId id, const BOUnlockArgs *args) {
    const BOCenterInfo *c = bo_center(id);
    const char *type = c->unlock_type;
    double n = c->unlock_number;
    switch (args->type) {
    case BO_UNLOCK_CAREER_STAT:
        return strcmp(type, kStatNames[args->stat]) == 0 && p->stats[args->stat] >= n;
    case BO_UNLOCK_MONEY:
        return strcmp(type, "money") == 0 && run->dollars >= n;
    case BO_UNLOCK_ANTE_UP:
        return strcmp(type, "ante_up") == 0 && args->ante == (int)n;
    case BO_UNLOCK_ROUND_WIN:
        if (strcmp(type, "round_win") != 0) return 0;
        if (is_named(id, BO_J_MATADOR)) {
            return run->hands_played == 1 && run->discards_left == run->discards_per_round &&
                   bo_blind_info(run->blind)->boss;
        }
        if (is_named(id, BO_J_TROUBADOUR)) return p->stats[BO_STAT_SINGLE_HAND_ROUND_STREAK] >= n;
        if (is_named(id, BO_J_HANGING_CHAD)) {
            return run->last_hand_played == bo_rules_hand_by_name(c->unlock_text) && bo_blind_info(run->blind)->boss;
        }
        return 0;
    case BO_UNLOCK_WIN:
        return strcmp(type, "win") == 0 && n >= run->round;
    case BO_UNLOCK_WIN_DECK:
        return strcmp(type, "win_deck") == 0 && bo_profile_deck_win_stake(p, bo_center_by_key(c->unlock_text)) > 0;
    case BO_UNLOCK_WIN_STAKE:
        return strcmp(type, "win_stake") == 0 && bo_profile_deck_win_stake(p, BO_CENTER_NONE) >= n;
    case BO_UNLOCK_WIN_NO_HAND: {
        if (strcmp(type, "win_no_hand") != 0) return 0;
        int hand = bo_rules_hand_by_name(c->unlock_text);
        return hand >= 0 && run->hands[hand].played == 0;
    }
    case BO_UNLOCK_WIN_CUSTOM:
        if (strcmp(type, "win_custom") != 0) return 0;
        if (is_named(id, BO_J_INVISIBLE)) return run->max_jokers <= 4;
        return is_named(id, BO_J_BLUEPRINT);
    case BO_UNLOCK_DISCARD_CUSTOM: {
        if (strcmp(type, "discard_custom") != 0) return 0;
        if (is_named(id, BO_J_HIT_THE_ROAD)) {
            int jacks = 0;
            for (int i = 0; i < args->card_count; i++) jacks += bo_rules_get_id(&args->cards[i]) == BO_JACK;
            return jacks >= 5;
        }
        if (is_named(id, BO_J_BRAINSTORM)) {
            // A Royal Flush: a Straight Flush with no card under 10.
            BOHandRules rules = bo_eval_rules(run);
            BOHandResult result = bo_hand_evaluate(args->cards, args->card_count, &rules);
            if (!bo_hand_contains(&result, BO_HAND_STRAIGHT_FLUSH)) return 0;
            for (int i = 0; i < args->card_count; i++) {
                if (bo_rules_get_id(&args->cards[i]) < 10) return 0;
            }
            return 1;
        }
        return 0;
    }
    case BO_UNLOCK_HAND_CONTENTS: {
        if (strcmp(type, "hand_contents") != 0) return 0;
        int tally = 0;
        for (int i = 0; i < args->card_count; i++) {
            const BOCard *card = &args->cards[i];
            if (is_named(id, BO_J_SEEING_DOUBLE)) {
                tally += bo_rules_get_id(card) == 7 && bo_rules_is_suit(run, card, BO_CLUBS, 0, 0);
            } else if (is_named(id, BO_J_TICKET)) {
                tally += card->enhancement == BO_ENH_GOLD;
            }
        }
        return is_named(id, BO_J_SEEING_DOUBLE) ? tally >= 4 : tally >= 5;
    }
    case BO_UNLOCK_PLAY_ALL_HEARTS: {
        if (strcmp(type, "play_all_hearts") != 0) return 0;
        const BOCard *areas[2] = {run->deck, run->hand};
        int counts[2] = {run->deck_count, run->hand_count};
        for (int a = 0; a < 2; a++) {
            for (int i = 0; i < counts[a]; i++) {
                if (!bo_card_is_stone(&areas[a][i]) && areas[a][i].suit == BO_HEARTS) return 0;
            }
        }
        return 1;
    }
    case BO_UNLOCK_RUN_CARD_REPLAYS: {
        if (strcmp(type, "run_card_replays") != 0) return 0;
        const BOCard *cards[256];
        int count = playing_cards(run, cards, 256);
        for (int i = 0; i < count; i++) {
            if (cards[i]->times_played >= n) return 1;
        }
        return 0;
    }
    case BO_UNLOCK_CHIP_SCORE:
        return strcmp(type, "chip_score") == 0 && args->chips >= n;
    case BO_UNLOCK_MODIFY_DECK: {
        if (strcmp(type, "modify_deck") != 0) return 0;
        const BOCard *cards[256];
        int count = playing_cards(run, cards, 256), tally = 0;
        for (int i = 0; i < count; i++) {
            const BOCard *card = cards[i];
            if (c->unlock_kind == 1) {
                tally += card->suit == bo_rules_suit_by_name(c->unlock_text);
            } else if (c->unlock_kind == 2) {
                tally += bo_rules_enhancement_center(card->enhancement) == bo_center_by_key(c->unlock_text);
            } else if (c->unlock_kind == 3) {
                tally += card->enhancement != 0;
            }
        }
        return c->unlock_kind != 0 && tally >= n;
    }
    case BO_UNLOCK_MODIFY_JOKERS:
        return strcmp(type, "modify_jokers") == 0 && count_jokers(run, is_polychrome) >= n;
    case BO_UNLOCK_HAVE_EDITION:
        return strcmp(type, "have_edition") == 0 && count_jokers(run, has_edition) >= n;
    case BO_UNLOCK_MIN_HAND_SIZE:
        return strcmp(type, "min_hand_size") == 0 && run->hand_size <= n;
    case BO_UNLOCK_INTEREST_STREAK:
        return strcmp(type, "interest_streak") == 0 && p->stats[BO_STAT_ROUND_INTEREST_CAP_STREAK] >= n;
    case BO_UNLOCK_RUN_REDEEM: {
        if (strcmp(type, "run_redeem") != 0) return 0;
        int redeemed = 0;
        for (int i = 0; i < BO_CENTER_COUNT; i++) redeemed += run->used_vouchers[i] != 0;
        return redeemed >= n;
    }
    case BO_UNLOCK_BLANK_REDEEMS:
        return strcmp(type, "blank_redeems") == 0 && p->blank_redeems >= n;
    case BO_UNLOCK_DOUBLE_GOLD:
        return strcmp(type, "double_gold") == 0;
    case BO_UNLOCK_CONTINUE_GAME:
        return strcmp(type, "continue_game") == 0;
    case BO_UNLOCK_DISCOVER_AMOUNT: {
        if (strcmp(type, "discover_amount") != 0) return 0;
        int total, tarots, planets;
        bo_profile_discover_tallies(p, &total, NULL, &tarots, &planets);
        if (c->unlock_kind == 1) return tarots >= n;
        if (c->unlock_kind == 2) return planets >= n;
        return total >= n;
    }
    case BO_UNLOCK_BLIND_DISCOVERIES: {
        if (strcmp(type, "blind_discoveries") != 0) return 0;
        int count = 0;
        for (int i = 0; i < BO_BLIND_COUNT; i++) count += p->blind_discovered[i];
        return count >= n;
    }
    }
    return 0;
}

void bo_profile_check(BORun *run, const BOUnlockArgs *args) {
    BOProfile *p = gActive;
    if (!counting(run)) return;
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        if (p->unlocked[i] || !bo_center((BOCenterId)i)->unlock_type) continue;
        if (condition_true(run, p, (BOCenterId)i, args)) unlock(p, (BOCenterId)i);
    }
}

void bo_profile_check_cards(BORun *run) {
    if (!counting(run)) return;
    static const BOUnlockType checks[] = {BO_UNLOCK_MODIFY_DECK, BO_UNLOCK_MODIFY_JOKERS, BO_UNLOCK_HAVE_EDITION,
                                          BO_UNLOCK_MIN_HAND_SIZE};
    for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); i++) {
        BOUnlockArgs args = {.type = checks[i]};
        bo_profile_check(run, &args);
    }
    const BOCard *cards[256];
    int count = playing_cards(run, cards, 256);
    for (int i = 0; i < count; i++) {
        if (cards[i]->enhancement == BO_ENH_GOLD && cards[i]->seal == BO_SEAL_GOLD) {
            BOUnlockArgs args = {.type = BO_UNLOCK_DOUBLE_GOLD};
            bo_profile_check(run, &args);
            break;
        }
    }
    for (int i = 0; i < run->joker_count; i++) {
        bo_profile_discover(run, run->jokers[i].center);
        if (run->jokers[i].edition) bo_profile_discover(run, (BOCenterId)(BO_E_BASE + run->jokers[i].edition));
    }
    if (run->joker_count > run->max_jokers) run->max_jokers = run->joker_count;
}

void bo_profile_play(BORun *run, const BOCard *cards, int count, BOHandType hand, double chips) {
    BOProfile *p = gActive;
    if (!p) return;
    bo_profile_stat(run, BO_STAT_CARDS_PLAYED, count);
    bo_profile_stat(run, BO_STAT_HANDS_PLAYED, 1);
    for (int i = 0; i < count; i++) {
        if (bo_rules_is_face(run, &cards[i], 0)) bo_profile_stat(run, BO_STAT_FACE_CARDS_PLAYED, 1);
    }
    if (hand >= 0 && hand < BO_HAND_COUNT && counting(run)) p->hand_usage[hand]++;
    BOUnlockArgs replays = {.type = BO_UNLOCK_RUN_CARD_REPLAYS};
    bo_profile_check(run, &replays);
    BOUnlockArgs contents = {.type = BO_UNLOCK_HAND_CONTENTS, .cards = cards, .card_count = count};
    bo_profile_check(run, &contents);
    BOUnlockArgs hearts = {.type = BO_UNLOCK_PLAY_ALL_HEARTS};
    bo_profile_check(run, &hearts);
    bo_profile_high_score(BO_HIGH_HAND, chips);
    BOUnlockArgs score = {.type = BO_UNLOCK_CHIP_SCORE, .chips = chips};
    bo_profile_check(run, &score);
    bo_profile_check_cards(run);
}

void bo_profile_discard(BORun *run, const BOCard *cards, int count) {
    if (!counting(run)) return;
    bo_profile_stat(run, BO_STAT_CARDS_DISCARDED, count);
    BOUnlockArgs args = {.type = BO_UNLOCK_DISCARD_CUSTOM, .cards = cards, .card_count = count};
    bo_profile_check(run, &args);
}

void bo_profile_round_won(BORun *run, int interest_steps) {
    BOProfile *p = gActive;
    if (!p) return;
    bo_profile_discover_blind(run, run->blind);
    if (bo_blind_info(run->blind)->boss) {
        run->current_boss_streak++;
        bo_profile_high_score(BO_HIGH_BOSS_STREAK, run->current_boss_streak);
    }
    if (run->hands_played == 1) {
        bo_profile_stat(run, BO_STAT_SINGLE_HAND_ROUND_STREAK, 1);
    } else if (counting(run)) {
        p->stats[BO_STAT_SINGLE_HAND_ROUND_STREAK] = 0;
    }
    BOUnlockArgs args = {.type = BO_UNLOCK_ROUND_WIN};
    bo_profile_check(run, &args);
    if (interest_steps >= 0 && counting(run)) {
        if (interest_steps == run->interest_cap / 5) {
            p->stats[BO_STAT_ROUND_INTEREST_CAP_STREAK] += 1;
        } else {
            p->stats[BO_STAT_ROUND_INTEREST_CAP_STREAK] = 0;
        }
        BOUnlockArgs streak = {.type = BO_UNLOCK_INTEREST_STREAK};
        bo_profile_check(run, &streak);
    }
}

void bo_profile_dollars(BORun *run, int amount) {
    if (!gActive) return;
    if (amount > 0) bo_profile_stat(run, BO_STAT_DOLLARS_EARNED, amount);
    bo_profile_high_score(BO_HIGH_MOST_MONEY, run->dollars);
    BOUnlockArgs args = {.type = BO_UNLOCK_MONEY};
    bo_profile_check(run, &args);
}

void bo_profile_stat(BORun *run, BOStat stat, double amount) {
    if (!counting(run)) return;
    gActive->stats[stat] += amount;
    BOUnlockArgs args = {.type = BO_UNLOCK_CAREER_STAT, .stat = stat};
    bo_profile_check(run, &args);
}

void bo_profile_high_score(BOHighScore score, double amount) {
    if (!gActive) return;
    if (amount > gActive->high_scores[score]) gActive->high_scores[score] = amount;
}

void bo_profile_discover(BORun *run, BOCenterId center) {
    BOProfile *p = gActive;
    if (!counting(run) || center < 0 || center >= BO_CENTER_COUNT || p->discovered[center]) return;
    p->discovered[center] = 1;
    BOUnlockArgs args = {.type = BO_UNLOCK_DISCOVER_AMOUNT};
    bo_profile_check(run, &args);
}

void bo_profile_discover_blind(BORun *run, BOBlindId blind) {
    BOProfile *p = gActive;
    if (!counting(run) || p->blind_discovered[blind]) return;
    p->blind_discovered[blind] = 1;
    BOUnlockArgs args = {.type = BO_UNLOCK_BLIND_DISCOVERIES};
    bo_profile_check(run, &args);
}

void bo_profile_start_run(BORun *run) {
    BOProfile *p = gActive;
    if (!counting(run) || run->back < BO_B_RED || run->back >= BO_B_RED + BO_DECK_COUNT) return;
    p->decks[run->back - BO_B_RED].count++;
    bo_profile_discover(run, run->back);
}

static void set_challenge_unlock(BOProfile *p);

// set_deck_win: the stake and all the stakes under it count a win.
static void set_deck_win(BOProfile *p, BORun *run) {
    int d = run->back - BO_B_RED;
    if (d < 0 || d >= BO_DECK_COUNT || run->stake < 1 || run->stake > BO_STAKE_COUNT) return;
    p->decks[d].wins[run->stake - 1]++;
    for (int s = 0; s < run->stake - 1; s++) {
        if (p->decks[d].wins[s] == 0) p->decks[d].wins[s] = 1;
    }
    set_challenge_unlock(p);
}

// set_challenge_unlock: 5 challenges open after wins with 5 decks, then one
// more for each completed challenge.
static void set_challenge_unlock(BOProfile *p) {
    if (p->all_unlocked) return;
    if (p->challenges_unlocked) {
        int completed = 0;
        for (int i = 0; i < BO_CHALLENGE_COUNT; i++) completed += p->challenges_completed[i];
        p->challenges_unlocked = completed + 5 < BO_CHALLENGE_COUNT ? completed + 5 : BO_CHALLENGE_COUNT;
        return;
    }
    int decks = 0;
    for (int d = 0; d < BO_DECK_COUNT; d++) decks += p->decks[d].wins[0] > 0;
    if (decks >= 5) p->challenges_unlocked = 5;
}

int bo_profile_challenges_open(const BOProfile *profile) {
    if (!profile || profile->all_unlocked) return BO_CHALLENGE_COUNT;
    return profile->challenges_unlocked;
}

void bo_profile_win(BORun *run) {
    BOProfile *p = gActive;
    if (!p) return;
    if (run->challenge) {
        p->challenges_completed[run->challenge - 1] = 1;
        set_challenge_unlock(p);
        return;
    }
    // win_game: the streaks, the deck, then the checks of state_events.lua.
    bo_profile_high_score(BO_HIGH_WIN_STREAK, p->high_scores[BO_HIGH_CURRENT_STREAK] + 1);
    p->high_scores[BO_HIGH_CURRENT_STREAK] += 1;
    set_deck_win(p, run);
    static const BOUnlockType checks[] = {BO_UNLOCK_WIN_NO_HAND, BO_UNLOCK_WIN_CUSTOM, BO_UNLOCK_WIN_DECK,
                                          BO_UNLOCK_WIN_STAKE, BO_UNLOCK_WIN};
    for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); i++) {
        BOUnlockArgs args = {.type = checks[i]};
        bo_profile_check(run, &args);
    }
    bo_profile_stat(run, BO_STAT_WINS, 1);
}

void bo_profile_lose(BORun *run) {
    BOProfile *p = gActive;
    if (!counting(run)) return;
    p->high_scores[BO_HIGH_CURRENT_STREAK] = 0;
    int d = run->back - BO_B_RED;
    if (d >= 0 && d < BO_DECK_COUNT && run->stake >= 1 && run->stake <= BO_STAKE_COUNT) {
        p->decks[d].losses[run->stake - 1]++;
    }
    bo_profile_stat(run, BO_STAT_LOSSES, 1);
}

// --- The file ---------------------------------------------------------------

typedef struct {
    char magic[4];  // "B4SP": a profile of Balatro4S.
    uint32_t layout;
} BOProfileHeader;

static uint32_t profile_layout(void) {
    return (uint32_t)sizeof(BOProfile) * 2654435761u ^ (uint32_t)BO_CENTER_COUNT * 40503u ^
           (uint32_t)BO_BLIND_COUNT * 97u ^ (uint32_t)BO_STAT_COUNT;
}

int bo_profile_write(const BOProfile *profile, const char *path) {
    char temp[1024];
    if (snprintf(temp, sizeof(temp), "%s.new", path) >= (int)sizeof(temp)) return 0;
    FILE *file = fopen(temp, "wb");
    if (!file) return 0;
    BOProfileHeader header = {{'B', '4', 'S', 'P'}, profile_layout()};
    int ok = fwrite(&header, sizeof(header), 1, file) == 1 && fwrite(profile, sizeof(*profile), 1, file) == 1;
    ok = fclose(file) == 0 && ok;
    if (!ok || rename(temp, path) != 0) {
        remove(temp);
        return 0;
    }
    return 1;
}

int bo_profile_read(BOProfile *profile, const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return 0;
    BOProfileHeader header;
    BOProfile *loaded = malloc(sizeof(BOProfile));
    int ok = loaded && fread(&header, sizeof(header), 1, file) == 1 && memcmp(header.magic, "B4SP", 4) == 0 &&
             header.layout == profile_layout() && fread(loaded, sizeof(BOProfile), 1, file) == 1 &&
             fgetc(file) == EOF;
    fclose(file);
    if (ok) memcpy(profile, loaded, sizeof(BOProfile));
    free(loaded);
    return ok;
}
