// The pools of cards and create_card, a port of get_current_pool,
// poll_edition, create_card, create_card_for_shop, get_next_voucher_key,
// get_next_tag_key, and get_pack of Balatro 1.0.1o.

#include "bo_pool.h"

#include <stdio.h>
#include <string.h>

#include "bo_joker.h"
#include "bo_rules.h"
#include "bo_tag.h"
#include "bo_run.h"

#pragma STDC FP_CONTRACT OFF

static const char *set_name(int set) {
    switch (set) {
    case BO_SET_JOKER: return "Joker";
    case BO_SET_TAROT: return "Tarot";
    case BO_SET_PLANET: return "Planet";
    case BO_SET_SPECTRAL: return "Spectral";
    case BO_SET_VOUCHER: return "Voucher";
    case BO_SET_ENHANCED: return "Enhanced";
    case BO_SET_TAG: return "Tag";
    case BO_POOL_TAROT_PLANET: return "Tarot_Planet";
    default: return "";
    }
}

static int owned_or_shown(const BORun *run, BOCenterId id) {
    return run->used_jokers[id] && bo_run_find_joker(run, BO_J_RING_MASTER) == 0;
}

static int deck_has_center(const BORun *run, const char *gate) {
    BOCenterId center = bo_center_by_key(gate);
    int enhancement = bo_rules_enhancement_of(center);
    return bo_rules_count_enhancement(run, enhancement) > 0;
}

int bo_pool_build(const BORun *run, int type, int rarity, BOCenterId *pool, int *available) {
    int count = 0;
    *available = 0;
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        const BOCenterInfo *c = bo_center((BOCenterId)i);
        int in_pool;
        if (type == BO_SET_JOKER) {
            in_pool = c->set == BO_SET_JOKER && c->rarity == rarity;
        } else if (type == BO_POOL_TAROT_PLANET) {
            in_pool = (c->set == BO_SET_TAROT || c->set == BO_SET_PLANET) && !c->omit;
        } else {
            in_pool = c->set == type && !c->omit;
        }
        if (!in_pool) continue;
        int add = 0;
        if (type == BO_SET_ENHANCED) {
            add = 1;
        } else if (type == BO_SET_TAG) {
            add = c->min_ante == 0 || c->min_ante <= run->ante;
        } else if (!owned_or_shown(run, (BOCenterId)i)) {
            if (c->set == BO_SET_VOUCHER) {
                if (!run->used_vouchers[i]) {
                    int include = 1;
                    const char *requires = bo_center_text(c, BO_F_REQUIRES_1);
                    if (requires && !run->used_vouchers[bo_center_by_key(requires)]) include = 0;
                    if (run->has_shop_voucher && run->shop_voucher.center == (BOCenterId)i) include = 0;
                    for (int t = 0; t < run->tag_voucher_count; t++) {
                        if (run->tag_vouchers[t].center == (BOCenterId)i) include = 0;
                    }
                    add = include;
                }
            } else if (c->set == BO_SET_PLANET) {
                int has = 0;
                double softlock = bo_center_number(c, BO_F_SOFTLOCK, &has);
                const char *hand = bo_center_text(c, BO_F_HAND_TYPE);
                int type_of_hand = bo_rules_hand_by_name(hand);
                if (!(has && softlock) || (type_of_hand >= 0 && run->hands[type_of_hand].played > 0)) add = 1;
            } else if (c->enhancement_gate) {
                add = deck_has_center(run, c->enhancement_gate);
            } else {
                add = 1;
            }
            if (i == BO_C_BLACK_HOLE || i == BO_C_SOUL) add = 0;
        }
        if (c->no_pool_flag && run->gros_michel_extinct) add = 0;
        if (c->yes_pool_flag && !run->gros_michel_extinct) add = 0;
        pool[count++] = add ? (BOCenterId)i : BO_CENTER_NONE;
        if (add) (*available)++;
    }
    if (*available == 0) {
        count = 0;
        BOCenterId fallback = BO_J_JOKER;
        if (type == BO_SET_TAROT || type == BO_POOL_TAROT_PLANET) fallback = BO_C_STRENGTH;
        else if (type == BO_SET_PLANET) fallback = BO_C_PLUTO;
        else if (type == BO_SET_SPECTRAL) fallback = BO_C_INCANTATION;
        else if (type == BO_SET_VOUCHER) fallback = BO_V_BLANK;
        else if (type == BO_SET_TAG) fallback = BO_TAG_HANDY;
        pool[count++] = fallback;
        *available = 1;
    }
    return count;
}

