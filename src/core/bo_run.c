#include "bo_run.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bo_consume.h"
#include "bo_eval.h"
#include "bo_joker.h"
#include "bo_pack.h"
#include "bo_pool.h"
#include "bo_rules.h"

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

void bo_run_ease_dollars(BORun *run, int amount) {
    if (run->defer_dollars) {
        run->pending_dollars += amount;
    } else {
        run->dollars += amount;
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

static void draw_to_hand(BORun *run) {
    int limit = run->hand_size > 0 ? run->hand_size : 0;
    int space = limit - run->hand_count;
    if (space > run->deck_count) space = run->deck_count;
    for (int i = 0; i < space && run->hand_count < BO_MAX_HAND; i++) {
        run->hand[run->hand_count++] = run->deck[--run->deck_count];
    }
    bo_run_sort_hand(run, run->sort_by_suit);
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
    int ancient[3], ancientCount = 0;
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

void bo_run_start(BORun *run, const char *seed) {
    memset(run, 0, sizeof(*run));
    bo_random_start(&run->random, seed);
    run->stake = 1;
    run->win_ante = 8;
    run->ante = 1;
    run->hand_size = 8;
    run->hands_per_round = 4;
    // The Red Deck gives one more discard than the 3 of the start.
    run->discards_per_round = 3 + 1;
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
    run->starting_deck_size = 52;
    run->voucher = BO_CENTER_NONE;
    run->last_tarot_planet = BO_CENTER_NONE;
    run->ecto_minus = 1;
    run->pack = BO_CENTER_NONE;
    run->last_hand_played = BO_HAND_NONE;
    run->most_played_hand = BO_HAND_HIGH_CARD;
    run->ancient_suit = BO_SPADES;
    run->used_packs[0] = run->used_packs[1] = BO_CENTER_NONE;
    bo_score_context_init(&run->context);
    for (int i = 0; i < BO_HAND_COUNT; i++) {
        run->hands[i].level = 1;
        // The three secret hands show only after the first play.
        run->hands[i].visible = !(i == BO_HAND_FLUSH_FIVE || i == BO_HAND_FLUSH_HOUSE || i == BO_HAND_FIVE_OF_A_KIND);
    }

    // The order of start_run: the boss, the voucher, the two tags, the deck,
    // the first shuffle, then the cards of the round.
    run->boss = bo_blind_new_boss(&run->random, run->ante, run->win_ante, run->bosses_used, NULL);
    run->voucher = bo_pool_next_voucher(run, 0);
    bo_pool_next_tag(run, NULL);
    bo_pool_next_tag(run, NULL);
    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 13; r++) {
            BOCard card = bo_card_make(kRankOrder[r], (BOSuit)kSuitOrder[s]);
            card.uid = bo_run_new_sort_id(run);
            run->deck[run->deck_count++] = card;
        }
    }
    run->hands_left = run->hands_per_round;
    run->discards_left = run->discards_per_round;
    shuffle_deck(run, "shuffle");
    reset_round_cards(run);
    run->slot = BO_SLOT_SMALL;
    run->state = BO_STATE_BLIND_SELECT;
}

BOBlindId bo_run_slot_blind(const BORun *run, BOBlindSlot slot) {
    if (slot == BO_SLOT_SMALL) return BO_BL_SMALL;
    if (slot == BO_SLOT_BIG) return BO_BL_BIG;
    return run->boss;
}

double bo_run_slot_chips(const BORun *run, BOBlindSlot slot) {
    int scaling = run->stake >= 6 ? 3 : run->stake >= 3 ? 2 : 1;
    return bo_blind_amount(run->ante, scaling) * bo_blind_info(bo_run_slot_blind(run, slot))->mult;
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
    run->blind_disabled = 0;
    // From stake 2 on, the Small Blind gives no reward.
    if (run->stake >= 2 && run->slot == BO_SLOT_SMALL) run->blind_dollars = 0;
    run->chips = 0;
    run->round++;
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_SETTING_BLIND);
        ctx.blind_boss = info->boss;
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    bo_run_remove_marked_jokers(run);
    char key[16];
    snprintf(key, sizeof(key), "nr%d", run->ante);
    shuffle_deck(run, key);
    draw_to_hand(run);
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_FIRST_HAND_DRAWN);
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    run->state = BO_STATE_SELECTING;
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

