#include "bo_run.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bo_boss.h"
#include "bo_challenge_data.h"
#include "bo_consume.h"
#include "bo_eval.h"
#include "bo_joker.h"
#include "bo_pack.h"
#include "bo_pool.h"
#include "bo_profile.h"
#include "bo_rules.h"
#include "bo_tag.h"
#include "bo_voucher.h"

#pragma STDC FP_CONTRACT OFF

// The suits and the ranks in the order of the keys of P_CARDS as strings,
// for example "C_2" before "C_A" before "C_T". start_run makes the cards in
// this order, so each card gets its sort_id in this order.
static const char kSuitOrder[4] = {BO_CLUBS, BO_DIAMONDS, BO_HEARTS, BO_SPADES};
static const int kRankOrder[13] = {2, 3, 4, 5, 6, 7, 8, 9, BO_ACE, BO_JACK, BO_KING, BO_QUEEN, 10};

uint32_t bo_run_new_sort_id(BORun *run) {
    return ++run->next_sort_id;
}

uint32_t bo_run_new_id(BORun *run) {
    return ++run->next_id;
}

int bo_run_find_joker(const BORun *run, BOCenterId center) {
    int count = 0;
    for (int i = 0; i < run->joker_count; i++) {
        if (run->jokers[i].center == center && !run->jokers[i].debuffed) count++;
    }
    for (int i = 0; i < run->consumable_count; i++) {
        if (run->consumables[i].center == center && !run->consumables[i].debuffed) count++;
    }
    return count;
}

int bo_run_spendable(const BORun *run) {
    return run->dollars - run->bankrupt_at;
}

// CardArea:update of the hand with minus_hand_size_per_X_dollar: the hand
// is one card smaller for each X dollars.
void bo_run_poll_hand_size(BORun *run) {
    if (!run->minus_hand_size_per_x_dollar) return;
    int steps = (int)floor((double)run->dollars / run->minus_hand_size_per_x_dollar);
    if (steps != run->last_poll_size) {
        bo_rules_change_hand_size(run, run->last_poll_size - steps);
        run->last_poll_size = steps;
    }
}

void bo_run_ease_dollars(BORun *run, int amount) {
    if (run->defer_dollars) {
        run->pending_dollars += amount;
    } else {
        run->dollars += amount;
        bo_profile_dollars(run, amount);
        bo_run_poll_hand_size(run);
    }
}

void bo_run_level_up_hand(BORun *run, BOHandType type, int amount) {
    if (type < 0 || type >= BO_HAND_COUNT) return;
    int level = run->hands[type].level + amount;
    run->hands[type].level = level < 0 ? 0 : level;
    run->context.levels[type] = run->hands[type].level;
}

static int compare_uid(const void *a, const void *b) {
    const BOCard *x = a, *y = b;
    return x->uid < y->uid ? -1 : x->uid > y->uid;
}

// CardArea:shuffle: sort by sort_id, then pseudoshuffle with the key.
static void shuffle_deck(BORun *run, const char *key) {
    qsort(run->deck, run->deck_count, sizeof(BOCard), compare_uid);
    int order[BO_MAX_CARDS];
    bo_pseudoshuffle(&run->random, bo_pseudoseed(&run->random, key), order, run->deck_count);
    BOCard shuffled[BO_MAX_CARDS];
    for (int i = 0; i < run->deck_count; i++) shuffled[i] = run->deck[order[i]];
    memcpy(run->deck, shuffled, sizeof(BOCard) * run->deck_count);
}

static int gSortBySuit;

static int compare_nominal(const void *a, const void *b) {
    double x = bo_card_nominal_key(a, gSortBySuit), y = bo_card_nominal_key(b, gSortBySuit);
    return x > y ? -1 : x < y;
}

void bo_run_sort_hand(BORun *run, int by_suit) {
    run->sort_by_suit = by_suit;
    gSortBySuit = by_suit;
    qsort(run->hand, run->hand_count, sizeof(BOCard), compare_nominal);
}

// Draws count cards from the top of the deck. A boss can turn a card face
// down as it comes.
void bo_run_draw_cards(BORun *run, int count) {
    if (count > run->deck_count) count = run->deck_count;
    int drawn = 0;
    for (int i = 0; i < count && run->hand_count < BO_MAX_HAND; i++) {
        BOCard card = run->deck[--run->deck_count];
        card.face_down = (uint8_t)bo_boss_stay_flipped(run, &card);
        // X-ray Vision: 1 in flipped_cards comes face down.
        if (run->flipped_cards && bo_pseudorandom(&run->random, "flipped_card") < 1.0 / run->flipped_cards) {
            card.face_down = 1;
        }
        run->hand[run->hand_count++] = card;
        drawn++;
    }
    // Each draw_card sorts the hand; with no card drawn, nothing sorts it.
    if (drawn > 0) bo_run_sort_hand(run, run->sort_by_suit);
}

// G.FUNCS.draw_from_deck_to_hand: fills the hand. The Serpent draws 3.
void bo_run_fill_hand(BORun *run) {
    int limit = run->hand_size > 0 ? run->hand_size : 0;
    int space = limit - run->hand_count;
    if (space > run->deck_count) space = run->deck_count;
    int serpent = bo_boss_draw_count(run);
    if (serpent >= 0) space = serpent;
    bo_run_draw_cards(run, space);
}

// The playing cards in G.playing_cards order: the order of creation.
static int playing_cards(const BORun *run, const BOCard **out) {
    int count = 0;
    const BOCard *areas[4] = {run->deck, run->hand, run->discard, run->play};
    int counts[4] = {run->deck_count, run->hand_count, run->discard_count, run->play_count};
    for (int a = 0; a < 4; a++) {
        for (int i = 0; i < counts[a]; i++) out[count++] = &areas[a][i];
    }
    for (int i = 1; i < count; i++) {
        const BOCard *t = out[i];
        int j = i - 1;
        while (j >= 0 && out[j]->uid > t->uid) {
            out[j + 1] = out[j];
            j--;
        }
        out[j + 1] = t;
    }
    return count;
}

// reset_idol_card, reset_mail_rank, reset_ancient_card, reset_castle_card.
static void reset_round_cards(BORun *run) {
    const BOCard *cards[BO_MAX_CARDS * 2];
    int count = playing_cards(run, cards);
    const BOCard *valid[BO_MAX_CARDS * 2];
    int validCount = 0;
    for (int i = 0; i < count; i++) {
        if (!bo_card_is_stone(cards[i])) valid[validCount++] = cards[i];
    }
    char key[24];
    run->idol_rank = BO_ACE;
    run->idol_suit = BO_SPADES;
    if (validCount > 0) {
        snprintf(key, sizeof(key), "idol%d", run->ante);
        const BOCard *c = valid[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), validCount)];
        run->idol_rank = c->rank;
        run->idol_suit = c->suit;
    }
    run->mail_rank = BO_ACE;
    if (validCount > 0) {
        snprintf(key, sizeof(key), "mail%d", run->ante);
        run->mail_rank = valid[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), validCount)]->rank;
    }
    static const int suits[4] = {BO_SPADES, BO_HEARTS, BO_CLUBS, BO_DIAMONDS};
    int ancient[4], ancientCount = 0;
    for (int i = 0; i < 4; i++) {
        if (suits[i] != run->ancient_suit) ancient[ancientCount++] = suits[i];
    }
    snprintf(key, sizeof(key), "anc%d", run->ante);
    run->ancient_suit = ancient[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), ancientCount)];
    run->castle_suit = BO_SPADES;
    if (validCount > 0) {
        snprintf(key, sizeof(key), "cas%d", run->ante);
        run->castle_suit = valid[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), validCount)]->suit;
    }
}