// Picks from the pool with the key, and again with "_resample2", ... when
// the pick is not available.
static BOCenterId pick(BORun *run, const BOCenterId *pool, int count, const char *key) {
    BOCenterId center = pool[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), count)];
    for (int it = 2; center == BO_CENTER_NONE && it < 10000; it++) {
        char again[96];
        snprintf(again, sizeof(again), "%s_resample%d", key, it);
        center = pool[bo_random_index(&run->random, bo_pseudoseed(&run->random, again), count)];
    }
    return center;
}

// get_current_pool: the key of the pool, as "Joker1sho3" or "Tarotsho3".
static BOCenterId draw_from_pool(BORun *run, int type, int rarity, int legendary, const char *append) {
    char key[64];
    if (type == BO_SET_JOKER) {
        if (rarity == 0) {
            char rkey[48];
            snprintf(rkey, sizeof(rkey), "rarity%d%s", run->ante, append ? append : "");
            double roll = bo_pseudorandom(&run->random, rkey);
            rarity = roll > 0.95 ? 3 : roll > 0.7 ? 2 : 1;
        }
        if (legendary) rarity = 4;
        if (legendary) {
            snprintf(key, sizeof(key), "Joker4");
        } else {
            snprintf(key, sizeof(key), "Joker%d%s%d", rarity, append ? append : "", run->ante);
        }
    } else {
        snprintf(key, sizeof(key), "%s%s%d", set_name(type), append ? append : "", run->ante);
    }
    static BOCenterId pool[BO_CENTER_COUNT];
    int available = 0;
    int count = bo_pool_build(run, type, rarity, pool, &available);
    return pick(run, pool, count, key);
}

int bo_pool_poll_edition(BORun *run, const char *key, double mod, int no_negative, int guaranteed) {
    double poll = bo_pseudorandom(&run->random, key ? key : "edition_generic");
    if (guaranteed) {
        if (poll > 1 - 0.003 * 25 && !no_negative) return BO_EDITION_NEGATIVE;
        if (poll > 1 - 0.006 * 25) return BO_EDITION_POLYCHROME;
        if (poll > 1 - 0.02 * 25) return BO_EDITION_HOLO;
        if (poll > 1 - 0.04 * 25) return BO_EDITION_FOIL;
        return BO_EDITION_NONE;
    }
    if (poll > 1 - 0.003 * mod && !no_negative) return BO_EDITION_NEGATIVE;
    if (poll > 1 - 0.006 * run->edition_rate * mod) return BO_EDITION_POLYCHROME;
    if (poll > 1 - 0.02 * run->edition_rate * mod) return BO_EDITION_HOLO;
    if (poll > 1 - 0.04 * run->edition_rate * mod) return BO_EDITION_FOIL;
    return BO_EDITION_NONE;
}

// Card:init and set_ability: a new card with a center.
static BOItem make_item(BORun *run, BOCenterId center) {
    BOItem item;
    memset(&item, 0, sizeof(item));
    item.sort_id = bo_run_new_sort_id(run);
    item.id = bo_run_new_id(run);
    bo_item_set_ability(&item, center, run->hands_played_total);
    if (center == BO_J_TODO_LIST) bo_joker_set_to_do(run, &item, 0);
    // set_ability marks the card as used, by its name.
    run->used_jokers[center] = 1;
    bo_rules_set_cost(run, &item);
    return item;
}