static void end_round(BORun *run) {
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
    if (run->ante == run->win_ante && blind->boss) run->won = 1;
    if (game_over) {
        bo_run_remove_marked_jokers(run);
        for (int i = 0; i < run->hand_count; i++) run->discard[run->discard_count++] = run->hand[i];
        run->hand_count = 0;
        run->state = BO_STATE_GAME_OVER;
        return;
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
    if (blind->boss) run->ante++;
    for (int i = 0; i < run->discard_count; i++) run->deck[run->deck_count++] = run->discard[i];
    run->discard_count = 0;
    if (blind->boss) {
        run->voucher = bo_pool_next_voucher(run, 0);
        for (int i = 0; i < run->deck_count; i++) run->deck[i].played_this_ante = 0;
    }
    reset_round_cards(run);

    // evaluate_round: the money of the round.
    memset(&run->eval, 0, sizeof(run->eval));
    add_eval_row(run, BO_EVAL_BLIND, 0, 0, run->blind_dollars);
    if (run->hands_left > 0) add_eval_row(run, BO_EVAL_HANDS, 0, run->hands_left, run->hands_left);
    for (int i = 0; i < run->joker_count; i++) {
        int has = 0;
        int dollars = bo_joker_dollar_bonus(run, &run->jokers[i], &has);
        if (has) add_eval_row(run, BO_EVAL_JOKER, i, 0, dollars);
    }
    if (run->dollars >= 5) {
        int steps = run->dollars / 5;
        if (steps > run->interest_cap / 5) steps = run->interest_cap / 5;
        add_eval_row(run, BO_EVAL_INTEREST, 0, steps, run->interest_amount * steps);
    }
    run->state = BO_STATE_ROUND_EVAL;
}

int bo_run_play(BORun *run, uint64_t mask, BOScore *score) {
    int count = count_bits(mask, run->hand_count);
    if (run->state != BO_STATE_SELECTING || run->hands_left <= 0 || count < 1 || count > BO_MAX_PLAY) return 0;
    run->blind_triggered = 0;
    run->play_count = take_from_hand(run, mask, run->play);
    run->hands_left--;
    for (int i = 0; i < run->play_count; i++) run->play[i].played_this_ante = 1;
    bo_eval_play(run, score);
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
        end_round(run);
    } else {
        draw_to_hand(run);
    }
    return 1;
}

int bo_run_discard(BORun *run, uint64_t mask) {
    int count = count_bits(mask, run->hand_count);
    if (run->state != BO_STATE_SELECTING || run->discards_left <= 0 || count < 1 || count > BO_MAX_PLAY) return 0;
    BOCard cards[BO_MAX_PLAY];
    take_from_hand(run, mask, cards);
    BOHandRules rules = bo_eval_rules(run);
    BOHandResult result = bo_hand_evaluate(cards, count, &rules);
    for (int j = 0; j < run->joker_count; j++) {
        BOContext ctx = context_for(BO_CTX_PRE_DISCARD);
        ctx.full_hand = cards;
        ctx.full_count = count;
        ctx.scoring_name = result.type;
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
    run->discards_left--;
    run->discards_used++;
    draw_to_hand(run);
    return 1;
}

static void fill_shop(BORun *run);

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
    run->chips = 0;
    if (run->slot == BO_SLOT_BOSS) {
        bo_pool_next_tag(run, NULL);
        bo_pool_next_tag(run, NULL);
        run->slot = BO_SLOT_SMALL;
        run->boss = bo_blind_new_boss(&run->random, run->ante, run->win_ante, run->bosses_used, NULL);
        run->boss_rerolled = 0;
    } else {
        run->slot++;
    }
    run->state = BO_STATE_SHOP;
    fill_shop(run);
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

static void fill_shop(BORun *run) {
    run->shop_count = 0;
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
}

int bo_run_reroll(BORun *run) {
    if (run->state != BO_STATE_SHOP) return 0;
    if (run->reroll_cost > bo_run_spendable(run) && run->reroll_cost > 0) return 0;
    if (run->reroll_cost > 0) bo_run_ease_dollars(run, -run->reroll_cost);
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
    bo_run_remove_marked_jokers(run);
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
    return bo_pack_open(run, pack.center, pack.cost);
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
    run->shop_count = 0;
    run->has_shop_voucher = 0;
    run->shop_pack_count = 0;
    run->state = BO_STATE_BLIND_SELECT;
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

void bo_run_add_joker(BORun *run, const BOItem *item) {
    if (run->joker_count >= BO_MAX_JOKERS) return;
    BOItem *slot = &run->jokers[run->joker_count++];
    *slot = *item;
    slot->couponed = 0;
    bo_joker_add_to_deck(run, slot);
    run->used_jokers[slot->center] = 1;
    bo_rules_set_cost(run, slot);
}

void bo_run_add_consumable(BORun *run, const BOItem *item) {
    if (run->consumable_count >= BO_MAX_CONSUMABLES) return;
    BOItem *slot = &run->consumables[run->consumable_count++];
    *slot = *item;
    slot->couponed = 0;
    bo_joker_add_to_deck(run, slot);
    run->used_jokers[slot->center] = 1;
    bo_rules_set_cost(run, slot);
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
