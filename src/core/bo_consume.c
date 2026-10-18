#include "bo_consume.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bo_joker.h"
#include "bo_pool.h"
#include "bo_rules.h"
#include "bo_run.h"

#pragma STDC FP_CONTRACT OFF

#define IS(key) (item->center == BO_C_##key)

static int count_bits(uint64_t mask, int limit) {
    int count = 0;
    for (int i = 0; i < limit; i++) count += (int)((mask >> i) & 1);
    return count;
}

// The hand is on, or a pack shows the hand: SELECTING_HAND, TAROT_PACK,
// SPECTRAL_PACK, or PLANET_PACK.
static int hand_state(const BORun *run) {
    if (run->state == BO_STATE_SELECTING) return 1;
    if (run->state != BO_STATE_PACK) return 0;
    const BOCenterInfo *pack = bo_center(run->pack);
    return pack->kind && (strcmp(pack->kind, "Arcana") == 0 || strcmp(pack->kind, "Spectral") == 0 ||
                          strcmp(pack->kind, "Celestial") == 0);
}

// The jokers with no edition: eligible_strength_jokers and
// eligible_editionless_jokers of Card:update.
static int editionless_jokers(const BORun *run, int *out) {
    int count = 0;
    for (int i = 0; i < run->joker_count; i++) {
        if (bo_item_center(&run->jokers[i])->set == BO_SET_JOKER && run->jokers[i].edition == BO_EDITION_NONE) {
            out[count++] = i;
        }
    }
    return count;
}

int bo_consume_temperance_money(const BORun *run, const BOItem *item) {
    int money = 0;
    for (int i = 0; i < run->joker_count; i++) {
        if (bo_item_center(&run->jokers[i])->set == BO_SET_JOKER) money += run->jokers[i].sell_cost;
    }
    int cap = (int)item->ability.extra;
    return money < cap ? money : cap;
}

int bo_consume_can_use(const BORun *run, const BOItem *item, uint64_t highlighted, BOUseFrom from) {
    const BOCenterInfo *c = bo_item_center(item);
    if (!c || !c->consumeable) return 0;
    int selected = count_bits(highlighted, run->hand_count);
    int eligible[BO_MAX_JOKERS];
    if (IS(HERMIT) || bo_center_text(c, BO_F_HAND_TYPE) || IS(TEMPERANCE) || IS(BLACK_HOLE)) return 1;
    if (IS(WHEEL_OF_FORTUNE) && editionless_jokers(run, eligible) > 0) return 1;
    if (IS(ANKH)) {
        for (int i = 0; i < run->joker_count; i++) {
            if (bo_item_center(&run->jokers[i])->set == BO_SET_JOKER && run->joker_slots > 1) return 1;
        }
    }
    if (IS(AURA) && selected == 1) {
        for (int i = 0; i < run->hand_count; i++) {
            if (((highlighted >> i) & 1) && run->hand[i].edition == BO_EDITION_NONE) return 1;
        }
    }
    if ((IS(ECTOPLASM) || IS(HEX)) && editionless_jokers(run, eligible) > 0) return 1;
    if (IS(EMPEROR) || IS(HIGH_PRIESTESS)) {
        if (run->consumable_count < run->consumable_slots || from == BO_USE_FROM_CONSUMABLES) return 1;
    }
    if (IS(FOOL)) {
        if ((run->consumable_count < run->consumable_slots || from == BO_USE_FROM_CONSUMABLES) &&
            run->last_tarot_planet != BO_CENTER_NONE && run->last_tarot_planet != BO_C_FOOL) {
            return 1;
        }
    }
    if (IS(JUDGEMENT) || IS(SOUL) || IS(WRAITH)) return run->joker_count < run->joker_slots;
    if (hand_state(run)) {
        int has = 0;
        double max = bo_center_number(c, BO_F_MAX_HIGHLIGHTED, &has);
        if (has) {
            int minHas = 0;
            double min = bo_center_number(c, BO_F_MIN_HIGHLIGHTED, &minHas);
            if (!minHas) min = 1;
            if (max >= selected && selected >= min) return 1;
        }
        if ((IS(FAMILIAR) || IS(GRIM) || IS(INCANTATION) || IS(IMMOLATE) || IS(SIGIL) || IS(OUIJA)) &&
            run->hand_count > 1) {
            return 1;
        }
    }
    return 0;
}

