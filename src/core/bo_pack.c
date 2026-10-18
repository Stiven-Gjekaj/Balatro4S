#include "bo_profile.h"
#include "bo_pack.h"

#include <stdio.h>
#include <string.h>

#include "bo_consume.h"
#include "bo_joker.h"
#include "bo_pool.h"
#include "bo_rules.h"
#include "bo_tag.h"

#pragma STDC FP_CONTRACT OFF

static int kind_is(const BOCenterInfo *pack, const char *kind) {
    return pack->kind && strcmp(pack->kind, kind) == 0;
}

static BOContext context_for(BOContextKind kind) {
    BOContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.kind = kind;
    return ctx;
}

// The planet of the hand that was played most, for Telescope.
static BOCenterId telescope_planet(BORun *run) {
    int best = -1, tally = 0;
    // G.handlist: from Flush Five down to High Card.
    for (int h = 0; h < BO_HAND_COUNT; h++) {
        if (run->hands[h].visible && run->hands[h].played > tally) {
            best = h;
            tally = run->hands[h].played;
        }
    }
    if (best < 0) return BO_CENTER_NONE;
    BOCenterId planet = BO_CENTER_NONE;
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        const BOCenterInfo *c = bo_center((BOCenterId)i);
        if (c->set == BO_SET_PLANET && bo_rules_hand_by_name(bo_center_text(c, BO_F_HAND_TYPE)) == best) {
            planet = (BOCenterId)i;
        }
    }
    return planet;
}

// A card of a Standard pack: a playing card with a chance of an
// enhancement, an edition, and a seal.
static BOShopCard standard_card(BORun *run) {
    BOShopCard out;
    memset(&out, 0, sizeof(out));
    char key[32];
    snprintf(key, sizeof(key), "stdset%d", run->ante);
    int enhanced = bo_pseudorandom(&run->random, key) > 0.6;
    BOItem made = enhanced ? bo_run_create_item(run, BO_SET_ENHANCED, 0, 0, 0, BO_CENTER_NONE, "sta", 0)
                           : bo_pool_make_item(run, BO_C_BASE);
    snprintf(key, sizeof(key), "frontsta%d", run->ante);
    BOCard card = bo_rules_random_front(run, key);
    card.uid = made.sort_id;
    card.enhancement = (uint8_t)bo_rules_enhancement_of(made.center);
    snprintf(key, sizeof(key), "standard_edition%d", run->ante);
    card.edition = (uint8_t)bo_pool_poll_edition(run, key, 2, 1, 0);
    snprintf(key, sizeof(key), "stdseal%d", run->ante);
    if (bo_pseudorandom(&run->random, key) > 1 - 0.02 * 10) {
        snprintf(key, sizeof(key), "stdsealtype%d", run->ante);
        double type = bo_pseudorandom(&run->random, key);
        card.seal = type > 0.75 ? BO_SEAL_RED : type > 0.5 ? BO_SEAL_BLUE : type > 0.25 ? BO_SEAL_GOLD : BO_SEAL_PURPLE;
    }
    out.is_playing_card = 1;
    out.card = card;
    out.item = made;
    return out;
}

static BOShopCard pack_card(BORun *run, const BOCenterInfo *pack, int index) {
    BOShopCard out;
    memset(&out, 0, sizeof(out));
    if (kind_is(pack, "Arcana")) {
        if (run->used_vouchers[BO_V_OMEN_GLOBE] && bo_pseudorandom(&run->random, "omen_globe") > 0.8) {
            out.item = bo_run_create_item(run, BO_SET_SPECTRAL, 0, 0, 1, BO_CENTER_NONE, "ar2", 2);
        } else {
            out.item = bo_run_create_item(run, BO_SET_TAROT, 0, 0, 1, BO_CENTER_NONE, "ar1", 2);
        }
    } else if (kind_is(pack, "Celestial")) {
        BOCenterId forced = BO_CENTER_NONE;
        if (run->used_vouchers[BO_V_TELESCOPE] && index == 0) forced = telescope_planet(run);
        out.item = bo_run_create_item(run, BO_SET_PLANET, 0, 0, 1, forced, "pl1", 2);
    } else if (kind_is(pack, "Spectral")) {
        out.item = bo_run_create_item(run, BO_SET_SPECTRAL, 0, 0, 1, BO_CENTER_NONE, "spe", 2);
    } else if (kind_is(pack, "Standard")) {
        out = standard_card(run);
    } else {
        out.item = bo_run_create_item(run, BO_SET_JOKER, 0, 0, 1, BO_CENTER_NONE, "buf", 2);
    }
    return out;
}

// G.FUNCS.draw_from_deck_to_hand for an Arcana or a Spectral pack.
static void draw_hand(BORun *run) {
    int limit = run->hand_size > 0 ? run->hand_size : 0;
    int space = limit - run->hand_count;
    if (space > run->deck_count) space = run->deck_count;
    for (int i = 0; i < space && run->hand_count < BO_MAX_HAND; i++) {
        run->hand[run->hand_count++] = run->deck[--run->deck_count];
    }
    bo_run_sort_hand(run, run->sort_by_suit);
}