// G.GAME.starting_params: the values that start_run gives to the run after
// the deck and the stake change them.
typedef struct {
    int hands, discards, dollars, hand_size, joker_slots, consumable_slots;
    int no_faces, erratic;
} BOStartParams;

// Back:apply_to_run: the starting params and the modifiers of the deck.
// The vouchers of the deck act at once, as Card.apply_to_run.
static void apply_back(BORun *run, BOStartParams *p) {
    BOCenterId vouchers[3];
    int voucherCount = 0;
    switch (run->back) {
    case BO_B_RED: p->discards++; break;
    case BO_B_BLUE: p->hands++; break;
    case BO_B_YELLOW: p->dollars += 10; break;
    case BO_B_GREEN:
        run->money_per_hand = 2;
        run->money_per_discard = 1;
        run->no_interest = 1;
        break;
    case BO_B_BLACK:
        p->hands--;
        p->joker_slots++;
        break;
    case BO_B_MAGIC: vouchers[voucherCount++] = BO_V_CRYSTAL_BALL; break;
    case BO_B_NEBULA:
        vouchers[voucherCount++] = BO_V_TELESCOPE;
        p->consumable_slots--;
        break;
    case BO_B_GHOST: run->spectral_rate = 2; break;
    case BO_B_ABANDONED: p->no_faces = 1; break;
    case BO_B_ZODIAC:
        vouchers[voucherCount++] = BO_V_TAROT_MERCHANT;
        vouchers[voucherCount++] = BO_V_PLANET_MERCHANT;
        vouchers[voucherCount++] = BO_V_OVERSTOCK_NORM;
        break;
    case BO_B_PAINTED:
        p->hand_size += 2;
        p->joker_slots--;
        break;
    case BO_B_PLASMA: run->ante_scaling = 2; break;
    case BO_B_ERRATIC: p->erratic = 1; break;
    default: break;
    }
    for (int i = 0; i < voucherCount; i++) {
        run->used_vouchers[vouchers[i]] = 1;
        bo_voucher_apply(run, vouchers[i]);
    }
}

// The consumables that the Magic Deck and the Ghost Deck give. Balatro makes
// them in an event, with create_card and a forced key.
static void back_consumables(BORun *run) {
    BOCenterId gifts[2];
    int count = 0;
    if (run->back == BO_B_MAGIC) gifts[count++] = BO_C_FOOL, gifts[count++] = BO_C_FOOL;
    if (run->back == BO_B_GHOST) gifts[count++] = BO_C_HEX;
    for (int i = 0; i < count; i++) {
        BOItem item = bo_run_create_item(run, BO_SET_TAROT, 0, 0, 0, gifts[i], "deck", 0);
        bo_run_add_consumable(run, &item);
    }
}

// Checkered Deck: the Clubs become Spades and the Diamonds become Hearts.
static void checkered_suits(BORun *run) {
    BOCard *areas[4] = {run->deck, run->hand, run->play, run->discard};
    int counts[4] = {run->deck_count, run->hand_count, run->play_count, run->discard_count};
    for (int a = 0; a < 4; a++) {
        for (int i = 0; i < counts[a]; i++) {
            if (areas[a][i].suit == BO_CLUBS) areas[a][i].suit = BO_SPADES;
            if (areas[a][i].suit == BO_DIAMONDS) areas[a][i].suit = BO_HEARTS;
        }
    }
}

void bo_run_apply_back(BORun *run, BOCenterId back) {
    BOStartParams p;
    memset(&p, 0, sizeof(p));
    run->back = back;
    apply_back(run, &p);
    back_consumables(run);
    if (back == BO_B_CHECKERED) checkered_suits(run);
}

void bo_run_start(BORun *run, const char *seed) {
    bo_run_start_deck(run, seed, BO_B_RED, 1);
}

// The protos of the cards of start_run, as {s, r, e, d, g}.
typedef struct {
    char s, r;
    const char *e, *d, *g;
} BOProto;

static const char kRankLetters[13] = {'2', '3', '4', '5', '6', '7', '8', '9', 'A', 'J', 'K', 'Q', 'T'};
static const char kSuitLetters[4] = {'C', 'D', 'H', 'S'};

// The sort key of a proto: (s or '')..(r or '')..(e or '')..(d or '')..(g or '').
static void proto_key(const BOProto *p, char *out, int size) {
    snprintf(out, (size_t)size, "%c%c%s%s%s", p->s, p->r, p->e ? p->e : "", p->d ? p->d : "", p->g ? p->g : "");
}

static int rank_of_letter(char r) {
    for (int i = 0; i < 13; i++) {
        if (kRankLetters[i] == r) return kRankOrder[i];
    }
    return 2;
}

static int suit_of_letter(char s) {
    for (int i = 0; i < 4; i++) {
        if (kSuitLetters[i] == s) return kSuitOrder[i];
    }
    return BO_SPADES;
}

// card_from_control: a playing card from a proto.
static BOCard card_from_proto(BORun *run, const BOProto *p) {
    BOCard card = bo_card_make(rank_of_letter(p->r), (BOSuit)suit_of_letter(p->s));
    card.uid = bo_run_new_sort_id(run);
    if (p->e) {
        BOCenterId center = bo_center_by_key(p->e);
        for (int e = 0; e <= BO_ENH_LUCKY; e++) {
            if (bo_rules_enhancement_center(e) == center) card.enhancement = (uint8_t)e;
        }
    }
    if (p->d) {
        static const char *editions[] = {"", "foil", "holo", "polychrome", "negative"};
        for (int e = 1; e < 5; e++) {
            if (strcmp(p->d, editions[e]) == 0) card.edition = (uint8_t)e;
        }
    }
    if (p->g) {
        static const char *seals[] = {"", "Gold", "Red", "Blue", "Purple"};
        for (int g = 1; g < 5; g++) {
            if (strcmp(p->g, seals[g]) == 0) card.seal = (uint8_t)g;
        }
    }
    return card;
}

static int has_letter(const char *letters, char c) {
    return letters && strchr(letters, c) != NULL;
}