BOCenterId bo_pool_forced_soul(BORun *run, int type) {
    BOCenterId forced = BO_CENTER_NONE;
    char key[48];
    if (type == BO_SET_TAROT || type == BO_SET_SPECTRAL || type == BO_POOL_TAROT_PLANET) {
        if (!owned_or_shown(run, BO_C_SOUL)) {
            snprintf(key, sizeof(key), "soul_%s%d", set_name(type), run->ante);
            if (bo_pseudorandom(&run->random, key) > 0.997) forced = BO_C_SOUL;
        }
    }
    if (type == BO_SET_PLANET || type == BO_SET_SPECTRAL) {
        if (!owned_or_shown(run, BO_C_BLACK_HOLE)) {
            snprintf(key, sizeof(key), "soul_%s%d", set_name(type), run->ante);
            if (bo_pseudorandom(&run->random, key) > 0.997) forced = BO_C_BLACK_HOLE;
        }
    }
    return forced;
}

BOItem bo_run_create_item(BORun *run, int type, int rarity, int legendary, int soulable, BOCenterId forced,
                          const char *key_append, int in_shop) {
    if (forced == BO_CENTER_NONE && soulable) forced = bo_pool_forced_soul(run, type);
    BOCenterId center = forced != BO_CENTER_NONE ? forced : draw_from_pool(run, type, rarity, legendary, key_append);
    BOItem item = make_item(run, center);
    if (bo_center(center)->set == BO_SET_JOKER) {
        if (in_shop) {
            char key[32];
            snprintf(key, sizeof(key), "%s%d", in_shop == 2 ? "packetper" : "etperpoll", run->ante);
            double poll = bo_pseudorandom(&run->random, key);
            const BOCenterInfo *c = bo_center(center);
            // set_eternal and set_perishable keep out the jokers that cannot
            // have the sticker.
            if (run->stake >= 4 && poll > 0.7) {
                if (c->eternal_compat && !item.perishable) item.eternal = 1;
            } else if (run->stake >= 7 && poll > 0.4 && poll <= 0.7) {
                if (c->perishable_compat && !item.eternal) {
                    item.perishable = 1;
                    item.perish_tally = 5;
                }
            }
            if (run->stake >= 8) {
                snprintf(key, sizeof(key), "%s%d", in_shop == 2 ? "packssjr" : "ssjr", run->ante);
                if (bo_pseudorandom(&run->random, key) > 0.7) item.rental = 1;
            }
        }
        char key[48];
        snprintf(key, sizeof(key), "edi%s%d", key_append ? key_append : "", run->ante);
        item.edition = (uint8_t)bo_pool_poll_edition(run, key, 1, 0, 0);
        bo_rules_set_cost(run, &item);
    }
    return item;
}