int bo_pack_open(BORun *run, BOCenterId center, int cost) {
    const BOCenterInfo *pack = bo_center(center);
    if (!pack || pack->set != BO_SET_BOOSTER) return 0;
    run->pack_return = run->state;
    run->state = BO_STATE_PACK;
    run->pack = center;
    int has = 0;
    double choose = bo_center_number(pack, BO_F_CHOOSE, &has);
    run->pack_choices = has ? (int)choose : 1;
    if (cost > 0) bo_run_ease_dollars(run, -cost);
    int size = (int)pack->extra;
    if (size > 5) size = 5;
    // The jokers see the open pack before its cards come, in the same event.
    BOItem opened;
    memset(&opened, 0, sizeof(opened));
    opened.center = center;
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_OPEN_BOOSTER);
        ctx.card = &opened;
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    run->pack_count = 0;
    for (int i = 0; i < size; i++) run->pack_cards[run->pack_count++] = pack_card(run, pack, i);
    for (; run->pending_hallucinations > 0; run->pending_hallucinations--) {
        BOItem tarot = bo_run_create_item(run, BO_SET_TAROT, 0, 0, 0, BO_CENTER_NONE, "hal", 0);
        bo_run_add_consumable(run, &tarot);
        run->consumeable_buffer = 0;
    }
    if (kind_is(pack, "Arcana") || kind_is(pack, "Spectral")) draw_hand(run);
    return 1;
}

int bo_pack_can_choose(const BORun *run, int index, uint64_t highlighted) {
    if (run->state != BO_STATE_PACK || index < 0 || index >= run->pack_count) return 0;
    const BOShopCard *card = &run->pack_cards[index];
    if (card->is_playing_card) return 1;
    const BOCenterInfo *c = bo_item_center(&card->item);
    if (c->set == BO_SET_JOKER) {
        // G.FUNCS.can_select_card.
        return card->item.edition == BO_EDITION_NEGATIVE || run->joker_count < run->joker_slots;
    }
    if (!bo_consume_can_use(run, &card->item, highlighted, BO_USE_FROM_PACK)) return 0;
    return !(card->item.center == BO_C_ANKH && run->joker_count >= run->joker_slots);
}

// G.FUNCS.end_consumeable: the hand goes back under the deck, the cards
// that are left in the pack go away, and the run goes back to its state.
static void close_pack(BORun *run) {
    BOCard hand[BO_MAX_HAND];
    int count = run->hand_count;
    memcpy(hand, run->hand, sizeof(BOCard) * count);
    run->hand_count = 0;
    // Each card of the hand goes to the bottom of the deck in turn, so the
    // last card of the hand is at the bottom.
    memmove(&run->deck[count], run->deck, sizeof(BOCard) * run->deck_count);
    for (int i = 0; i < count; i++) run->deck[i] = hand[count - 1 - i];
    run->deck_count += count;
    int left = run->pack_count;
    run->pack_count = 0;
    for (int i = 0; i < left; i++) {
        if (!run->pack_cards[i].is_playing_card) bo_run_forget_card(run, run->pack_cards[i].item.center);
    }
    run->state = run->pack_return;
    run->pack = BO_CENTER_NONE;
    // end_consumeable: the next tag of the blind select can act.
    bo_tag_new_blind_choice(run);
}

int bo_pack_choose(BORun *run, int index, uint64_t highlighted) {
    if (!bo_pack_can_choose(run, index, highlighted)) return 0;
    BOShopCard chosen = run->pack_cards[index];
    memmove(&run->pack_cards[index], &run->pack_cards[index + 1],
            sizeof(BOShopCard) * (run->pack_count - index - 1));
    run->pack_count--;
    const BOCenterInfo *c = bo_item_center(&chosen.item);
    if (chosen.is_playing_card) {
        // The card goes to the bottom of the deck.
        if (run->deck_count < BO_MAX_CARDS) {
            memmove(&run->deck[1], run->deck, sizeof(BOCard) * run->deck_count);
            run->deck[0] = chosen.card;
            run->deck_count++;
        }
        bo_rules_playing_cards_added(run, 1);
    } else if (c->set == BO_SET_JOKER) {
        bo_run_add_joker(run, &chosen.item);
    } else {
        bo_consume_use(run, &chosen.item, highlighted);
        bo_run_forget_card(run, chosen.item.center);
        bo_run_remove_marked_jokers(run);
        bo_profile_discover(run, chosen.item.center);
        // A card that a Tarot or a Planet pack uses.
        const char *kind = bo_center(run->pack)->kind;
        if (kind && strcmp(kind, "Arcana") == 0) bo_profile_stat(run, BO_STAT_TAROT_READING_USED, 1);
        if (kind && strcmp(kind, "Celestial") == 0) bo_profile_stat(run, BO_STAT_PLANETARIUM_USED, 1);
    }
    bo_profile_check_cards(run);
    if (run->pack_choices > 1) {
        run->pack_choices--;
    } else {
        close_pack(run);
    }
    return 1;
}

void bo_pack_skip(BORun *run) {
    if (run->state != BO_STATE_PACK) return;
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = context_for(BO_CTX_SKIPPING_BOOSTER);
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    close_pack(run);
}

int bo_pack_can_skip(const BORun *run) {
    // G.FUNCS.can_skip_booster: an Arcana or a Spectral pack waits until
    // the hand is there.
    return run->state == BO_STATE_PACK && run->pack_count > 0;
}