// --- The playing cards ----------------------------------------------------

static int compare_uid(const void *a, const void *b) {
    const BOCard *x = *(BOCard *const *)a, *y = *(BOCard *const *)b;
    return x->uid < y->uid ? -1 : x->uid > y->uid;
}

// pseudorandom_element of a list of cards: the cards sort by sort_id.
static BOCard *random_card(BORun *run, BOCard **cards, int count, const char *key) {
    if (count == 0) return NULL;
    qsort(cards, count, sizeof(BOCard *), compare_uid);
    return cards[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), count)];
}

// The suits and the ranks of pseudorandom_element of a list of letters.
static const int kSuitLetters[4] = {BO_SPADES, BO_HEARTS, BO_DIAMONDS, BO_CLUBS};  // S H D C

static int random_suit(BORun *run, const char *key) {
    return kSuitLetters[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), 4)];
}

static int random_rank(BORun *run, const int *ranks, int count, const char *key) {
    return ranks[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), count)];
}

// The order of P_CENTER_POOLS.Enhanced without Stone, for Familiar, Grim,
// and Incantation.
static const int kSpectralEnhancements[7] = {BO_ENH_BONUS, BO_ENH_MULT, BO_ENH_WILD, BO_ENH_GLASS,
                                             BO_ENH_STEEL, BO_ENH_GOLD, BO_ENH_LUCKY};

// The removed cards: the jokers see them, then they leave the hand.
static void remove_cards(BORun *run, uint32_t *uids, int count) {
    int faces = 0;
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < run->hand_count; j++) {
            if (run->hand[j].uid == uids[i] && bo_rules_is_face(run, &run->hand[j], 0)) faces++;
        }
    }
    // The jokers see the cards before they break, so no card is shattered
    // yet for Glass Joker.
    for (int j = 0; j < run->joker_count; j++) {
        BOContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.kind = BO_CTX_REMOVE_PLAYING_CARDS;
        ctx.cardarea = BO_IN_JOKERS;
        ctx.faces_removed = faces;
        bo_joker_calculate(run, &run->jokers[j], &ctx);
    }
    int kept = 0;
    for (int j = 0; j < run->hand_count; j++) {
        int gone = 0;
        for (int i = 0; i < count; i++) gone |= run->hand[j].uid == uids[i];
        if (!gone) run->hand[kept++] = run->hand[j];
    }
    run->hand_count = kept;
}

static void add_to_hand(BORun *run, BOCard card) {
    if (run->hand_count < BO_MAX_HAND) run->hand[run->hand_count++] = card;
}

// --- The jokers -----------------------------------------------------------

static void destroy_joker(BORun *run, int index) {
    run->jokers[index].getting_sliced = 1;
}

static void remove_destroyed_jokers(BORun *run) {
    for (int i = run->joker_count - 1; i >= 0; i--) {
        if (run->jokers[i].getting_sliced) bo_run_remove_joker(run, i);
    }
}

// Card:set_edition on a joker in the slots. A new Negative edition gives
// one more slot.
static void set_joker_edition(BORun *run, BOItem *joker, int edition) {
    if (edition == BO_EDITION_NEGATIVE && joker->edition != BO_EDITION_NEGATIVE) run->joker_slots++;
    joker->edition = (uint8_t)edition;
    bo_rules_set_cost(run, joker);
}

static int random_joker(BORun *run, const int *indexes, int count, const char *key) {
    // The jokers are in the order of their sort_id for pseudorandom_element.
    int sorted[BO_MAX_JOKERS];
    memcpy(sorted, indexes, sizeof(int) * count);
    for (int i = 1; i < count; i++) {
        int t = sorted[i], j = i - 1;
        while (j >= 0 && run->jokers[sorted[j]].sort_id > run->jokers[t].sort_id) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = t;
    }
    return sorted[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), count)];
}