// rules.custom of a challenge: the modifiers of start_run.
static void apply_custom_rules(BORun *run, const BOChallengeInfo *ch) {
    for (int i = 0; i < ch->custom_count; i++) {
        const BOChallengeRule *r = &ch->custom[i];
        if (strcmp(r->id, "no_reward") == 0) {
            run->no_blind_reward |= 7;
        } else if (strcmp(r->id, "no_reward_specific") == 0 && r->text) {
            if (strcmp(r->text, "Small") == 0) run->no_blind_reward |= 1;
            if (strcmp(r->text, "Big") == 0) run->no_blind_reward |= 2;
            if (strcmp(r->text, "Boss") == 0) run->no_blind_reward |= 4;
        } else if (strcmp(r->id, "no_extra_hand_money") == 0) {
            run->no_extra_hand_money = 1;
        } else if (strcmp(r->id, "no_interest") == 0) {
            run->no_interest = 1;
        } else if (strcmp(r->id, "chips_dollar_cap") == 0) {
            run->chips_dollar_cap = 1;
        } else if (strcmp(r->id, "all_eternal") == 0) {
            run->all_eternal = 1;
        } else if (strcmp(r->id, "debuff_played_cards") == 0) {
            run->debuff_played_cards = 1;
        } else if (strcmp(r->id, "inflation") == 0) {
            run->inflation_rule = 1;
        } else if (strcmp(r->id, "no_shop_jokers") == 0) {
            run->joker_rate = 0;
        } else if (strcmp(r->id, "flipped_cards") == 0) {
            run->flipped_cards = (int)r->number;
        } else if (strcmp(r->id, "minus_hand_size_per_X_dollar") == 0) {
            run->minus_hand_size_per_x_dollar = (int)r->number;
        } else if (strcmp(r->id, "set_eternal_ante") == 0) {
            run->set_eternal_ante = (int)r->number;
        } else if (strcmp(r->id, "set_joker_slots_ante") == 0) {
            run->set_joker_slots_ante = (int)r->number;
        } else if (strcmp(r->id, "discard_cost") == 0) {
            run->discard_cost = (int)r->number;
        }
    }
}

void bo_run_apply_challenge_rules(BORun *run, int index) {
    const BOChallengeInfo *ch = bo_challenge(index);
    if (ch) apply_custom_rules(run, ch);
}

// The rules of args.challenge in start_run: the vouchers, the modifiers,
// the custom rules, and the banned keys.
static void apply_challenge(BORun *run, const BOChallengeInfo *ch, BOStartParams *p) {
    for (int i = 0; i < ch->voucher_count; i++) {
        BOCenterId v = bo_center_by_key(ch->vouchers[i]);
        if (v != BO_CENTER_NONE) run->used_vouchers[v] = 1;
    }
    for (int i = 0; i < ch->modifier_count; i++) {
        const BOChallengeRule *m = &ch->modifiers[i];
        int value = (int)m->number;
        if (strcmp(m->id, "dollars") == 0) p->dollars = value;
        else if (strcmp(m->id, "discards") == 0) p->discards = value;
        else if (strcmp(m->id, "hands") == 0) p->hands = value;
        else if (strcmp(m->id, "hand_size") == 0) p->hand_size = value;
        else if (strcmp(m->id, "joker_slots") == 0) p->joker_slots = value;
        else if (strcmp(m->id, "consumable_slots") == 0) p->consumable_slots = value;
        else if (strcmp(m->id, "reroll_cost") == 0) run->reroll_cost_base = run->reroll_cost = value;
    }
    apply_custom_rules(run, ch);
    for (int i = 0; i < ch->banned_count; i++) {
        BOCenterId c = bo_center_by_key(ch->banned[i]);
        if (c != BO_CENTER_NONE) {
            run->banned[c] = 1;
            continue;
        }
        for (int b = 0; b < BO_BLIND_COUNT; b++) {
            if (strcmp(bo_blind_info((BOBlindId)b)->key, ch->banned[i]) == 0) run->banned_blinds[b] = 1;
        }
    }
}

// The events of args.challenge after start_run: add_joker for each joker
// and consumable, then Card.apply_to_run for each voucher.
static void challenge_cards(BORun *run, const BOChallengeInfo *ch) {
    for (int i = 0; i < ch->joker_count; i++) {
        const BOChallengeCard *j = &ch->jokers[i];
        BOCenterId center = bo_center_by_key(j->key);
        if (center == BO_CENTER_NONE) continue;
        BOItem item = bo_pool_make_item(run, center);
        if (j->edition) {
            static const char *editions[] = {"", "foil", "holo", "polychrome", "negative"};
            for (int e = 1; e < 5; e++) {
                if (strcmp(j->edition, editions[e]) == 0) item.edition = (uint8_t)e;
            }
        }
        // set_eternal: only a joker that can be eternal.
        if (j->eternal && bo_center(center)->eternal_compat) item.eternal = 1;
        item.pinned = j->pinned;
        if (bo_center(center)->consumeable) {
            bo_run_add_consumable(run, &item);
        } else {
            bo_run_add_joker(run, &item);
        }
    }
    for (int i = 0; i < ch->consumable_count; i++) {
        BOCenterId center = bo_center_by_key(ch->consumables[i].key);
        if (center == BO_CENTER_NONE) continue;
        BOItem item = bo_pool_make_item(run, center);
        bo_run_add_consumable(run, &item);
    }
    for (int i = 0; i < ch->voucher_count; i++) {
        BOCenterId v = bo_center_by_key(ch->vouchers[i]);
        if (v != BO_CENTER_NONE) bo_voucher_apply(run, v);
    }
}

static void start_run(BORun *run, const char *seed, BOCenterId back, int stake, int challenge);

void bo_run_start_deck(BORun *run, const char *seed, BOCenterId back, int stake) {
    start_run(run, seed, back, stake, -1);
}

void bo_run_start_challenge(BORun *run, const char *seed, int index) {
    const BOChallengeInfo *ch = bo_challenge(index);
    BOCenterId back = BO_B_CHALLENGE;
    if (ch && ch->deck_type) {
        for (int i = 0; i < BO_CENTER_COUNT; i++) {
            if (bo_center((BOCenterId)i)->set == BO_SET_BACK && strcmp(bo_center((BOCenterId)i)->name, ch->deck_type) == 0) {
                back = (BOCenterId)i;
            }
        }
    }
    start_run(run, seed, back, 1, ch ? index : -1);
}

