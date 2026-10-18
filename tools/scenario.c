// scenario: plays one hand from a scenario file with the core of
// Balatro4S, and prints the result in the format of tools/scenario.lua.
// Usage: scenario FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bo_boss.h"
#include "bo_consume.h"
#include "bo_eval.h"
#include "bo_pool.h"
#include "bo_joker.h"
#include "bo_pack.h"
#include "bo_tag.h"
#include "bo_voucher.h"
#include "bo_rules.h"
#include "bo_run.h"

static int parse_card(BORun *run, const char *token, BOCard *out) {
    char copy[64];
    snprintf(copy, sizeof(copy), "%s", token);
    char *parts[8];
    int count = 0;
    for (char *p = strtok(copy, ":"); p && count < 8; p = strtok(NULL, ":")) parts[count++] = p;
    if (count == 0 || strlen(parts[0]) != 3) return 0;
    static const char kSuits[] = "SHCD";
    static const char kRanks[] = "23456789TJQKA";
    const char *s = strchr(kSuits, parts[0][0]);
    const char *r = strchr(kRanks, parts[0][2]);
    if (!s || !r) return 0;
    BOCard card = bo_rules_new_playing_card(run, (int)(r - kRanks) + 2, (int)(s - kSuits));
    for (int i = 1; i < count; i++) {
        BOCenterId center = bo_center_by_key(parts[i]);
        if (center != BO_CENTER_NONE) card.enhancement = (uint8_t)bo_rules_enhancement_of(center);
        else if (strcmp(parts[i], "foil") == 0) card.edition = BO_EDITION_FOIL;
        else if (strcmp(parts[i], "holo") == 0) card.edition = BO_EDITION_HOLO;
        else if (strcmp(parts[i], "polychrome") == 0) card.edition = BO_EDITION_POLYCHROME;
        else if (strcmp(parts[i], "Gold") == 0) card.seal = BO_SEAL_GOLD;
        else if (strcmp(parts[i], "Red") == 0) card.seal = BO_SEAL_RED;
        else if (strcmp(parts[i], "Blue") == 0) card.seal = BO_SEAL_BLUE;
        else if (strcmp(parts[i], "Purple") == 0) card.seal = BO_SEAL_PURPLE;
    }
    *out = card;
    return 1;
}

static int edition_by_name(const char *name) {
    if (strcmp(name, "foil") == 0) return BO_EDITION_FOIL;
    if (strcmp(name, "holo") == 0) return BO_EDITION_HOLO;
    if (strcmp(name, "polychrome") == 0) return BO_EDITION_POLYCHROME;
    if (strcmp(name, "negative") == 0) return BO_EDITION_NEGATIVE;
    return BO_EDITION_NONE;
}

static void set_ability(BORun *run, BOItem *item, char **words, int count) {
    for (int i = 2; i < count; i++) {
        char *eq = strchr(words[i], '=');
        if (!eq) continue;
        *eq = 0;
        const char *k = words[i], *v = eq + 1;
        if (strcmp(k, "edition") == 0) item->edition = (uint8_t)edition_by_name(v);
        else if (strcmp(k, "mult") == 0) item->ability.mult = atof(v);
        else if (strcmp(k, "x_mult") == 0) item->ability.x_mult = atof(v);
        else if (strcmp(k, "extra") == 0) item->ability.extra = atof(v);
        else if (strcmp(k, "sell") == 0) item->sell_cost = atoi(v);
        else if (strncmp(k, "extra_", 6) == 0) {
            for (int f = 0; f < BO_FIELD_COUNT; f++) {
                if (strcmp(bo_field_name((BOField)f), k) == 0) bo_item_set(item, (BOField)f, atof(v));
            }
        }
    }
    bo_rules_set_cost(run, item);
}

