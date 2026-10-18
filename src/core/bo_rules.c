#include "bo_rules.h"

#include <math.h>
#include <string.h>

#include "bo_joker.h"
#include "bo_run.h"

#pragma STDC FP_CONTRACT OFF

// From tools/oracle.sh: the order of pairs(G.GAME.hands) in LuaJIT.
static const int kHandsPairsOrder[BO_HAND_COUNT] = {
    BO_HAND_FLUSH, BO_HAND_FLUSH_FIVE, BO_HAND_PAIR, BO_HAND_STRAIGHT_FLUSH,
    BO_HAND_TWO_PAIR, BO_HAND_HIGH_CARD, BO_HAND_FIVE_OF_A_KIND, BO_HAND_FULL_HOUSE,
    BO_HAND_THREE_OF_A_KIND, BO_HAND_STRAIGHT, BO_HAND_FLUSH_HOUSE, BO_HAND_FOUR_OF_A_KIND,
};

int bo_rules_hands_pairs_order(int i) {
    return kHandsPairsOrder[i];
}

int bo_rules_hand_by_name(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < BO_HAND_COUNT; i++) {
        if (strcmp(bo_hand_name((BOHandType)i, 0), name) == 0) return i;
    }
    return -1;
}

int bo_rules_suit_by_name(const char *name) {
    if (!name) return -1;
    if (strcmp(name, "Spades") == 0) return BO_SPADES;
    if (strcmp(name, "Hearts") == 0) return BO_HEARTS;
    if (strcmp(name, "Clubs") == 0) return BO_CLUBS;
    if (strcmp(name, "Diamonds") == 0) return BO_DIAMONDS;
    return -1;
}

int bo_rules_get_id(const BOCard *card) {
    if (bo_card_is_stone(card) && !card->vampired) return -100 - (int)(card->uid % 1000000);
    return card->rank;
}

int bo_rules_is_face(const BORun *run, const BOCard *card, int from_boss) {
    if (card->debuffed && !from_boss) return 0;
    int id = bo_rules_get_id(card);
    return id == 11 || id == 12 || id == 13 || bo_run_find_joker(run, BO_J_PAREIDOLIA) > 0;
}

static int is_red(int suit) {
    return suit == BO_HEARTS || suit == BO_DIAMONDS;
}

int bo_rules_is_suit(const BORun *run, const BOCard *card, int suit, int bypass_debuff, int flush_calc) {
    int smeared = bo_run_find_joker(run, BO_J_SMEARED) > 0;
    if (flush_calc) {
        if (bo_card_is_stone(card)) return 0;
        if (card->enhancement == BO_ENH_WILD && !card->debuffed) return 1;
        if (smeared && is_red(card->suit) == is_red(suit)) return 1;
        return card->suit == suit;
    }
    if (card->debuffed && !bypass_debuff) return 0;
    if (bo_card_is_stone(card)) return 0;
    if (card->enhancement == BO_ENH_WILD) return 1;
    if (smeared && is_red(card->suit) == is_red(suit)) return 1;
    return card->suit == suit;
}

static const BOCenterId kEnhancementCenters[] = {
    [BO_ENH_NONE] = BO_C_BASE,   [BO_ENH_BONUS] = BO_M_BONUS, [BO_ENH_MULT] = BO_M_MULT,
    [BO_ENH_WILD] = BO_M_WILD,   [BO_ENH_GLASS] = BO_M_GLASS, [BO_ENH_STEEL] = BO_M_STEEL,
    [BO_ENH_STONE] = BO_M_STONE, [BO_ENH_GOLD] = BO_M_GOLD,   [BO_ENH_LUCKY] = BO_M_LUCKY,
};

BOCenterId bo_rules_enhancement_center(int enhancement) {
    if (enhancement < 0 || enhancement > BO_ENH_LUCKY) return BO_C_BASE;
    return kEnhancementCenters[enhancement];
}

int bo_rules_enhancement_of(BOCenterId center) {
    for (int i = 0; i <= BO_ENH_LUCKY; i++) {
        if (kEnhancementCenters[i] == center) return i;
    }
    return BO_ENH_NONE;
}

void bo_rules_set_enhancement(BORun *run, BOCard *card, int enhancement) {
    (void)run;
    card->enhancement = (uint8_t)enhancement;
}

typedef int (*BOCardTest)(const BOCard *card, int value);

static int count_cards(const BORun *run, BOCardTest test, int value) {
    int count = 0;
    const BOCard *areas[4] = {run->deck, run->hand, run->discard, run->play};
    int counts[4] = {run->deck_count, run->hand_count, run->discard_count, run->play_count};
    for (int a = 0; a < 4; a++) {
        for (int i = 0; i < counts[a]; i++) {
            if (!test || test(&areas[a][i], value)) count++;
        }
    }
    return count;
}

static int has_enhancement(const BOCard *card, int enhancement) {
    return card->enhancement == enhancement;
}

static int is_enhanced(const BOCard *card, int unused) {
    (void)unused;
    return card->enhancement != BO_ENH_NONE;
}

static int has_rank(const BOCard *card, int rank) {
    return bo_rules_get_id(card) == rank;
}

int bo_rules_playing_card_count(const BORun *run) {
    return count_cards(run, NULL, 0);
}