static void start_run(BORun *run, const char *seed, BOCenterId back, int stake, int challenge) {
    memset(run, 0, sizeof(*run));
    bo_random_start(&run->random, seed);
    run->back = back;
    run->stake = stake;
    run->ante_scaling = 1;
    run->money_per_hand = 1;
    run->win_ante = 8;
    run->ante = 1;
    run->hand_size = 8;
    run->dollars = 4;
    run->interest_cap = 25;
    run->interest_amount = 1;
    run->probabilities = 1;
    run->edition_rate = 1;
    run->joker_rate = 20;
    run->tarot_rate = 4;
    run->planet_rate = 4;
    run->reroll_cost_base = 5;
    run->reroll_cost = 5;
    run->joker_slots = 5;
    run->consumable_slots = 2;
    run->shop_joker_max = 2;
    run->voucher = BO_CENTER_NONE;
    run->last_tarot_planet = BO_CENTER_NONE;
    run->ecto_minus = 1;
    run->pack = BO_CENTER_NONE;
    run->mouth_hand = BO_HAND_NONE;
    run->temp_reroll_cost = -1;
    run->last_hand_played = BO_HAND_NONE;
    run->most_played_hand = BO_HAND_HIGH_CARD;
    // start_run sets ancient_card.suit to nil, so the first pick has the
    // four suits.
    run->ancient_suit = -1;
    run->used_packs[0] = run->used_packs[1] = BO_CENTER_NONE;
    bo_score_context_init(&run->context);
    for (int i = 0; i < BO_HAND_COUNT; i++) {
        run->hands[i].level = 1;
        // The three secret hands show only after the first play.
        run->hands[i].visible = !(i == BO_HAND_FLUSH_FIVE || i == BO_HAND_FLUSH_HOUSE || i == BO_HAND_FIVE_OF_A_KIND);
    }
    BOStartParams p = {4, 3, 4, 8, 5, 2, 0, 0};
    // The Red Stake and the stakes over it take the money of the Small
    // Blind, and the Blue Stake takes one discard.
    if (stake >= 2) run->no_blind_reward |= 1;
    if (stake >= 5) p.discards--;
    apply_back(run, &p);
    const BOChallengeInfo *ch = challenge >= 0 ? bo_challenge(challenge) : NULL;
    if (ch) {
        run->challenge = challenge + 1;
        apply_challenge(run, ch, &p);
    }
    run->hands_per_round = p.hands;
    run->discards_per_round = p.discards;
    run->dollars = p.dollars;
    run->hand_size = p.hand_size;
    run->joker_slots += p.joker_slots - 5;
    run->consumable_slots += p.consumable_slots - 2;

    // The order of start_run: the boss, the voucher, the two tags, the deck,
    // the first shuffle, then the cards of the round.
    run->boss = bo_blind_new_boss(&run->random, run->ante, run->win_ante, run->bosses_used, run->banned_blinds);
    run->voucher = bo_pool_next_voucher(run, 0);
    run->blind_tags[0] = bo_pool_next_tag(run, NULL);
    run->blind_tags[1] = bo_pool_next_tag(run, NULL);
    // The protos of the cards of start_run: the cards of the challenge, or
    // the 52 keys of P_CARDS with the filters of the challenge. The Erratic
    // Deck takes a random key for each of the 52. Then the protos sort as
    // strings.
    BOProto protos[BO_MAX_CARDS];
    int protoCount = 0;
    if (ch && ch->deck_card_count > 0) {
        for (int i = 0; i < ch->deck_card_count && protoCount < BO_MAX_CARDS; i++) {
            const BOChallengeDeckCard *dc = &ch->deck_cards[i];
            BOProto proto = {dc->suit, dc->rank, dc->enhancement, dc->edition, dc->seal};
            protos[protoCount++] = proto;
        }
    } else {
        for (int i = 0; i < 52; i++) {
            int index = i;
            if (p.erratic) index = bo_random_index(&run->random, bo_pseudoseed(&run->random, "erratic"), 52);
            BOProto proto = {kSuitLetters[index / 13], kRankLetters[index % 13], NULL, NULL, NULL};
            int keep = 1;
            if (ch) {
                if (ch->yes_ranks && !has_letter(ch->yes_ranks, proto.r)) keep = 0;
                if (ch->no_ranks && has_letter(ch->no_ranks, proto.r)) keep = 0;
                if (ch->yes_suits && !has_letter(ch->yes_suits, proto.s)) keep = 0;
                if (ch->no_suits && has_letter(ch->no_suits, proto.s)) keep = 0;
                proto.e = ch->deck_enhancement;
                proto.d = ch->deck_edition;
                proto.g = ch->deck_gold_seal ? "Gold" : NULL;
            }
            if (p.no_faces && (proto.r == 'K' || proto.r == 'Q' || proto.r == 'J')) keep = 0;
            if (keep) protos[protoCount++] = proto;
        }
    }
    char ka[48], kb[48];
    for (int i = 1; i < protoCount; i++) {
        for (int j = i; j > 0; j--) {
            proto_key(&protos[j - 1], ka, sizeof(ka));
            proto_key(&protos[j], kb, sizeof(kb));
            if (strcmp(ka, kb) <= 0) break;
            BOProto t = protos[j];
            protos[j] = protos[j - 1];
            protos[j - 1] = t;
        }
    }
    for (int i = 0; i < protoCount && run->deck_count < BO_MAX_CARDS; i++) {
        run->deck[run->deck_count++] = card_from_proto(run, &protos[i]);
    }
    run->starting_deck_size = run->deck_count;
    run->hands_left = run->hands_per_round;
    run->discards_left = run->discards_per_round;
    shuffle_deck(run, "shuffle");
    reset_round_cards(run);
    // The events of Back:apply_to_run and of the challenge come after
    // start_run.
    back_consumables(run);
    if (run->back == BO_B_CHECKERED) checkered_suits(run);
    if (ch) challenge_cards(run, ch);
    run->slot = BO_SLOT_SMALL;
    run->state = BO_STATE_BLIND_SELECT;
    bo_tag_orbital_choices(run);
    bo_profile_discover(run, run->blind_tags[0]);
    bo_profile_discover(run, run->blind_tags[1]);
    bo_profile_start_run(run);
}

BOBlindId bo_run_slot_blind(const BORun *run, BOBlindSlot slot) {
    if (slot == BO_SLOT_SMALL) return BO_BL_SMALL;
    if (slot == BO_SLOT_BIG) return BO_BL_BIG;
    return run->boss;
}

double bo_run_slot_chips(const BORun *run, BOBlindSlot slot) {
    int scaling = run->stake >= 6 ? 3 : run->stake >= 3 ? 2 : 1;
    return bo_blind_amount(run->ante, scaling) * bo_blind_info(bo_run_slot_blind(run, slot))->mult * run->ante_scaling;
}

static BOContext context_for(BOContextKind kind) {
    BOContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.kind = kind;
    return ctx;
}

// Removes the jokers that an effect marked: eaten, melted, destroyed.
void bo_run_remove_marked_jokers(BORun *run) {
    for (int i = run->joker_count - 1; i >= 0; i--) {
        if (run->jokers[i].getting_sliced) bo_run_remove_joker(run, i);
    }
}

void bo_run_select_blind(BORun *run) {
    if (run->state != BO_STATE_BLIND_SELECT) return;
    // new_round.
    run->discards_left = run->discards_per_round + run->round_bonus_discards;
    if (run->discards_left < 0) run->discards_left = 0;
    run->hands_left = run->hands_per_round + run->round_bonus_hands;
    if (run->hands_left < 1) run->hands_left = 1;
    run->hands_played = 0;
    run->discards_used = 0;
    run->reroll_cost_increase = 0;
    run->used_packs_set = 0;
    run->used_packs[0] = run->used_packs[1] = BO_CENTER_NONE;
    for (int i = 0; i < BO_HAND_COUNT; i++) run->hands[i].played_this_round = 0;
    run->free_rerolls = bo_run_find_joker(run, BO_J_CHAOS);
    bo_rules_calculate_reroll_cost(run, 1);
    run->round_bonus_hands = 0;
    run->round_bonus_discards = 0;

    run->blind = bo_run_slot_blind(run, run->slot);
    const BOBlindInfo *info = bo_blind_info(run->blind);
    run->blind_chips = bo_run_slot_chips(run, run->slot);
    run->blind_dollars = info->dollars;
    for (int i = 0; i < run->hand_count; i++) run->hand[i].face_down = 0;
    for (int i = 0; i < run->deck_count; i++) run->deck[i].face_down = 0;
    // From stake 2 on, the Small Blind gives no reward.
    // modifiers.no_blind_reward: the stakes and the challenges.
    int blindType = run->slot == BO_SLOT_SMALL ? 1 : run->slot == BO_SLOT_BIG ? 2 : 4;
    if (run->no_blind_reward & blindType) run->blind_dollars = 0;
    run->chips = 0;
    run->round++;
    run->state = BO_STATE_SELECTING;
    bo_profile_high_score(BO_HIGH_FURTHEST_ROUND, run->round);
    bo_profile_high_score(BO_HIGH_FURTHEST_ANTE, run->ante);
    if (run->slot != BO_SLOT_BOSS) run->current_boss_streak = 0;
    bo_boss_set_blind(run);
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_SETTING_BLIND);
        ctx.blind_boss = info->boss;
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    bo_run_remove_marked_jokers(run);
    char key[16];
    snprintf(key, sizeof(key), "nr%d", run->ante);
    shuffle_deck(run, key);
    bo_tag_round_start_bonus(run);
    bo_run_fill_hand(run);
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_FIRST_HAND_DRAWN);
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    bo_boss_drawn_to_hand(run);
    bo_profile_check_cards(run);
}