static void print_jokers(BORun *run) {
    for (int i = 0; i < run->joker_count; i++) {
        BOItem *j = &run->jokers[i];
        if (j->getting_sliced) continue;
        printf("joker %d %s mult=%.17g x_mult=%.17g", i + 1, bo_center(j->center)->key, j->ability.mult,
               j->ability.x_mult);
        const BOCenterInfo *c = bo_center(j->center);
        int hasExtraTable = 0;
        for (int f = 0; f < c->config_count; f++) {
            if (strncmp(bo_field_name((BOField)c->config[f].field), "extra_", 6) == 0) hasExtraTable = 1;
        }
        int numberExtra = 0;
        bo_center_number(c, BO_F_EXTRA, &numberExtra);
        if (numberExtra) {
            printf(" extra=%.17g", j->ability.extra);
        } else if (hasExtraTable) {
            // The fields of the extra table that are numbers, in the order of their names.
            const char *names[BO_ABILITY_FIELDS];
            double values[BO_ABILITY_FIELDS];
            int n = 0;
            for (int f = 0; f < j->ability.count; f++) {
                const char *fname = bo_field_name((BOField)j->ability.fields[f]);
                if (strncmp(fname, "extra_", 6) != 0) continue;
                if (bo_center_text(c, (BOField)j->ability.fields[f])) continue;
                names[n] = fname;
                values[n] = j->ability.values[f];
                n++;
            }
            for (int a = 0; a < n; a++) {
                for (int b = a + 1; b < n; b++) {
                    if (strcmp(names[b], names[a]) < 0) {
                        const char *tn = names[a];
                        names[a] = names[b];
                        names[b] = tn;
                        double tv = values[a];
                        values[a] = values[b];
                        values[b] = tv;
                    }
                }
            }
            for (int a = 0; a < n; a++) printf(" %s=%.17g", names[a], values[a]);
        }
        printf("\n");
    }
}

static void print_levels(BORun *run) {
    printf("levels");
    const char *levelNames[BO_HAND_COUNT];
    int levels[BO_HAND_COUNT], n = 0;
    for (int i = 0; i < BO_HAND_COUNT; i++) {
        if (run->hands[i].level != 1) {
            levelNames[n] = bo_hand_name((BOHandType)i, 0);
            levels[n++] = run->hands[i].level;
        }
    }
    for (int a = 0; a < n; a++) {
        for (int b = a + 1; b < n; b++) {
            if (strcmp(levelNames[b], levelNames[a]) < 0) {
                const char *t = levelNames[a];
                levelNames[a] = levelNames[b];
                levelNames[b] = t;
                int tl = levels[a];
                levels[a] = levels[b];
                levels[b] = tl;
            }
        }
    }
    for (int a = 0; a < n; a++) {
        char hand[32];
        snprintf(hand, sizeof(hand), "%s", levelNames[a]);
        for (char *c = hand; *c; c++) if (*c == ' ') *c = '_';
        printf(" %s=%d", hand, levels[a]);
    }
    printf("\n");
}

static const char *edition_name(int edition) {
    static const char *names[] = {"", "foil", "holo", "polychrome", "negative"};
    return names[edition];
}

// A playing card as tools/scenario.lua prints it: S_A:m_glass:holo:Red.
static void print_card(const BOCard *card) {
    static const char kSuits[] = "SHCD";
    static const char kRanks[] = "23456789TJQKA";
    static const char *seals[] = {"", "Gold", "Red", "Blue", "Purple"};
    printf(" %c_%c", kSuits[card->suit], kRanks[card->rank - 2]);
    if (card->enhancement) printf(":%s", bo_center(bo_rules_enhancement_center(card->enhancement))->key);
    if (card->edition) printf(":%s", edition_name(card->edition));
    if (card->seal) printf(":%s", seals[card->seal]);
}

// Uses a consumable on the hand, and prints the state after it.
static void use_card(BORun *run, BOCenterId center, uint64_t highlighted) {
    BOItem item = bo_pool_make_item(run, center);
    bo_run_add_consumable(run, &item);
    for (int i = 0; i < run->joker_count; i++) bo_joker_update(run, &run->jokers[i]);
    int index = run->consumable_count - 1;
    int can = bo_run_can_use_consumable(run, index, highlighted);
    printf("can_use %s\n", can ? "true" : "false");
    if (can) bo_run_use_consumable(run, index, highlighted);
    printf("hand");
    for (int i = 0; i < run->hand_count; i++) print_card(&run->hand[i]);
    printf("\n");
    printf("hand_size %d\n", run->hand_size > 0 ? run->hand_size : 0);
    printf("dollars %d\n", run->dollars);
    printf("joker_slots %d\n", run->joker_slots);
    for (int i = 0; i < run->joker_count; i++) {
        printf("joker_card %d %s%s%s%s\n", i + 1, bo_center(run->jokers[i].center)->key,
               run->jokers[i].edition ? ":" : "", edition_name(run->jokers[i].edition),
               run->jokers[i].eternal ? ":eternal" : "");
    }
    print_jokers(run);
    printf("consumable_slots %d\n", run->consumable_slots);
    for (int i = 0; i < run->consumable_count; i++) {
        printf("consumable %d %s%s%s\n", i + 1, bo_center(run->consumables[i].center)->key,
               run->consumables[i].edition ? ":" : "", edition_name(run->consumables[i].edition));
    }
    print_levels(run);
}

