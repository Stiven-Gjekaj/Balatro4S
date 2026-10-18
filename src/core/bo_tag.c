#include "bo_tag.h"

#include <stdio.h>
#include <string.h>

#include "bo_joker.h"
#include "bo_pack.h"
#include "bo_pool.h"
#include "bo_rules.h"

#pragma STDC FP_CONTRACT OFF

#define MAX_TAGS ((int)(sizeof(((BORun *)0)->tags) / sizeof(((BORun *)0)->tags[0])))

// config.type of a tag, as "immediate". The data keeps it in hand_type.
static int tag_type_is(BOCenterId center, const char *type) {
    const char *t = bo_center(center)->hand_type;
    return t && strcmp(t, type) == 0;
}

static double tag_number(BOCenterId center, BOField field) {
    return bo_center_number(bo_center(center), field, NULL);
}

// Tag:remove, after the yep of a tag: the triggered tags leave the list.
static void remove_triggered(BORun *run) {
    int kept = 0;
    for (int i = 0; i < run->tag_count; i++) {
        if (!run->tags[i].triggered) run->tags[kept++] = run->tags[i];
    }
    run->tag_count = kept;
}

void bo_tag_orbital_choices(BORun *run) {
    if (run->orbital_ante == run->ante) return;
    run->orbital_ante = run->ante;
    // pairs(G.GAME.hands) over the visible hands. LuaJIT gives another
    // order in each process; Balatro4S uses one fixed order.
    int visible[BO_HAND_COUNT], count = 0;
    for (int i = 0; i < BO_HAND_COUNT; i++) {
        int type = bo_rules_hands_pairs_order(i);
        if (run->hands[type].visible) visible[count++] = type;
    }
    for (int slot = 0; slot < 3; slot++) {
        run->orbital_choices[slot] =
            count ? visible[bo_random_index(&run->random, bo_pseudoseed(&run->random, "orbital"), count)] : BO_HAND_HIGH_CARD;
    }
}

static void add_one(BORun *run, BOCenterId center, int orbital_hand) {
    if (run->tag_count >= MAX_TAGS) return;
    BOTag *tag = &run->tags[run->tag_count++];
    memset(tag, 0, sizeof(*tag));
    tag->center = center;
    tag->orbital_hand = orbital_hand;
}

void bo_tag_add(BORun *run, BOCenterId center, int blind_type) {
    // Tag:set_ability: the hand of an Orbital Tag.
    int orbital = BO_HAND_HIGH_CARD;
    if (center == BO_TAG_ORBITAL && blind_type >= 0) {
        bo_tag_orbital_choices(run);
        orbital = run->orbital_choices[blind_type];
    }
    // tag_add: each Double Tag copies a new tag that is not a Double Tag.
    int copies = 0;
    if (center != BO_TAG_DOUBLE) {
        for (int i = 0; i < run->tag_count; i++) {
            if (run->tags[i].center == BO_TAG_DOUBLE && !run->tags[i].triggered) {
                run->tags[i].triggered = 1;
                copies++;
            }
        }
    }
    add_one(run, center, orbital);
    for (int i = 0; i < copies; i++) add_one(run, center, orbital);
    remove_triggered(run);
}