// Moves the cards of the hand that mask selects to out, in hand order.
static int take_from_hand(BORun *run, uint64_t mask, BOCard *out) {
    int taken = 0, kept = 0;
    for (int i = 0; i < run->hand_count; i++) {
        if ((mask >> i) & 1) {
            out[taken++] = run->hand[i];
        } else {
            run->hand[kept++] = run->hand[i];
        }
    }
    run->hand_count = kept;
    return taken;
}

static int count_bits(uint64_t mask, int limit) {
    int count = 0;
    for (int i = 0; i < limit; i++) count += (int)((mask >> i) & 1);
    return count;
}

static void add_eval_row(BORun *run, BOEvalKind kind, int index, int count, int dollars) {
    BORoundEval *eval = &run->eval;
    if (eval->row_count == BO_EVAL_MAX_ROWS) return;
    BOEvalRow *row = &eval->rows[eval->row_count++];
    row->kind = (uint8_t)kind;
    row->index = (uint8_t)index;
    row->count = count;
    row->dollars = dollars;
    eval->total += dollars;
}

void bo_run_end_round(BORun *run) {
    int game_over = run->chips < run->blind_chips;
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_END_OF_ROUND);
        ctx.game_over = game_over;
        BOEffect e = bo_joker_calculate(run, &run->jokers[i], &ctx);
        if (e.any && e.saved) game_over = 0;
        BOItem *joker = &run->jokers[i];
        // calculate_rental and calculate_perishable.
        if (joker->rental) bo_run_ease_dollars(run, -3);
        if (joker->perishable && joker->perish_tally > 0) {
            joker->perish_tally--;
            if (joker->perish_tally == 0) bo_rules_set_debuff(run, joker, 1, 1);
        }
    }
    const BOBlindInfo *blind = bo_blind_info(run->blind);
    if (game_over) {
        bo_run_remove_marked_jokers(run);
        for (int i = 0; i < run->hand_count; i++) run->discard[run->discard_count++] = run->hand[i];
        run->hand_count = 0;
        run->state = BO_STATE_GAME_OVER;
        bo_profile_lose(run);
        return;
    }
    // win_game comes before the money of the round.
    if (run->ante == run->win_ante && blind->boss && !run->won) {
        run->won = 1;
        bo_profile_win(run);
    }
    run->unused_discards += run->discards_left;
    if (blind->boss) {
        int best = BO_HAND_HIGH_CARD, played = -1, order = 100;
        for (int i = 0; i < BO_HAND_COUNT; i++) {
            int type = bo_rules_hands_pairs_order(i);
            if (run->hands[type].played > played || (run->hands[type].played == played && order > type)) {
                played = run->hands[type].played;
                best = type;
                order = type;
            }
        }
        run->most_played_hand = (BOHandType)best;
    }
    // The cards in the hand at the end of the round: Gold Cards pay, and
    // Blue Seals make planets.
    for (int i = 0; i < run->hand_count; i++) {
        BOCard *card = &run->hand[i];
        int reps = 1;
        for (int j = 0; j < reps; j++) {
            int dollars = 0, any = 0;
            if (!card->debuffed) {
                const BOCenterInfo *c = bo_center(bo_rules_enhancement_center(card->enhancement));
                if (c->h_dollars > 0) {
                    dollars = (int)c->h_dollars;
                    any = 1;
                }
                if (card->seal == BO_SEAL_BLUE && run->consumable_count + run->consumeable_buffer < run->consumable_slots) {
                    run->consumeable_buffer++;
                    if (run->last_hand_played != BO_HAND_NONE) {
                        BOCenterId planet = BO_CENTER_NONE;
                        for (int k = 0; k < BO_CENTER_COUNT; k++) {
                            const BOCenterInfo *p = bo_center((BOCenterId)k);
                            if (p->set == BO_SET_PLANET &&
                                bo_rules_hand_by_name(bo_center_text(p, BO_F_HAND_TYPE)) == (int)run->last_hand_played) {
                                planet = (BOCenterId)k;
                            }
                        }
                        BOItem item = bo_run_create_item(run, BO_SET_PLANET, 0, 0, 0, planet, "blusl", 0);
                        bo_run_add_consumable(run, &item);
                        run->consumeable_buffer = 0;
                    }
                    any = 1;
                }
            }
            int effects = 0;
            for (int k = 0; k < run->joker_count; k++) {
                BOContext ctx = context_for(BO_CTX_END_OF_ROUND);
                ctx.individual = 1;
                ctx.cardarea = BO_IN_HAND;
                ctx.other_card = card;
                BOEffect e = bo_joker_calculate(run, &run->jokers[k], &ctx);
                if (e.any) effects++;
            }
            if (j == 0) {
                if (card->seal == BO_SEAL_RED && !card->debuffed && (any || effects > 0)) reps++;
                for (int k = 0; k < run->joker_count; k++) {
                    BOContext ctx = context_for(BO_CTX_END_OF_ROUND);
                    ctx.repetition = 1;
                    ctx.cardarea = BO_IN_HAND;
                    ctx.other_card = card;
                    ctx.card_effects_any = any || effects > 0;
                    BOEffect e = bo_joker_calculate(run, &run->jokers[k], &ctx);
                    if (e.any && e.repetitions > 0) reps += e.repetitions;
                }
            }
            if (dollars) bo_run_ease_dollars(run, dollars);
        }
    }
    bo_run_remove_marked_jokers(run);

    // The hand and the discard pile go back to the deck.
    for (int i = 0; i < run->hand_count; i++) run->discard[run->discard_count++] = run->hand[i];
    run->hand_count = 0;
    if (blind->boss) {
        // Typecast: at this ante the jokers become eternal and the joker
        // slots become 0.
        if (run->set_eternal_ante && run->ante == run->set_eternal_ante) {
            for (int i = 0; i < run->joker_count; i++) {
                BOItem *j = &run->jokers[i];
                if (bo_item_center(j)->eternal_compat && !j->perishable) j->eternal = 1;
            }
        }
        if (run->set_joker_slots_ante && run->ante == run->set_joker_slots_ante) run->joker_slots = 0;
        run->ante++;
        bo_profile_high_score(BO_HIGH_FURTHEST_ANTE, run->ante);
        BOUnlockArgs ante = {.type = BO_UNLOCK_ANTE_UP, .ante = run->ante};
        bo_profile_check(run, &ante);
    }
    for (int i = 0; i < run->discard_count; i++) run->deck[run->deck_count++] = run->discard[i];
    run->discard_count = 0;
    if (blind->boss) {
        run->voucher = bo_pool_next_voucher(run, 0);
        for (int i = 0; i < run->deck_count; i++) run->deck[i].played_this_ante = 0;
    }
    if (run->temp_handsize) {
        bo_rules_change_hand_size(run, -run->temp_handsize);
        run->temp_handsize = 0;
    }
    if (run->temp_reroll_cost >= 0) {
        run->temp_reroll_cost = -1;
        bo_rules_calculate_reroll_cost(run, 1);
    }
    reset_round_cards(run);

    // evaluate_round: the money of the round.
    memset(&run->eval, 0, sizeof(run->eval));
    // Mr. Bones saved the run: the blind gives no money.
    add_eval_row(run, BO_EVAL_BLIND, 0, 0, run->chips >= run->blind_chips ? run->blind_dollars : 0);
    if (run->hands_left > 0 && !run->no_extra_hand_money) {
        add_eval_row(run, BO_EVAL_HANDS, 0, run->hands_left, run->hands_left * run->money_per_hand);
    }
    if (run->discards_left > 0 && run->money_per_discard) {
        add_eval_row(run, BO_EVAL_DISCARDS, 0, run->discards_left, run->discards_left * run->money_per_discard);
    }
    for (int i = 0; i < run->joker_count; i++) {
        int has = 0;
        int dollars = bo_joker_dollar_bonus(run, &run->jokers[i], &has);
        if (has) add_eval_row(run, BO_EVAL_JOKER, i, 0, dollars);
    }
    int tagDollars[BO_MAX_TAGS], tagPlaces[BO_MAX_TAGS];
    int tagRows = bo_tag_eval(run, tagDollars, tagPlaces, BO_MAX_TAGS);
    for (int i = 0; i < tagRows; i++) add_eval_row(run, BO_EVAL_TAG, tagPlaces[i], 0, tagDollars[i]);
    int interestSteps = -1;
    if (run->dollars >= 5 && !run->no_interest) {
        int steps = run->dollars / 5;
        if (steps > run->interest_cap / 5) steps = run->interest_cap / 5;
        add_eval_row(run, BO_EVAL_INTEREST, 0, steps, run->interest_amount * steps);
        interestSteps = steps;
    }
    bo_profile_round_won(run, interestSteps);
    // evaluate_round defeats the blind in an event, after the money of the
    // held cards and of the jokers.
    bo_boss_defeat(run);
    // Anaglyph Deck: a Double Tag after a boss, in an event after the defeat.
    if (run->back == BO_B_ANAGLYPH && blind->boss) bo_tag_add(run, BO_TAG_DOUBLE, BO_TAG_FROM_NONE);
    run->state = BO_STATE_ROUND_EVAL;
}