static void print_state(BORun *run) {
    printf("hand");
    for (int i = 0; i < run->hand_count; i++) print_card(&run->hand[i]);
    printf("\n");
    printf("deck");
    for (int i = run->deck_count - 1; i >= 0; i--) print_card(&run->deck[i]);
    printf("\n");
    printf("hand_size %d\n", run->hand_size > 0 ? run->hand_size : 0);
    printf("dollars %d\n", run->dollars);
    printf("joker_slots %d\n", run->joker_slots);
    for (int i = 0; i < run->joker_count; i++) {
        printf("joker_card %d %s%s%s%s\n", i + 1, bo_center(run->jokers[i].center)->key,
               run->jokers[i].edition ? ":" : "", edition_name(run->jokers[i].edition),
               run->jokers[i].eternal ? ":eternal" : "");
    }
    print_jokers(run);
    printf("consumable_slots %d\n", run->consumable_slots);
    for (int i = 0; i < run->consumable_count; i++) {
        printf("consumable %d %s%s%s\n", i + 1, bo_center(run->consumables[i].center)->key,
               run->consumables[i].edition ? ":" : "", edition_name(run->consumables[i].edition));
    }
    print_levels(run);
}

#define MAX_STEPS 8

typedef struct {
    int skip;
    int index;
    uint64_t picks;
} PackStep;

// Opens a pack in the shop, takes the steps, and prints the state.
static void open_pack(BORun *run, BOCenterId center, const PackStep *steps, int count) {
    run->state = BO_STATE_SHOP;
    for (int i = 0; i < run->joker_count; i++) bo_joker_update(run, &run->jokers[i]);
    BOItem booster = bo_pool_make_item(run, center);
    bo_run_forget_card(run, center);
    bo_pack_open(run, center, booster.cost);
    printf("pack");
    for (int i = 0; i < run->pack_count; i++) {
        const BOShopCard *card = &run->pack_cards[i];
        if (card->is_playing_card) {
            print_card(&card->card);
        } else {
            printf(" %s%s%s", bo_center(card->item.center)->key, card->item.edition ? ":" : "",
                   edition_name(card->item.edition));
        }
    }
    printf("\n");
    printf("choices %d\n", run->pack_choices);
    for (int s = 0; s < count && run->state == BO_STATE_PACK; s++) {
        if (steps[s].skip) {
            bo_pack_skip(run);
            break;
        }
        for (int i = 0; i < run->joker_count; i++) bo_joker_update(run, &run->jokers[i]);
        // Card:update runs for the cards of the pack too.
        int index = steps[s].index - 1;
        if (index >= 0 && index < run->pack_count && !run->pack_cards[index].is_playing_card) {
            bo_joker_update(run, &run->pack_cards[index].item);
        }
        int can = bo_pack_can_choose(run, steps[s].index - 1, steps[s].picks);
        printf("can_choose %s\n", can ? "true" : "false");
        if (can) bo_pack_choose(run, steps[s].index - 1, steps[s].picks);
    }
    print_state(run);
}