static void create_consumable(BORun *run, int type, BOCenterId forced, const char *key) {
    if (run->consumable_count >= run->consumable_slots) return;
    BOItem card = bo_run_create_item(run, type, 0, 0, 0, forced, key, 0);
    bo_run_add_consumable(run, &card);
}

// --- The use --------------------------------------------------------------

// set_consumeable_usage.
static void count_usage(BORun *run, const BOItem *item, BOCenterId *last) {
    const BOCenterInfo *c = bo_item_center(item);
    if (c->set == BO_SET_TAROT) run->tarots_used++;
    if (c->set == BO_SET_PLANET && !run->planet_used[item->center]) {
        run->planet_used[item->center] = 1;
        run->planets_used_unique++;
    }
    // An event sets last_tarot_planet after the events of the card, so The
    // Fool makes the card of before.
    if (c->set == BO_SET_TAROT || c->set == BO_SET_PLANET) *last = item->center;
}

void bo_consume_use(BORun *run, const BOItem *item, uint64_t highlighted) {
    const BOCenterInfo *c = bo_item_center(item);
    BOCenterId last = run->last_tarot_planet;
    count_usage(run, item, &last);
    if (item->debuffed) {
        run->last_tarot_planet = last;
        return;
    }
    // G.hand.highlighted, in the order of the hand.
    BOCard *chosen[BO_MAX_HAND];
    int chosenCount = 0;
    for (int i = 0; i < run->hand_count; i++) {
        if ((highlighted >> i) & 1) chosen[chosenCount++] = &run->hand[i];
    }
    int glassChosen = 0;
    for (int i = 0; i < chosenCount; i++) glassChosen += chosen[i]->enhancement == BO_ENH_GLASS;
    const char *modConv = bo_center_text(c, BO_F_MOD_CONV);
    const char *suitConv = bo_center_text(c, BO_F_SUIT_CONV);

    if (modConv || suitConv) {
        if (IS(DEATH)) {
            // The card on the right becomes the model.
            BOCard *rightmost = chosenCount ? chosen[chosenCount - 1] : NULL;
            for (int i = 0; i < chosenCount; i++) {
                if (chosen[i] == rightmost) continue;
                uint32_t uid = chosen[i]->uid;
                *chosen[i] = *rightmost;
                chosen[i]->uid = uid;
            }
        } else if (IS(STRENGTH)) {
            for (int i = 0; i < chosenCount; i++) {
                int rank = chosen[i]->rank;
                chosen[i]->rank = (uint8_t)(rank == BO_ACE ? 2 : rank + 1);
            }
        } else if (suitConv) {
            int suit = bo_rules_suit_by_name(suitConv);
            for (int i = 0; i < chosenCount; i++) chosen[i]->suit = (uint8_t)suit;
        } else {
            int enhancement = bo_rules_enhancement_of(bo_center_by_key(modConv));
            for (int i = 0; i < chosenCount; i++) bo_rules_set_enhancement(run, chosen[i], enhancement);
        }
    }
    if (IS(BLACK_HOLE)) {
        for (int h = 0; h < BO_HAND_COUNT; h++) bo_run_level_up_hand(run, (BOHandType)h, 1);
    }
    if (IS(TALISMAN) || IS(DEJA_VU) || IS(TRANCE) || IS(MEDIUM)) {
        const char *seal = bo_center_text(c, BO_F_EXTRA);
        if (chosenCount > 0 && seal) {
            if (strcmp(seal, "Gold") == 0) chosen[0]->seal = BO_SEAL_GOLD;
            if (strcmp(seal, "Red") == 0) chosen[0]->seal = BO_SEAL_RED;
            if (strcmp(seal, "Blue") == 0) chosen[0]->seal = BO_SEAL_BLUE;
            if (strcmp(seal, "Purple") == 0) chosen[0]->seal = BO_SEAL_PURPLE;
        }
    }
    if (IS(AURA) && chosenCount > 0) {
        chosen[0]->edition = (uint8_t)bo_pool_poll_edition(run, "aura", 1, 1, 1);
    }
    if (IS(CRYPTID) && chosenCount > 0) {
        BOCard model = *chosen[0];
        int copies = (int)item->ability.extra;
        for (int i = 0; i < copies; i++) {
            BOCard copy = model;
            copy.uid = bo_run_new_sort_id(run);
            add_to_hand(run, copy);
        }
        bo_rules_playing_cards_added(run, copies);
    }
    if (IS(SIGIL) || IS(OUIJA)) {
        if (IS(SIGIL)) {
            int suit = random_suit(run, "sigil");
            for (int i = 0; i < run->hand_count; i++) run->hand[i].suit = (uint8_t)suit;
        } else {
            static const int ranks[13] = {2, 3, 4, 5, 6, 7, 8, 9, 10, BO_JACK, BO_QUEEN, BO_KING, BO_ACE};
            int rank = random_rank(run, ranks, 13, "ouija");
            for (int i = 0; i < run->hand_count; i++) run->hand[i].rank = (uint8_t)rank;
            bo_rules_change_hand_size(run, -1);
        }
    }
    if (bo_center_text(c, BO_F_HAND_TYPE)) {
        int type = bo_rules_hand_by_name(bo_center_text(c, BO_F_HAND_TYPE));
        if (type >= 0) bo_run_level_up_hand(run, (BOHandType)type, 1);
    }
    int removeCard = 0;
    bo_center_number(c, BO_F_REMOVE_CARD, &removeCard);
    if (removeCard) {
        uint32_t destroyed[BO_MAX_HAND];
        int destroyedCount = 0;
        if (IS(HANGED_MAN)) {
            for (int i = chosenCount - 1; i >= 0; i--) destroyed[destroyedCount++] = chosen[i]->uid;
            remove_cards(run, destroyed, destroyedCount);
        } else if (IS(FAMILIAR) || IS(GRIM) || IS(INCANTATION)) {
            BOCard *hand[BO_MAX_HAND];
            for (int i = 0; i < run->hand_count; i++) hand[i] = &run->hand[i];
            BOCard *gone = random_card(run, hand, run->hand_count, "random_destroy");
            if (gone) destroyed[destroyedCount++] = gone->uid;
            remove_cards(run, destroyed, destroyedCount);
            int count = (int)item->ability.extra;
            for (int i = 0; i < count; i++) {
                int rank = BO_ACE, suit;
                if (IS(FAMILIAR)) {
                    static const int faces[3] = {BO_JACK, BO_QUEEN, BO_KING};
                    rank = random_rank(run, faces, 3, "familiar_create");
                    suit = random_suit(run, "familiar_create");
                } else if (IS(GRIM)) {
                    suit = random_suit(run, "grim_create");
                } else {
                    static const int numbers[9] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
                    rank = random_rank(run, numbers, 9, "incantation_create");
                    suit = random_suit(run, "incantation_create");
                }
                int enhancement = kSpectralEnhancements[bo_random_index(
                    &run->random, bo_pseudoseed(&run->random, "spe_card"), 7)];
                BOCard card = bo_rules_new_playing_card(run, rank, suit);
                card.enhancement = (uint8_t)enhancement;
                add_to_hand(run, card);
            }
            bo_rules_playing_cards_added(run, count);
        } else if (IS(IMMOLATE)) {
            BOCard *hand[BO_MAX_HAND];
            int count = run->hand_count;
            for (int i = 0; i < count; i++) hand[i] = &run->hand[i];
            qsort(hand, count, sizeof(BOCard *), compare_uid);
            int order[BO_MAX_HAND];
            bo_pseudoshuffle(&run->random, bo_pseudoseed(&run->random, "immolate"), order, count);
            int destroy = (int)bo_item_get(item, BO_F_EXTRA_DESTROY);
            for (int i = 0; i < destroy && i < count; i++) destroyed[destroyedCount++] = hand[order[i]]->uid;
            remove_cards(run, destroyed, destroyedCount);
            bo_run_ease_dollars(run, (int)bo_item_get(item, BO_F_EXTRA_DOLLARS));
        }
    }
    if (IS(FOOL)) create_consumable(run, BO_POOL_TAROT_PLANET, run->last_tarot_planet, "fool");
    if (IS(HERMIT)) {
        int dollars = run->dollars < (int)item->ability.extra ? run->dollars : (int)item->ability.extra;
        bo_run_ease_dollars(run, dollars > 0 ? dollars : 0);
    }
    if (IS(TEMPERANCE)) bo_run_ease_dollars(run, bo_consume_temperance_money(run, item));
    if (IS(EMPEROR) || IS(HIGH_PRIESTESS)) {
        int count = (int)bo_center_number(c, IS(EMPEROR) ? BO_F_TAROTS : BO_F_PLANETS, NULL);
        int space = run->consumable_slots - run->consumable_count;
        if (count > space) count = space;
        for (int i = 0; i < count; i++) {
            create_consumable(run, IS(EMPEROR) ? BO_SET_TAROT : BO_SET_PLANET, BO_CENTER_NONE,
                              IS(EMPEROR) ? "emp" : "pri");
        }
    }
    if (IS(JUDGEMENT) || IS(SOUL)) {
        BOItem joker = bo_run_create_item(run, BO_SET_JOKER, 0, IS(SOUL), 0, BO_CENTER_NONE, IS(SOUL) ? "sou" : "jud", 0);
        bo_run_add_joker(run, &joker);
    }
    if (IS(ANKH) && run->joker_count > 0) {
        int all[BO_MAX_JOKERS];
        for (int i = 0; i < run->joker_count; i++) all[i] = i;
        int chosenJoker = random_joker(run, all, run->joker_count, "ankh_choice");
        BOItem model = run->jokers[chosenJoker];
        for (int i = 0; i < run->joker_count; i++) {
            if (i != chosenJoker && !run->jokers[i].eternal) destroy_joker(run, i);
        }
        remove_destroyed_jokers(run);
        BOItem copy = bo_rules_copy_item(run, &model, model.edition == BO_EDITION_NEGATIVE);
        bo_run_add_joker(run, &copy);
    }
    if (IS(WRAITH)) {
        BOItem joker = bo_run_create_item(run, BO_SET_JOKER, 3, 0, 0, BO_CENTER_NONE, "wra", 0);
        bo_run_add_joker(run, &joker);
        if (run->dollars != 0) bo_run_ease_dollars(run, -run->dollars);
    }
    if (IS(WHEEL_OF_FORTUNE) || IS(ECTOPLASM) || IS(HEX)) {
        int eligible[BO_MAX_JOKERS];
        int count = editionless_jokers(run, eligible);
        int hit = IS(ECTOPLASM) || IS(HEX) ||
                  bo_pseudorandom(&run->random, "wheel_of_fortune") < run->probabilities / item->ability.extra;
        if (hit && count > 0) {
            const char *seed = IS(WHEEL_OF_FORTUNE) ? "wheel_of_fortune" : IS(ECTOPLASM) ? "ectoplasm" : "hex";
            int target = random_joker(run, eligible, count, seed);
            BOItem *joker = &run->jokers[target];
            int edition = IS(ECTOPLASM)  ? BO_EDITION_NEGATIVE
                          : IS(HEX)      ? BO_EDITION_POLYCHROME
                                         : bo_pool_poll_edition(run, "wheel_of_fortune", 1, 1, 1);
            set_joker_edition(run, joker, edition);
            if (IS(HEX)) {
                for (int i = 0; i < run->joker_count; i++) {
                    if (i != target && !run->jokers[i].eternal) destroy_joker(run, i);
                }
                remove_destroyed_jokers(run);
            }
            if (IS(ECTOPLASM)) {
                if (run->ecto_minus == 0) run->ecto_minus = 1;
                bo_rules_change_hand_size(run, -run->ecto_minus);
                run->ecto_minus++;
            }
        }
    }
    run->last_tarot_planet = last;

    // The jokers see the use: Constellation, and Glass Joker for The
    // Hanged Man.
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.kind = BO_CTX_USING_CONSUMEABLE;
        ctx.consumeable = (BOItem *)item;
        ctx.highlighted_glass = glassChosen;
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
}