int bo_run_play(BORun *run, uint64_t mask, BOScore *score) {
    int count = count_bits(mask, run->hand_count);
    if (run->state != BO_STATE_SELECTING || run->hands_left <= 0 || count < 1 || count > BO_MAX_PLAY) return 0;
    run->blind_triggered = 0;
    run->play_count = take_from_hand(run, mask, run->play);
    run->hands_left--;
    for (int i = 0; i < run->play_count; i++) {
        run->play[i].played_this_ante = 1;
        run->play[i].face_down = 0;
        run->play[i].forced_selection = 0;
    }
    for (int i = 0; i < run->hand_count; i++) run->hand[i].forced_selection = 0;
    for (int i = 0; i < run->play_count; i++) run->play[i].times_played++;
    bo_boss_press_play(run);
    bo_eval_play(run, score);
    bo_profile_play(run, run->play, run->play_count, score->type, score->total);
    // Double or Nothing: the scoring cards stay debuffed.
    if (run->debuff_played_cards) {
        for (int i = 0; i < run->play_count; i++) {
            if ((score->scoring >> i) & 1) run->play[i].perma_debuff = run->play[i].debuffed = 1;
        }
    }
    // draw_from_play_to_discard: the destroyed cards leave the game.
    for (int i = 0; i < run->play_count; i++) {
        BOCard *card = &run->play[i];
        int gone = card->destroyed || card->shattered;
        card->vampired = 0;
        card->lucky_trigger = 0;
        if (!gone) run->discard[run->discard_count++] = *card;
    }
    run->play_count = 0;
    bo_run_remove_marked_jokers(run);
    run->hands_played_total++;
    run->hands_played++;
    if (run->chips >= run->blind_chips || run->hands_left < 1) {
        bo_run_end_round(run);
    } else {
        bo_run_fill_hand(run);
        bo_boss_drawn_to_hand(run);
    }
    return 1;
}

void bo_run_discard_cards(BORun *run, uint64_t mask, int hook) {
    int count = count_bits(mask, run->hand_count);
    // discard_cards_from_highlighted does nothing with no card.
    if (count == 0) return;
    BOCard cards[BO_MAX_HAND];
    for (int i = 0; i < run->hand_count + run->deck_count + run->discard_count; i++) {
        if (i < run->hand_count) run->hand[i].forced_selection = 0;
    }
    take_from_hand(run, mask, cards);
    if (!hook) bo_profile_discard(run, cards, count);
    BOHandRules rules = bo_eval_rules(run);
    BOHandResult result = bo_hand_evaluate(cards, count, &rules);
    for (int j = 0; j < run->joker_count; j++) {
        BOContext ctx = context_for(BO_CTX_PRE_DISCARD);
        ctx.full_hand = cards;
        ctx.full_count = count;
        ctx.scoring_name = result.type;
        ctx.hook = hook;
        bo_joker_calculate(run, &run->jokers[j], &ctx);
    }
    int destroyedGlass = 0, destroyedFaces = 0, destroyed = 0;
    for (int i = 0; i < count; i++) {
        BOCard *card = &cards[i];
        // calculate_seal with discard: the Purple Seal makes a Tarot card.
        if (!card->debuffed && card->seal == BO_SEAL_PURPLE &&
            run->consumable_count + run->consumeable_buffer < run->consumable_slots) {
            run->consumeable_buffer++;
            BOItem item = bo_run_create_item(run, BO_SET_TAROT, 0, 0, 0, BO_CENTER_NONE, "8ba", 0);
            bo_run_add_consumable(run, &item);
            run->consumeable_buffer = 0;
        }
        int removed = 0;
        for (int j = 0; j < run->joker_count; j++) {
            BOContext ctx = context_for(BO_CTX_DISCARD);
            ctx.other_card = card;
            ctx.full_hand = cards;
            ctx.full_count = count;
            BOEffect e = bo_joker_calculate(run, &run->jokers[j], &ctx);
            if (e.any && e.remove) removed = 1;
        }
        if (removed) {
            destroyed++;
            if (card->enhancement == BO_ENH_GLASS) {
                card->shattered = 1;
                destroyedGlass++;
            }
            if (bo_rules_is_face(run, card, 0)) destroyedFaces++;
            card->destroyed = 1;
        } else {
            card->face_down = 0;
            run->discard[run->discard_count++] = *card;
        }
    }
    if (destroyed) {
        for (int j = 0; j < run->joker_count; j++) {
            BOContext ctx = context_for(BO_CTX_REMOVE_PLAYING_CARDS);
            ctx.cardarea = BO_IN_JOKERS;
            ctx.glass_shattered = destroyedGlass;
            ctx.faces_removed = destroyedFaces;
            bo_joker_calculate(run, &run->jokers[j], &ctx);
        }
    }
    bo_run_remove_marked_jokers(run);
    if (!hook) {
        if (run->discard_cost) bo_run_ease_dollars(run, -run->discard_cost);
        bo_rules_ease_discard(run, -1);
        run->discards_used++;
    }
}