// Buys a voucher in a new shop, and prints the state of the run.
static void redeem(BORun *run, BOCenterId center) {
    run->state = BO_STATE_SHOP;
    run->shop_count = 0;
    for (int i = 0; i < 2; i++) run->shop[run->shop_count++] = bo_pool_create_shop_card(run);
    run->shop_voucher = bo_pool_make_item(run, center);
    run->has_shop_voucher = 1;
    bo_voucher_redeem(run);
    printf("dollars %d\n", run->dollars);
    printf("ante %d\n", run->ante);
    printf("rates %.17g %.17g %.17g %.17g\n", run->tarot_rate, run->planet_rate, run->edition_rate,
           run->playing_card_rate);
    printf("discount %d\n", run->discount_percent);
    printf("interest_cap %d\n", run->interest_cap);
    printf("hands %d %d\n", run->hands_per_round, run->hands_left);
    printf("discards %d %d\n", run->discards_per_round, run->discards_left);
    printf("reroll %d %d\n", run->reroll_cost_base, run->reroll_cost);
    printf("slots %d %d %d\n", run->joker_slots, run->consumable_slots, run->hand_size > 0 ? run->hand_size : 0);
    printf("shop %d", run->shop_joker_max);
    for (int i = 0; i < run->shop_count; i++) printf(" %s:%d", bo_center(run->shop[i].item.center)->key, run->shop[i].item.cost);
    printf("\n");
    printf("costs");
    for (int i = 0; i < run->joker_count; i++) printf(" %d/%d", run->jokers[i].cost, run->jokers[i].sell_cost);
    printf("\n");
}

// Skips a Small Blind for each tag, then opens a shop, and prints it.
static void skip_for_tags(BORun *run, const BOCenterId *tags, int count) {
    run->state = BO_STATE_BLIND_SELECT;
    for (int t = 0; t < count; t++) {
        run->skips++;
        bo_tag_add(run, tags[t], BO_TAG_FROM_SMALL);
        for (int i = 0; i < run->joker_count; i++) {
            BOContext ctx;
            memset(&ctx, 0, sizeof(ctx));
            ctx.kind = BO_CTX_SKIP_BLIND;
            bo_joker_calculate(run, &run->jokers[i], &ctx);
        }
        bo_tag_immediate(run);
    }
    run->state = BO_STATE_SHOP;
    run->voucher = BO_CENTER_NONE;
    bo_run_fill_shop(run);
    printf("dollars %d\n", run->dollars);
    printf("skips %d\n", run->skips);
    printf("jokers");
    for (int i = 0; i < run->joker_count; i++) {
        printf(" %s%s%s", bo_center(run->jokers[i].center)->key, run->jokers[i].edition ? ":" : "",
               edition_name(run->jokers[i].edition));
    }
    printf("\n");
    printf("tags");
    for (int i = 0; i < run->tag_count; i++) printf(" %s", bo_center(run->tags[i].center)->key);
    printf("\n");
    printf("shop");
    for (int i = 0; i < run->shop_count; i++) {
        printf(" %s%s%s:%d", bo_center(run->shop[i].item.center)->key, run->shop[i].item.edition ? ":" : "",
               edition_name(run->shop[i].item.edition), run->shop[i].item.cost);
    }
    printf("\n");
    printf("vouchers");
    for (int i = 0; i < run->tag_voucher_count; i++) printf(" %s", bo_center(run->tag_vouchers[i].center)->key);
    printf("\n");
    printf("packs");
    for (int i = 0; i < run->shop_pack_count; i++) printf(" %d", run->shop_packs[i].cost);
    printf("\n");
    printf("reroll %d\n", run->reroll_cost);
    print_levels(run);
}

static const char *suit_name(int suit) {
    static const char *names[] = {"Spades", "Hearts", "Clubs", "Diamonds"};
    return names[suit];
}

static char rank_char(int rank) {
    return rank >= 2 && rank <= 14 ? "23456789TJQKA"[rank - 2] : '-';
}

