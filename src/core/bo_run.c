#include "bo_run.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma STDC FP_CONTRACT OFF

// The suits and the ranks in the order of the keys of P_CARDS as strings,
// for example "C_2" before "C_A" before "C_T". start_run makes the cards in
// this order, so each card gets its sort_id in this order.
static const char kSuitOrder[4] = {BO_CLUBS, BO_DIAMONDS, BO_HEARTS, BO_SPADES};
static const int kRankOrder[13] = {2, 3, 4, 5, 6, 7, 8, 9, BO_ACE, BO_JACK, BO_KING, BO_QUEEN, 10};

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
    bo_score_context_init(&run->context);

    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 13; r++) {
            BOCard card = bo_card_make(kRankOrder[r], (BOSuit)kSuitOrder[s]);
            card.uid = ++run->next_uid;
            run->deck[run->deck_count++] = card;
        }
    }
    run->hands_left = run->hands_per_round;
    run->discards_left = run->discards_per_round;

    // The order of start_run: the first shuffle of the deck, then the boss.
    // The keys are not the same, so the order changes no number.
    double seedShuffle = bo_pseudoseed(&run->random, "shuffle");
    int order[BO_MAX_CARDS];
    bo_pseudoshuffle(&run->random, seedShuffle, order, run->deck_count);
    BOCard shuffled[BO_MAX_CARDS];
    for (int i = 0; i < run->deck_count; i++) shuffled[i] = run->deck[order[i]];
    memcpy(run->deck, shuffled, sizeof(BOCard) * run->deck_count);
    run->boss = bo_blind_new_boss(&run->random, run->ante, run->win_ante, run->bosses_used, NULL);
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
    int space = run->hand_size - run->hand_count;
    if (space > run->deck_count) space = run->deck_count;
    for (int i = 0; i < space && run->hand_count < BO_MAX_HAND; i++) {
        run->hand[run->hand_count++] = run->deck[--run->deck_count];
    }
    bo_run_sort_hand(run, run->sort_by_suit);
}

void bo_run_select_blind(BORun *run) {
    if (run->state != BO_STATE_BLIND_SELECT) return;
    run->blind = bo_run_slot_blind(run, run->slot);
    const BOBlindInfo *info = bo_blind_info(run->blind);
    run->blind_chips = bo_run_slot_chips(run, run->slot);
    run->blind_dollars = info->dollars;
    // From stake 2 on, the Small Blind gives no reward.
    if (run->stake >= 2 && run->slot == BO_SLOT_SMALL) run->blind_dollars = 0;
    run->chips = 0;
    run->round++;
    char key[16];
    snprintf(key, sizeof(key), "nr%d", run->ante);
    shuffle_deck(run, key);
    draw_to_hand(run);
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

static void end_round(BORun *run) {
    // The cards that are left go to the discard pile with the others.
    for (int i = 0; i < run->hand_count; i++) run->discard[run->discard_count++] = run->hand[i];
    run->hand_count = 0;
    if (run->chips < run->blind_chips) {
        run->state = BO_STATE_GAME_OVER;
        return;
    }
    if (run->ante == run->win_ante && run->slot == BO_SLOT_BOSS) run->won = 1;

    BORoundEval *eval = &run->eval;
    memset(eval, 0, sizeof(*eval));
    eval->blind = run->blind_dollars;
    eval->hands = run->hands_left;
    if (run->dollars >= 5) {
        int steps = run->dollars / 5;
        if (steps > run->interest_cap / 5) steps = run->interest_cap / 5;
        eval->interest = steps;
    }
    eval->total = eval->blind + eval->hands + eval->interest;
    run->state = BO_STATE_ROUND_EVAL;
}

int bo_run_play(BORun *run, uint64_t mask, BOScore *score) {
    int count = count_bits(mask, run->hand_count);
    if (run->state != BO_STATE_SELECTING || run->hands_left <= 0 || count < 1 || count > BO_MAX_PLAY) return 0;
    BOCard played[BO_MAX_PLAY];
    take_from_hand(run, mask, played);
    bo_score_play(&run->context, played, count, run->hand, run->hand_count, score);
    run->hands_played[score->type]++;
    run->chips += score->total;
    run->hands_left--;
    for (int i = 0; i < count; i++) run->discard[run->discard_count++] = played[i];
    if (run->chips >= run->blind_chips || run->hands_left == 0) {
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
    for (int i = 0; i < count; i++) run->discard[run->discard_count++] = cards[i];
    run->discards_left--;
    draw_to_hand(run);
    return 1;
}

void bo_run_cash_out(BORun *run) {
    if (run->state != BO_STATE_ROUND_EVAL) return;
    run->dollars += run->eval.total;
    run->chips = 0;
    // All the cards go back to the deck.
    for (int i = 0; i < run->discard_count; i++) run->deck[run->deck_count++] = run->discard[i];
    run->discard_count = 0;
    char key[24];
    snprintf(key, sizeof(key), "cashout%d", run->ante);
    shuffle_deck(run, key);
    run->hands_left = run->hands_per_round;
    run->discards_left = run->discards_per_round;
    if (run->slot == BO_SLOT_BOSS) {
        run->ante++;
        run->slot = BO_SLOT_SMALL;
        run->boss = bo_blind_new_boss(&run->random, run->ante, run->win_ante, run->bosses_used, NULL);
    } else {
        run->slot++;
    }
    run->state = BO_STATE_BLIND_SELECT;
}