int bo_run_discard(BORun *run, uint64_t mask) {
    int count = count_bits(mask, run->hand_count);
    if (run->state != BO_STATE_SELECTING || run->discards_left <= 0 || count < 1 || count > BO_MAX_PLAY) return 0;
    bo_run_discard_cards(run, mask, 0);
    bo_run_fill_hand(run);
    bo_boss_drawn_to_hand(run);
    return 1;
}

void bo_run_fill_shop(BORun *run);

void bo_run_cash_out(BORun *run) {
    if (run->state != BO_STATE_ROUND_EVAL) return;
    char key[24];
    snprintf(key, sizeof(key), "cashout%d", run->ante);
    shuffle_deck(run, key);
    run->jokers_purchased = 0;
    run->discards_left = run->discards_per_round + run->round_bonus_discards;
    if (run->discards_left < 0) run->discards_left = 0;
    run->hands_left = run->hands_per_round + run->round_bonus_hands;
    if (run->hands_left < 1) run->hands_left = 1;
    run->dollars += run->eval.total;
    bo_profile_dollars(run, run->eval.total);
    bo_run_poll_hand_size(run);
    bo_profile_stat(run, BO_STAT_ROUNDS, 1);
    run->chips = 0;
    run->shop_free = 0;
    run->shop_d6ed = 0;
    if (run->slot == BO_SLOT_BOSS) {
        run->blind_tags[0] = bo_pool_next_tag(run, NULL);
        run->blind_tags[1] = bo_pool_next_tag(run, NULL);
        bo_profile_discover(run, run->blind_tags[0]);
        bo_profile_discover(run, run->blind_tags[1]);
        run->slot = BO_SLOT_SMALL;
        run->boss = bo_blind_new_boss(&run->random, run->ante, run->win_ante, run->bosses_used, run->banned_blinds);
        run->boss_rerolled = 0;
        run->skipped_slots = 0;
    } else {
        run->slot++;
    }
    run->state = BO_STATE_SHOP;
    bo_run_fill_shop(run);
}

// --- The shop -------------------------------------------------------------

void bo_run_forget_card(BORun *run, BOCenterId center) {
    // Card:remove: the card is not used any more, when no card with its
    // name is in the jokers or the consumables.
    if (center == BO_CENTER_NONE) return;
    for (int i = 0; i < run->joker_count; i++) {
        if (run->jokers[i].center == center) return;
    }
    for (int i = 0; i < run->consumable_count; i++) {
        if (run->consumables[i].center == center) return;
    }
    run->used_jokers[center] = 0;
}

void bo_run_fill_shop(BORun *run) {
    bo_tag_shop_start(run);
    run->shop_count = 0;
    run->tag_voucher_count = 0;
    for (int i = 0; i < run->shop_joker_max && run->shop_count < BO_MAX_SHOP; i++) {
        run->shop[run->shop_count++] = bo_pool_create_shop_card(run);
    }
    run->has_shop_voucher = 0;
    if (run->voucher != BO_CENTER_NONE) {
        run->shop_voucher = bo_pool_make_item(run, run->voucher);
        run->has_shop_voucher = 1;
    }
    run->shop_pack_count = 0;
    for (int i = 0; i < 2; i++) {
        if (!run->used_packs_set) run->used_packs[i] = bo_pool_get_pack(run, "shop_pack", NULL);
        if (run->used_packs[i] != BO_CENTER_NONE) {
            run->shop_packs[run->shop_pack_count++] = bo_pool_make_item(run, run->used_packs[i]);
        }
    }
    run->used_packs_set = 1;
    bo_tag_voucher_add(run);
    bo_tag_shop_final_pass(run);
}

int bo_run_reroll(BORun *run) {
    if (run->state != BO_STATE_SHOP) return 0;
    if (run->reroll_cost > bo_run_spendable(run) && run->reroll_cost > 0) return 0;
    if (run->reroll_cost > 0) bo_run_ease_dollars(run, -run->reroll_cost);
    bo_profile_stat(run, BO_STAT_SHOP_DOLLARS_SPENT, run->reroll_cost);
    bo_profile_stat(run, BO_STAT_SHOP_REROLLS, 1);
    int finalFree = run->free_rerolls > 0;
    run->free_rerolls = run->free_rerolls - 1 < 0 ? 0 : run->free_rerolls - 1;
    bo_rules_calculate_reroll_cost(run, finalFree);
    int old = run->shop_count;
    BOCenterId gone[BO_MAX_SHOP];
    for (int i = 0; i < old; i++) gone[i] = run->shop[i].item.center;
    run->shop_count = 0;
    for (int i = old - 1; i >= 0; i--) bo_run_forget_card(run, gone[i]);
    for (int i = 0; i < run->shop_joker_max && run->shop_count < BO_MAX_SHOP; i++) {
        run->shop[run->shop_count++] = bo_pool_create_shop_card(run);
    }
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_REROLL_SHOP);
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    return 1;
}

int bo_run_buy(BORun *run, int index) {
    if (run->state != BO_STATE_SHOP || index < 0 || index >= run->shop_count) return 0;
    BOShopCard *shop = &run->shop[index];
    BOItem item = shop->item;
    if (item.cost > bo_run_spendable(run) && item.cost > 0) return 0;
    const BOCenterInfo *c = bo_item_center(&item);
    if (!shop->is_playing_card) {
        int negative = item.edition == BO_EDITION_NEGATIVE;
        if (c->set == BO_SET_JOKER && !(run->joker_count < run->joker_slots + negative)) return 0;
        if (c->consumeable && !(run->consumable_count < run->consumable_slots + negative)) return 0;
    }
    BOShopCard bought = *shop;
    memmove(&run->shop[index], &run->shop[index + 1], sizeof(BOShopCard) * (run->shop_count - index - 1));
    run->shop_count--;
    if (bought.is_playing_card) {
        if (run->deck_count < BO_MAX_CARDS) run->deck[run->deck_count++] = bought.card;
        bo_rules_playing_cards_added(run, 1);
    } else if (c->consumeable) {
        bo_run_add_consumable(run, &bought.item);
    } else {
        bo_run_add_joker(run, &bought.item);
        run->jokers_purchased++;
    }
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_BUYING_CARD);
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    if (item.cost != 0) bo_run_ease_dollars(run, -item.cost);
    if (run->inflation_rule) {
        run->inflation++;
        bo_run_reprice(run);
    }
    bo_profile_stat(run, BO_STAT_SHOP_DOLLARS_SPENT, item.cost);
    if (bought.is_playing_card) bo_profile_stat(run, BO_STAT_PLAYING_CARDS_BOUGHT, 1);
    else if (c->set == BO_SET_PLANET) bo_profile_stat(run, BO_STAT_PLANETS_BOUGHT, 1);
    else if (c->set == BO_SET_TAROT) bo_profile_stat(run, BO_STAT_TAROTS_BOUGHT, 1);
    bo_profile_check_cards(run);
    return 1;
}