int bo_rules_count_enhancement(const BORun *run, int enhancement) {
    return count_cards(run, has_enhancement, enhancement);
}

int bo_rules_count_enhanced(const BORun *run) {
    return count_cards(run, is_enhanced, 0);
}

int bo_rules_count_rank(const BORun *run, int rank) {
    return count_cards(run, has_rank, rank);
}

void bo_rules_change_hand_size(BORun *run, int delta) {
    run->hand_size += delta;
    if (run->hand_size < 0) run->hand_size = 0;
}

void bo_rules_set_cost(BORun *run, BOItem *item) {
    bo_item_set_cost(item, run->inflation, run->discount_percent, bo_run_find_joker(run, BO_J_ASTRONOMER) > 0, 0,
                     run->ante);
}

void bo_rules_set_all_costs(BORun *run) {
    for (int i = 0; i < run->joker_count; i++) bo_rules_set_cost(run, &run->jokers[i]);
    for (int i = 0; i < run->consumable_count; i++) bo_rules_set_cost(run, &run->consumables[i]);
    for (int i = 0; i < run->shop_count; i++) bo_rules_set_cost(run, &run->shop[i].item);
    if (run->has_shop_voucher) bo_rules_set_cost(run, &run->shop_voucher);
    for (int i = 0; i < run->shop_pack_count; i++) bo_rules_set_cost(run, &run->shop_packs[i]);
}

void bo_rules_calculate_reroll_cost(BORun *run, int skip_increment) {
    if (run->free_rerolls < 0) run->free_rerolls = 0;
    if (run->free_rerolls > 0) {
        run->reroll_cost = 0;
        return;
    }
    if (!skip_increment) run->reroll_cost_increase++;
    run->reroll_cost = run->reroll_cost_base + run->reroll_cost_increase;
}

BOItem bo_rules_copy_item(BORun *run, const BOItem *other, int strip_edition) {
    BOItem copy = *other;
    copy.sort_id = bo_run_new_sort_id(run);
    copy.id = bo_run_new_id(run);
    copy.getting_sliced = 0;
    if (strip_edition) copy.edition = BO_EDITION_NONE;
    bo_rules_set_cost(run, &copy);
    return copy;
}

void bo_rules_disable_blind(BORun *run) {
    run->blind_disabled = 1;
}

void bo_rules_add_tag(BORun *run, BOCenterId tag) {
    if (run->tag_count < (int)(sizeof(run->tags) / sizeof(run->tags[0]))) run->tags[run->tag_count++] = tag;
}

// The keys of G.P_CARDS in the order of their sort: "C_2" to "C_9", "C_A",
// "C_J", "C_K", "C_Q", "C_T", then the same for D, H, and S.
static const int kFrontSuits[4] = {BO_CLUBS, BO_DIAMONDS, BO_HEARTS, BO_SPADES};
static const int kFrontRanks[13] = {2, 3, 4, 5, 6, 7, 8, 9, BO_ACE, BO_JACK, BO_KING, BO_QUEEN, 10};

BOCard bo_rules_random_front(BORun *run, const char *key) {
    int index = bo_random_index(&run->random, bo_pseudoseed(&run->random, key), 52);
    return bo_card_make(kFrontRanks[index % 13], (BOSuit)kFrontSuits[index / 13]);
}

BOCard bo_rules_new_playing_card(BORun *run, int rank, int suit) {
    BOCard card = bo_card_make(rank, (BOSuit)suit);
    card.uid = bo_run_new_sort_id(run);
    return card;
}

void bo_rules_playing_cards_added(BORun *run, int count) {
    BOCard none;
    memset(&none, 0, sizeof(none));
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.kind = BO_CTX_PLAYING_CARD_ADDED;
        ctx.full_count = count;
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
}

void bo_rules_certificate(BORun *run) {
    BOCard front = bo_rules_random_front(run, "cert_fr");
    BOCard card = bo_rules_new_playing_card(run, front.rank, front.suit);
    double seal = bo_pseudorandom(&run->random, "certsl");
    if (seal > 0.75) card.seal = BO_SEAL_RED;
    else if (seal > 0.5) card.seal = BO_SEAL_BLUE;
    else if (seal > 0.25) card.seal = BO_SEAL_GOLD;
    else card.seal = BO_SEAL_PURPLE;
    if (run->hand_count < BO_MAX_HAND) run->hand[run->hand_count++] = card;
    bo_run_sort_hand(run, run->sort_by_suit);
    bo_rules_playing_cards_added(run, 1);
}

void bo_rules_add_stone_card(BORun *run) {
    BOCard front = bo_rules_random_front(run, "marb_fr");
    BOCard card = bo_rules_new_playing_card(run, front.rank, front.suit);
    card.enhancement = BO_ENH_STONE;
    if (run->deck_count < BO_MAX_CARDS) run->deck[run->deck_count++] = card;
    bo_rules_playing_cards_added(run, 1);
}

void bo_rules_dna_copy(BORun *run, const BOCard *card) {
    BOCard copy = *card;
    copy.uid = bo_run_new_sort_id(run);
    if (run->hand_count < BO_MAX_HAND) run->hand[run->hand_count++] = copy;
}