void bo_tag_immediate(BORun *run) {
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || !tag_type_is(tag->center, "immediate")) continue;
        switch (tag->center) {
        case BO_TAG_TOP_UP:
            for (int n = 0; n < (int)tag_number(tag->center, BO_F_SPAWN_JOKERS); n++) {
                if (run->joker_count < run->joker_slots) {
                    BOItem joker = bo_run_create_item(run, BO_SET_JOKER, 1, 0, 0, BO_CENTER_NONE, "top", 0);
                    bo_run_add_joker(run, &joker);
                }
            }
            break;
        case BO_TAG_SKIP:
            bo_run_ease_dollars(run, run->skips * (int)tag_number(tag->center, BO_F_SKIP_BONUS));
            break;
        case BO_TAG_GARBAGE:
            bo_run_ease_dollars(run, run->unused_discards * (int)tag_number(tag->center, BO_F_DOLLARS_PER_DISCARD));
            break;
        case BO_TAG_HANDY:
            bo_run_ease_dollars(run, run->hands_played_total * (int)tag_number(tag->center, BO_F_DOLLARS_PER_HAND));
            break;
        case BO_TAG_ECONOMY: {
            int max = (int)tag_number(tag->center, BO_F_MAX);
            int dollars = run->dollars > 0 ? run->dollars : 0;
            bo_run_ease_dollars(run, dollars < max ? dollars : max);
            break;
        }
        case BO_TAG_ORBITAL:
            bo_run_level_up_hand(run, (BOHandType)tag->orbital_hand, (int)tag_number(tag->center, BO_F_LEVELS));
            break;
        default: continue;
        }
        tag->triggered = 1;
    }
    remove_triggered(run);
}

int bo_tag_new_blind_choice(BORun *run) {
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || !tag_type_is(tag->center, "new_blind_choice")) continue;
        BOCenterId pack = BO_CENTER_NONE;
        switch (tag->center) {
        case BO_TAG_CHARM: pack = BO_P_ARCANA_MEGA_1; break;
        case BO_TAG_METEOR: pack = BO_P_CELESTIAL_MEGA_1; break;
        case BO_TAG_ETHEREAL: pack = BO_P_SPECTRAL_NORMAL_1; break;
        case BO_TAG_STANDARD: pack = BO_P_STANDARD_MEGA_1; break;
        case BO_TAG_BUFFOON: pack = BO_P_BUFFOON_MEGA_1; break;
        case BO_TAG_BOSS: break;
        default: continue;
        }
        tag->triggered = 1;
        remove_triggered(run);
        if (pack != BO_CENTER_NONE) {
            // A free pack. The next tag waits until the pack closes.
            bo_pack_open(run, pack, 0);
            return 1;
        }
        // G.FUNCS.reroll_boss from the tag: free, then the next tag.
        run->boss_rerolled = 1;
        run->boss = bo_blind_new_boss(&run->random, run->ante, run->win_ante, run->bosses_used, NULL);
        bo_tag_new_blind_choice(run);
        return 1;
    }
    return 0;
}

void bo_tag_round_start_bonus(BORun *run) {
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || tag->center != BO_TAG_JUGGLE) continue;
        int size = (int)bo_center(tag->center)->h_size;
        bo_rules_change_hand_size(run, size);
        run->temp_handsize += size;
        tag->triggered = 1;
    }
    remove_triggered(run);
}

int bo_tag_eval(BORun *run) {
    int dollars = 0;
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || tag->center != BO_TAG_INVESTMENT) continue;
        if (!bo_blind_info(run->blind)->boss) continue;
        dollars += (int)tag_number(tag->center, BO_F_DOLLARS);
        tag->triggered = 1;
    }
    remove_triggered(run);
    return dollars;
}

void bo_tag_shop_start(BORun *run) {
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || tag->center != BO_TAG_D_SIX || run->shop_d6ed) continue;
        run->shop_d6ed = 1;
        run->temp_reroll_cost = 0;
        bo_rules_calculate_reroll_cost(run, 1);
        tag->triggered = 1;
    }
    remove_triggered(run);
}