static int sell_item(BORun *run, BOItem *items, int *count, int index) {
    if (index < 0 || index >= *count) return 0;
    BOItem *item = &items[index];
    if (item->eternal) return 0;
    BOContext self = context_for(BO_CTX_SELLING_SELF);
    bo_joker_calculate(run, item, &self);
    int sell = item->sell_cost;
    int isJoker = items == run->jokers;
    BOItem sold = *item;
    if (isJoker) {
        bo_run_remove_joker(run, index);
    } else {
        bo_joker_remove_from_deck(run, item);
        memmove(&items[index], &items[index + 1], sizeof(BOItem) * (*count - index - 1));
        (*count)--;
        bo_run_forget_card(run, sold.center);
    }
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_SELLING_CARD);
        ctx.card = &sold;
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    bo_run_ease_dollars(run, sell);
    bo_profile_stat(run, BO_STAT_CARDS_SOLD, 1);
    if (isJoker && bo_center(sold.center)->set == BO_SET_JOKER) bo_profile_stat(run, BO_STAT_JOKERS_SOLD, 1);
    bo_run_remove_marked_jokers(run);
    // Verdant Leaf ends when a joker is sold.
    if (isJoker && run->blind_active && run->blind == BO_BL_FINAL_LEAF) bo_boss_disable(run);
    // A boss that is disabled can leave a round that is already won.
    if (run->state == BO_STATE_SELECTING && run->blind_disabled && bo_blind_info(run->blind)->boss &&
        run->chips >= run->blind_chips) {
        bo_run_end_round(run);
    }
    return 1;
}

int bo_run_sell_joker(BORun *run, int index) {
    return sell_item(run, run->jokers, &run->joker_count, index);
}

int bo_run_sell_consumable(BORun *run, int index) {
    return sell_item(run, run->consumables, &run->consumable_count, index);
}

int bo_run_open_pack(BORun *run, int index) {
    if (run->state != BO_STATE_SHOP || index < 0 || index >= run->shop_pack_count) return 0;
    BOItem pack = run->shop_packs[index];
    if (pack.cost > bo_run_spendable(run) && pack.cost > 0) return 0;
    // The pack leaves its slot. Its place in used_packs becomes USED.
    for (int i = 0, seen = 0; i < 2; i++) {
        if (run->used_packs[i] == BO_CENTER_NONE) continue;
        if (seen++ == index) {
            run->used_packs[i] = BO_CENTER_NONE;
            break;
        }
    }
    memmove(&run->shop_packs[index], &run->shop_packs[index + 1],
            sizeof(BOItem) * (run->shop_pack_count - index - 1));
    run->shop_pack_count--;
    bo_run_forget_card(run, pack.center);
    bo_profile_discover(run, pack.center);
    int opened = bo_pack_open(run, pack.center, pack.cost);
    if (opened && run->inflation_rule) {
        run->inflation++;
        bo_run_reprice(run);
    }
    return opened;
}

int bo_run_can_use_consumable(const BORun *run, int index, uint64_t highlighted) {
    if (index < 0 || index >= run->consumable_count) return 0;
    const BOItem *item = &run->consumables[index];
    if (!bo_consume_can_use(run, item, highlighted, BO_USE_FROM_CONSUMABLES)) return 0;
    // Card:check_use: Ankh needs a free joker slot.
    return !(item->center == BO_C_ANKH && run->joker_count >= run->joker_slots);
}

int bo_run_use_consumable(BORun *run, int index, uint64_t highlighted) {
    if (!bo_run_can_use_consumable(run, index, highlighted)) return 0;
    BOItem item = run->consumables[index];
    // The card leaves its area, then it acts, then it dissolves.
    memmove(&run->consumables[index], &run->consumables[index + 1],
            sizeof(BOItem) * (run->consumable_count - index - 1));
    run->consumable_count--;
    bo_consume_use(run, &item, highlighted);
    bo_joker_remove_from_deck(run, &item);
    bo_run_forget_card(run, item.center);
    bo_run_remove_marked_jokers(run);
    bo_profile_discover(run, item.center);
    bo_profile_check_cards(run);
    return 1;
}

void bo_run_end_shop(BORun *run) {
    if (run->state != BO_STATE_SHOP) return;
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_ENDING_SHOP);
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    for (int i = 0; i < run->shop_count; i++) bo_run_forget_card(run, run->shop[i].item.center);
    if (run->has_shop_voucher) bo_run_forget_card(run, run->shop_voucher.center);
    for (int i = 0; i < run->shop_pack_count; i++) bo_run_forget_card(run, run->shop_packs[i].center);
    for (int i = 0; i < run->tag_voucher_count; i++) bo_run_forget_card(run, run->tag_vouchers[i].center);
    run->shop_count = 0;
    run->has_shop_voucher = 0;
    run->tag_voucher_count = 0;
    run->shop_pack_count = 0;
    run->state = BO_STATE_BLIND_SELECT;
    bo_tag_orbital_choices(run);
}

void bo_run_move_joker(BORun *run, int from, int to) {
    if (from < 0 || from >= run->joker_count || to < 0 || to >= run->joker_count || from == to) return;
    BOItem moved = run->jokers[from];
    if (from < to) {
        memmove(&run->jokers[from], &run->jokers[from + 1], sizeof(BOItem) * (to - from));
    } else {
        memmove(&run->jokers[to + 1], &run->jokers[to], sizeof(BOItem) * (from - to));
    }
    run->jokers[to] = moved;
}

void bo_run_reprice(BORun *run) {
    for (int i = 0; i < run->joker_count; i++) bo_rules_set_cost(run, &run->jokers[i]);
    for (int i = 0; i < run->consumable_count; i++) bo_rules_set_cost(run, &run->consumables[i]);
    for (int i = 0; i < run->shop_count; i++) {
        if (!run->shop[i].is_playing_card) bo_rules_set_cost(run, &run->shop[i].item);
    }
    for (int i = 0; i < run->shop_pack_count; i++) bo_rules_set_cost(run, &run->shop_packs[i]);
    if (run->has_shop_voucher) bo_rules_set_cost(run, &run->shop_voucher);
    for (int i = 0; i < run->tag_voucher_count; i++) bo_rules_set_cost(run, &run->tag_vouchers[i]);
}

void bo_run_add_joker(BORun *run, const BOItem *item) {
    if (run->joker_count >= BO_MAX_JOKERS) return;
    BOItem *slot = &run->jokers[run->joker_count++];
    *slot = *item;
    slot->couponed = 0;
    bo_joker_add_to_deck(run, slot);
    run->used_jokers[slot->center] = 1;
    bo_rules_set_cost(run, slot);
    bo_profile_check_cards(run);
}

void bo_run_add_consumable(BORun *run, const BOItem *item) {
    if (run->consumable_count >= BO_MAX_CONSUMABLES) return;
    BOItem *slot = &run->consumables[run->consumable_count++];
    *slot = *item;
    slot->couponed = 0;
    bo_joker_add_to_deck(run, slot);
    run->used_jokers[slot->center] = 1;
    bo_rules_set_cost(run, slot);
    bo_profile_discover(run, slot->center);
}

void bo_run_remove_joker(BORun *run, int index) {
    if (index < 0 || index >= run->joker_count) return;
    BOItem removed = run->jokers[index];
    // A debuffed joker already left the deck.
    if (!removed.debuffed) bo_joker_remove_from_deck(run, &run->jokers[index]);
    memmove(&run->jokers[index], &run->jokers[index + 1], sizeof(BOItem) * (run->joker_count - index - 1));
    run->joker_count--;
    bo_run_forget_card(run, removed.center);
}