// A "start" line: the state of bo_run_start_deck, as tools/start.lua prints
// the state of start_run.
static int print_start(FILE *file, const char *seed) {
    char line[256], key[64];
    BOCenterId back = BO_B_RED;
    int stake = 1, challenge = -1;
    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "back %63s", key) == 1) back = bo_center_by_key(key);
        sscanf(line, "stake %d", &stake);
        sscanf(line, "challenge %d", &challenge);
    }
    static BORun runStorage;
    BORun *run = &runStorage;
    if (challenge >= 0) {
        bo_run_start_challenge(run, seed, challenge);
    } else {
        bo_run_start_deck(run, seed, back, stake);
    }
    printf("start hands %d discards %d dollars %d hand_size %d joker_slots %d consumable_slots %d\n",
           run->hands_per_round, run->discards_per_round, run->dollars, run->hand_size, run->joker_slots,
           run->consumable_slots);
    printf("boss %s voucher %s tags %s %s\n", bo_blind_info(run->boss)->key, bo_center(run->voucher)->key,
           bo_center(run->blind_tags[0])->key, bo_center(run->blind_tags[1])->key);
    // The used vouchers, in the order of their keys.
    const char *vouchers[64];
    int voucherCount = 0;
    for (int c = 0; c < BO_CENTER_COUNT && voucherCount < 64; c++) {
        if (run->used_vouchers[c]) vouchers[voucherCount++] = bo_center((BOCenterId)c)->key;
    }
    for (int i = 1; i < voucherCount; i++) {
        for (int j = i; j > 0 && strcmp(vouchers[j - 1], vouchers[j]) > 0; j--) {
            const char *t = vouchers[j];
            vouchers[j] = vouchers[j - 1];
            vouchers[j - 1] = t;
        }
    }
    printf("vouchers");
    for (int i = 0; i < voucherCount; i++) printf(" %s", vouchers[i]);
    printf("\nconsumables");
    for (int i = 0; i < run->consumable_count; i++) printf(" %s", bo_center(run->consumables[i].center)->key);
    printf("\nrates tarot %g planet %g spectral %g shop_slots %d\n", run->tarot_rate, run->planet_rate,
           run->spectral_rate, run->shop_joker_max);
    printf("blind_scaling %g interest %s per_hand %d per_discard %d\n", run->ante_scaling,
           run->no_interest ? "no" : "yes", run->money_per_hand, run->money_per_discard);
    printf("blinds %.17g %.17g %.17g\n", bo_run_slot_chips(run, BO_SLOT_SMALL), bo_run_slot_chips(run, BO_SLOT_BIG),
           bo_run_slot_chips(run, BO_SLOT_BOSS));
    // The top of the deck is the end of the list.
    printf("deck %d", run->deck_count);
    for (int i = run->deck_count - 1; i >= 0; i--) print_card(&run->deck[i]);
    printf("\njokers");
    for (int i = 0; i < run->joker_count; i++) {
        const BOItem *j = &run->jokers[i];
        printf(" %s%s%s%s%s", bo_center(j->center)->key, j->edition ? ":" : "", edition_name(j->edition),
               j->eternal ? ":E" : "", j->pinned ? ":P" : "");
    }
    printf("\nrules reward %d%d%d hand_money %s cap %s flip %d minus %d eternal_ante %d slots_ante %d "
           "discard_cost %d joker_rate %g inflation %s debuff %s all_eternal %s\n",
           !(run->no_blind_reward & 1), !(run->no_blind_reward & 2), !(run->no_blind_reward & 4),
           run->no_extra_hand_money ? "no" : "yes", run->chips_dollar_cap ? "yes" : "no", run->flipped_cards,
           run->minus_hand_size_per_x_dollar, run->set_eternal_ante, run->set_joker_slots_ante, run->discard_cost,
           run->joker_rate, run->inflation_rule ? "yes" : "no", run->debuff_played_cards ? "yes" : "no",
           run->all_eternal ? "yes" : "no");
    // The banned keys, in the order of their keys.
    const char *banned[BO_CENTER_COUNT + BO_BLIND_COUNT];
    int bannedCount = 0;
    for (int c = 0; c < BO_CENTER_COUNT; c++) {
        if (run->banned[c]) banned[bannedCount++] = bo_center((BOCenterId)c)->key;
    }
    for (int b = 0; b < BO_BLIND_COUNT; b++) {
        if (run->banned_blinds[b]) banned[bannedCount++] = bo_blind_info((BOBlindId)b)->key;
    }
    for (int i = 1; i < bannedCount; i++) {
        for (int j = i; j > 0 && strcmp(banned[j - 1], banned[j]) > 0; j--) {
            const char *t = banned[j];
            banned[j] = banned[j - 1];
            banned[j - 1] = t;
        }
    }
    printf("banned");
    for (int i = 0; i < bannedCount; i++) printf(" %s", banned[i]);
    printf("\nround idol %c %s mail %c ancient %s castle %s\n", rank_char(run->idol_rank),
           run->idol_rank > 0 ? suit_name(run->idol_suit) : "-", rank_char(run->mail_rank),
           suit_name(run->ancient_suit), suit_name(run->castle_suit));
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: scenario FILE\n");
        return 2;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) return 1;
    static BORun runStorage;
    BORun *run = &runStorage;
    char line[1024];
    char seed[16] = "ABCDEFGH";
    int start = 0, rulesIndex = -1;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "seed ", 5) == 0) sscanf(line + 5, "%15s", seed);
        if (strncmp(line, "start", 5) == 0) start = 1;
        sscanf(line, "rules %d", &rulesIndex);
    }
    if (start) return print_start(file, seed);
    // The state of start_run with no deck, as tools/scenario.lua makes it.
    bo_run_start(run, seed);
    bo_random_start(&run->random, seed);
    run->deck_count = 0;
    run->state = BO_STATE_SELECTING;
    run->blind = BO_BL_SMALL;
    run->blind_chips = 300;
    // A "rules N" line: the custom rules of challenge N, before the blind.
    if (rulesIndex >= 0) bo_run_apply_challenge_rules(run, rulesIndex);
    run->blind_dollars = (run->no_blind_reward & 1) ? 0 : bo_blind_info(BO_BL_SMALL)->dollars;
    run->dollars = 4;
    run->hands_left = 4;
    run->discards_left = 3;
    // The cards of the round as init_game_object sets them: no rank for The
    // Idol and Mail-In Rebate, and Spades.
    run->idol_rank = -999;
    run->idol_suit = BO_SPADES;
    run->mail_rank = -999;
    run->ancient_suit = BO_SPADES;
    run->castle_suit = BO_SPADES;

    BOCenterId useCenter = BO_CENTER_NONE, packCenter = BO_CENTER_NONE, redeemCenter = BO_CENTER_NONE;
    BOCenterId back = BO_CENTER_NONE;
    PackStep steps[MAX_STEPS];
    int blindId = -1;
    BOCenterId tags[8];
    int tagCount = 0;
    uint64_t discardMask = 0;
    int hasDiscard = 0, doEndRound = 0;
    int stepCount = 0;
    uint64_t useMask = 0;
    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        char *words[64];
        int count = 0;
        for (char *p = strtok(line, " \t\r\n"); p && count < 64; p = strtok(NULL, " \t\r\n")) words[count++] = p;
        if (count == 0) continue;
        const char *key = words[0];
        if (strcmp(key, "ante") == 0) run->ante = atoi(words[1]);
        else if (strcmp(key, "dollars") == 0) run->dollars = atoi(words[1]);
        else if (strcmp(key, "hands_left") == 0) run->hands_left = atoi(words[1]);
        else if (strcmp(key, "discards_left") == 0) run->discards_left = atoi(words[1]);
        else if (strcmp(key, "hands_played") == 0) run->hands_played = atoi(words[1]);
        else if (strcmp(key, "discards_used") == 0) run->discards_used = atoi(words[1]);
        else if (strcmp(key, "idol") == 0) {
            run->idol_rank = atoi(words[1]);
            run->idol_suit = bo_rules_suit_by_name(words[2]);
        } else if (strcmp(key, "mail") == 0) run->mail_rank = atoi(words[1]);
        else if (strcmp(key, "ancient") == 0) run->ancient_suit = bo_rules_suit_by_name(words[1]);
        else if (strcmp(key, "castle") == 0) run->castle_suit = bo_rules_suit_by_name(words[1]);
        else if (strcmp(key, "level") == 0 || strcmp(key, "played") == 0) {
            char name[32];
            snprintf(name, sizeof(name), "%s", words[1]);
            for (char *c = name; *c; c++) if (*c == '_') *c = ' ';
            int type = bo_rules_hand_by_name(name);
            if (type < 0) continue;
            if (key[0] == 'l') bo_run_level_up_hand(run, (BOHandType)type, atoi(words[2]) - 1);
            else run->hands[type].played = atoi(words[2]);
        } else if (strcmp(key, "joker") == 0 || strcmp(key, "consumable") == 0) {
            BOCenterId center = bo_center_by_key(words[1]);
            if (center == BO_CENTER_NONE) continue;
            BOItem item = bo_pool_make_item(run, center);
            set_ability(run, &item, words, count);
            if (key[0] == 'j') bo_run_add_joker(run, &item);
            else bo_run_add_consumable(run, &item);
        } else if (strcmp(key, "deck") == 0) {
            int n = atoi(words[1]);
            for (int i = 0; i < n; i++) {
                BOCard card;
                parse_card(run, "C_2", &card);
                run->deck[run->deck_count++] = card;
            }
        } else if (strcmp(key, "play") == 0) {
            for (int i = 1; i < count; i++) parse_card(run, words[i], &run->play[run->play_count++]);
        } else if (strcmp(key, "hand") == 0) {
            for (int i = 1; i < count; i++) parse_card(run, words[i], &run->hand[run->hand_count++]);
        } else if (strcmp(key, "use") == 0) {
            useCenter = bo_center_by_key(words[1]);
            for (int i = 2; i < count; i++) useMask |= (uint64_t)1 << (atoi(words[i]) - 1);
        } else if (strcmp(key, "deck_cards") == 0) {
            // Each card goes under the deck, as G.deck:emplace does.
            for (int i = 1; i < count && run->deck_count < BO_MAX_CARDS; i++) {
                memmove(&run->deck[1], run->deck, sizeof(BOCard) * run->deck_count);
                parse_card(run, words[i], &run->deck[0]);
                run->deck_count++;
            }
        } else if (strcmp(key, "blind") == 0) {
            for (int b = 0; b < BO_BLIND_COUNT; b++) {
                if (strcmp(bo_blind_info((BOBlindId)b)->key, words[1]) == 0) blindId = b;
            }
        } else if (strcmp(key, "most_played") == 0) {
            char name[32];
            snprintf(name, sizeof(name), "%s", words[1]);
            for (char *c = name; *c; c++) if (*c == '_') *c = ' ';
            int type = bo_rules_hand_by_name(name);
            if (type >= 0) run->most_played_hand = (BOHandType)type;
        } else if (strcmp(key, "discard") == 0) {
            discardMask = 0;
            hasDiscard = 1;
            for (int i = 1; i < count; i++) discardMask |= (uint64_t)1 << (atoi(words[i]) - 1);
        } else if (strcmp(key, "end_round") == 0) {
            doEndRound = 1;
        } else if (strcmp(key, "target") == 0) {
            run->blind_chips = atof(words[1]);
        } else if (strcmp(key, "tag") == 0 && tagCount < 8) {
            tags[tagCount++] = bo_center_by_key(words[1]);
        } else if (strcmp(key, "redeem") == 0) {
            redeemCenter = bo_center_by_key(words[1]);
        } else if (strcmp(key, "open") == 0) {
            packCenter = bo_center_by_key(words[1]);
        } else if ((strcmp(key, "choose") == 0 || strcmp(key, "skip") == 0) && stepCount < MAX_STEPS) {
            PackStep *step = &steps[stepCount++];
            step->skip = key[0] == 's';
            step->index = count > 1 ? atoi(words[1]) : 0;
            step->picks = 0;
            for (int i = 2; i < count; i++) step->picks |= (uint64_t)1 << (atoi(words[i]) - 1);
        } else if (strcmp(key, "last") == 0) {
            run->last_tarot_planet = bo_center_by_key(words[1]);
        } else if (strcmp(key, "eternal") == 0) {
            int i = atoi(words[1]) - 1;
            // set_eternal: only a joker that can be eternal.
            if (i >= 0 && i < run->joker_count && bo_item_center(&run->jokers[i])->eternal_compat &&
                !run->jokers[i].perishable) {
                run->jokers[i].eternal = 1;
            }
        } else if (strcmp(key, "back") == 0) {
            back = bo_center_by_key(words[1]);
        }
    }
    fclose(file);
    // Back:apply_to_run after the cards of the scenario.
    if (back != BO_CENTER_NONE) bo_run_apply_back(run, back);
    if (tagCount > 0) {
        skip_for_tags(run, tags, tagCount);
        return 0;
    }
    if (redeemCenter != BO_CENTER_NONE) {
        redeem(run, redeemCenter);
        return 0;
    }
    if (packCenter != BO_CENTER_NONE) {
        open_pack(run, packCenter, steps, stepCount);
        return 0;
    }
    if (useCenter != BO_CENTER_NONE) {
        use_card(run, useCenter, useMask);
        return 0;
    }
    if (blindId >= 0) {
        run->blind = (BOBlindId)blindId;
        run->blind_dollars = bo_blind_info(run->blind)->dollars;
        int typeBit = run->blind == BO_BL_SMALL ? 1 : run->blind == BO_BL_BIG ? 2 : 4;
        if (run->no_blind_reward & typeBit) run->blind_dollars = 0;
        bo_boss_set_blind(run);
        bo_boss_press_play(run);
    }
    if (hasDiscard) {
        // The discard and the draw, as bo_run_discard does them.
        bo_run_discard_cards(run, discardMask, 0);
        bo_run_fill_hand(run);
    }
    run->hands_left--;

    BOScore score;
    BOHandRules rules = bo_eval_rules(run);
    BOHandResult before = bo_hand_evaluate(run->play, run->play_count, &rules);
    bo_eval_play(run, &score);
    char name[32];
    snprintf(name, sizeof(name), "%s", bo_hand_name(before.type, 0));
    for (char *c = name; *c; c++) if (*c == ' ') *c = '_';
    printf("hand %s\n", name);
    printf("chips %.17g\n", score.chips);
    printf("mult %.17g\n", score.mult);
    printf("total %.17g\n", run->chips);
    printf("dollars %d\n", run->dollars);
    print_jokers(run);
    printf("consumables %d\n", run->consumable_count);
    print_levels(run);
    if (blindId >= 0) {
        printf("cards");
        for (int i = 0; i < run->play_count; i++) {
            // A broken or destroyed card left the play.
            if (run->play[i].shattered || run->play[i].destroyed) continue;
            print_card(&run->play[i]);
            if (run->play[i].debuffed) printf(":D");
        }
        printf(" |");
        for (int i = 0; i < run->hand_count; i++) {
            print_card(&run->hand[i]);
            if (run->hand[i].debuffed) printf(":D");
        }
        printf("\n");
        printf("round %d %d %d 0 %d\n", run->hands_left, run->discards_left, run->hand_size > 0 ? run->hand_size : 0,
               run->discard_count);
    }
    if (doEndRound) {
        // A card that broke or was destroyed left G.playing_cards.
        int kept = 0;
        for (int i = 0; i < run->play_count; i++) {
            if (!run->play[i].shattered && !run->play[i].destroyed) run->play[kept++] = run->play[i];
        }
        run->play_count = kept;
        run->state = BO_STATE_SELECTING;
        bo_run_end_round(run);
        static const char *suits[4] = {"Spades", "Hearts", "Clubs", "Diamonds"};
        printf("end %s %d %d %d\n", run->state == BO_STATE_GAME_OVER ? "lost" : "won", run->dollars, run->deck_count,
               run->unused_discards);
        printf("cards %d %s %d %s %s\n", run->idol_rank, suits[run->idol_suit], run->mail_rank, suits[run->ancient_suit],
               suits[run->castle_suit]);
        for (int i = 0; i < run->joker_count; i++) bo_joker_update(run, &run->jokers[i]);
        printf("bonus");
        for (int i = 0; i < run->joker_count; i++) {
            int has = 0;
            int dollars = bo_joker_dollar_bonus(run, &run->jokers[i], &has);
            if (has) printf(" %d", dollars);
            else printf(" -");
        }
        printf("\n");
        print_jokers(run);
        printf("consumables");
        for (int i = 0; i < run->consumable_count; i++) printf(" %s", bo_center(run->consumables[i].center)->key);
        printf("\n");
        // The rows of money of evaluate_round, and the tags after it.
        printf("eval");
        for (int i = 0; i < run->eval.row_count; i++) {
            const BOEvalRow *row = &run->eval.rows[i];
            switch (row->kind) {
            case BO_EVAL_BLIND: printf(" blind1:%d", row->dollars); break;
            case BO_EVAL_HANDS: printf(" hands:%d", row->dollars); break;
            case BO_EVAL_DISCARDS: printf(" discards:%d", row->dollars); break;
            case BO_EVAL_JOKER: printf(" joker%d:%d", row->index + 1, row->dollars); break;
            case BO_EVAL_TAG: printf(" tag%d:%d", row->index + 1, row->dollars); break;
            case BO_EVAL_INTEREST: printf(" interest:%d", row->dollars); break;
            }
        }
        printf("\ntags");
        for (int i = 0; i < run->tag_count; i++) printf(" %s", bo_center(run->tags[i].center)->key);
        printf("\n");
    }
    return 0;
}