BOShopCard bo_pool_create_shop_card(BORun *run) {
    BOShopCard shop;
    memset(&shop, 0, sizeof(shop));
    // Rare Tag and Uncommon Tag make the card in place of the shop.
    if (bo_tag_store_joker_create(run, &shop)) return shop;
    double total = run->joker_rate + run->tarot_rate + run->planet_rate + run->playing_card_rate + run->spectral_rate;
    char key[32];
    snprintf(key, sizeof(key), "cdt%d", run->ante);
    double polled = bo_pseudorandom(&run->random, key) * total;
    double check = 0;
    struct {
        int type;
        double rate;
    } kinds[5] = {
        {BO_SET_JOKER, run->joker_rate},
        {BO_SET_TAROT, run->tarot_rate},
        {BO_SET_PLANET, run->planet_rate},
        {-1, run->playing_card_rate},
        {BO_SET_SPECTRAL, run->spectral_rate},
    };
    // The type of the playing card: Illusion polls "illusion" for each shop
    // card, when the table is made.
    if (run->used_vouchers[BO_V_ILLUSION] && bo_pseudorandom(&run->random, "illusion") > 0.6) {
        kinds[3].type = BO_SET_ENHANCED;
    }
    for (int k = 0; k < 5; k++) {
        if (polled > check && polled <= check + kinds[k].rate) {
            if (kinds[k].type == -1 || kinds[k].type == BO_SET_ENHANCED) {
                BOCenterId center = BO_C_BASE;
                if (kinds[k].type == BO_SET_ENHANCED) center = draw_from_pool(run, BO_SET_ENHANCED, 0, 0, "sho");
                char front[32];
                snprintf(front, sizeof(front), "frontsho%d", run->ante);
                BOCard card = bo_rules_random_front(run, front);
                // One Card() makes the card, so it has one sort_id.
                shop.item = make_item(run, center);
                card.uid = shop.item.sort_id;
                card.enhancement = (uint8_t)bo_rules_enhancement_of(center);
                shop.is_playing_card = 1;
                shop.card = card;
                if (run->used_vouchers[BO_V_ILLUSION] && bo_pseudorandom(&run->random, "illusion") > 0.8) {
                    double poll = bo_pseudorandom(&run->random, "illusion");
                    shop.card.edition = poll > 1 - 0.15 ? BO_EDITION_POLYCHROME : poll > 0.5 ? BO_EDITION_HOLO : BO_EDITION_FOIL;
                    shop.item.edition = shop.card.edition;
                    bo_rules_set_cost(run, &shop.item);
                }
            } else {
                shop.item = bo_run_create_item(run, kinds[k].type, 0, 0, 0, BO_CENTER_NONE, "sho", 1);
                bo_tag_store_joker_modify(run, &shop.item);
            }
            return shop;
        }
        check += kinds[k].rate;
    }
    // The original returns nil here. This only happens with all rates at 0.
    shop.item = bo_run_create_item(run, BO_SET_JOKER, 0, 0, 0, BO_CENTER_NONE, "sho", 1);
    return shop;
}

BOCenterId bo_pool_next_voucher(BORun *run, int from_tag) {
    static BOCenterId pool[BO_CENTER_COUNT];
    int available = 0;
    int count = bo_pool_build(run, BO_SET_VOUCHER, 0, pool, &available);
    char key[32];
    if (from_tag) {
        snprintf(key, sizeof(key), "Voucher_fromtag");
    } else {
        snprintf(key, sizeof(key), "Voucher%d", run->ante);
    }
    return pick(run, pool, count, key);
}

BOCenterId bo_pool_next_tag(BORun *run, const char *append) {
    static BOCenterId pool[BO_CENTER_COUNT];
    int available = 0;
    int count = bo_pool_build(run, BO_SET_TAG, 0, pool, &available);
    char key[48];
    snprintf(key, sizeof(key), "Tag%s%d", append ? append : "", run->ante);
    return pick(run, pool, count, key);
}

BOCenterId bo_pool_get_pack(BORun *run, const char *key_base, const char *kind) {
    if (!run->first_shop_buffoon) {
        run->first_shop_buffoon = 1;
        // The original takes p_buffoon_normal_1 or _2 with math.random. The
        // two packs differ only in their art.
        return BO_P_BUFFOON_NORMAL_1;
    }
    double cume = 0;
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        const BOCenterInfo *c = bo_center((BOCenterId)i);
        if (c->set != BO_SET_BOOSTER) continue;
        if (!kind || (c->kind && strcmp(kind, c->kind) == 0)) cume += c->weight ? c->weight : 1;
    }
    char key[48];
    snprintf(key, sizeof(key), "%s%d", key_base ? key_base : "pack_generic", run->ante);
    double poll = bo_pseudorandom(&run->random, key) * cume;
    double it = 0;
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        const BOCenterInfo *c = bo_center((BOCenterId)i);
        if (c->set != BO_SET_BOOSTER) continue;
        double weight = c->weight ? c->weight : 1;
        if (!kind || (c->kind && strcmp(kind, c->kind) == 0)) it += weight;
        if (it >= poll && it - weight <= poll) return (BOCenterId)i;
    }
    return BO_P_ARCANA_NORMAL_1;
}

BOItem bo_pool_make_item(BORun *run, BOCenterId center) {
    return make_item(run, center);
}