int bo_tag_store_joker_create(BORun *run, BOShopCard *out) {
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || !tag_type_is(tag->center, "store_joker_create")) continue;
        tag->triggered = 1;
        if (tag->center == BO_TAG_RARE) {
            // NOPE when the player has each rare joker.
            int owned = 0;
            uint8_t seen[BO_CENTER_COUNT];
            memset(seen, 0, sizeof(seen));
            for (int j = 0; j < run->joker_count; j++) {
                const BOCenterInfo *c = bo_item_center(&run->jokers[j]);
                if (c->rarity == 3 && !seen[run->jokers[j].center]) {
                    seen[run->jokers[j].center] = 1;
                    owned++;
                }
            }
            int rares = 0;
            for (int c = 0; c < BO_CENTER_COUNT; c++) {
                if (bo_center((BOCenterId)c)->set == BO_SET_JOKER && bo_center((BOCenterId)c)->rarity == 3) rares++;
            }
            if (rares <= owned) {
                remove_triggered(run);
                continue;
            }
        }
        memset(out, 0, sizeof(*out));
        int rare = tag->center == BO_TAG_RARE;
        out->item = bo_run_create_item(run, BO_SET_JOKER, rare ? 3 : 2, 0, 0, BO_CENTER_NONE, rare ? "rta" : "uta", 1);
        out->item.couponed = 1;
        bo_rules_set_cost(run, &out->item);
        remove_triggered(run);
        bo_tag_store_joker_modify(run, &out->item);
        return 1;
    }
    return 0;
}

void bo_tag_store_joker_modify(BORun *run, BOItem *card) {
    if (card->edition != BO_EDITION_NONE || bo_item_center(card)->set != BO_SET_JOKER) return;
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || !tag_type_is(tag->center, "store_joker_modify")) continue;
        int edition = BO_EDITION_NONE;
        switch (tag->center) {
        case BO_TAG_FOIL: edition = BO_EDITION_FOIL; break;
        case BO_TAG_HOLO: edition = BO_EDITION_HOLO; break;
        case BO_TAG_POLYCHROME: edition = BO_EDITION_POLYCHROME; break;
        case BO_TAG_NEGATIVE: edition = BO_EDITION_NEGATIVE; break;
        default: break;
        }
        tag->triggered = 1;
        if (edition != BO_EDITION_NONE) {
            card->edition = (uint8_t)edition;
            card->couponed = 1;
            bo_rules_set_cost(run, card);
        }
        break;
    }
    remove_triggered(run);
}

void bo_tag_voucher_add(BORun *run) {
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || tag->center != BO_TAG_VOUCHER) continue;
        tag->triggered = 1;
        if (run->tag_voucher_count < (int)(sizeof(run->tag_vouchers) / sizeof(run->tag_vouchers[0]))) {
            BOCenterId voucher = bo_pool_next_voucher(run, 1);
            run->tag_vouchers[run->tag_voucher_count++] = bo_pool_make_item(run, voucher);
        }
    }
    remove_triggered(run);
}

void bo_tag_shop_final_pass(BORun *run) {
    for (int i = 0; i < run->tag_count; i++) {
        BOTag *tag = &run->tags[i];
        if (tag->triggered || tag->center != BO_TAG_COUPON || run->shop_free) continue;
        run->shop_free = 1;
        for (int j = 0; j < run->shop_count; j++) {
            run->shop[j].item.couponed = 1;
            bo_rules_set_cost(run, &run->shop[j].item);
        }
        for (int j = 0; j < run->shop_pack_count; j++) {
            run->shop_packs[j].couponed = 1;
            bo_rules_set_cost(run, &run->shop_packs[j]);
        }
        tag->triggered = 1;
    }
    remove_triggered(run);
}

int bo_run_can_skip_blind(const BORun *run) {
    return run->state == BO_STATE_BLIND_SELECT && run->slot != BO_SLOT_BOSS;
}

int bo_run_skip_blind(BORun *run) {
    if (!bo_run_can_skip_blind(run)) return 0;
    run->skips++;
    int slot = run->slot;
    run->skipped_slots |= 1 << slot;
    bo_tag_add(run, run->blind_tags[slot], slot);
    run->slot = slot == BO_SLOT_SMALL ? BO_SLOT_BIG : BO_SLOT_BOSS;
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.kind = BO_CTX_SKIP_BLIND;
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    bo_tag_immediate(run);
    bo_tag_new_blind_choice(run);
    return 1;
}
