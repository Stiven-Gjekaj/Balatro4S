#include "bo_item.h"

#include <math.h>
#include <string.h>

#pragma STDC FP_CONTRACT OFF

const BOCenterInfo *bo_item_center(const BOItem *item) {
    return bo_center(item->center);
}

void bo_item_set_ability(BOItem *item, BOCenterId center, int hands_played) {
    const BOCenterInfo *c = bo_center(center);
    item->center = center;
    BOAbility *a = &item->ability;
    double extraValue = a->extra_value;
    memset(a, 0, sizeof(*a));
    a->extra_value = extraValue;
    if (!c) return;
    a->mult = c->mult;
    a->h_mult = c->h_mult;
    a->h_x_mult = c->h_x_mult;
    a->h_dollars = c->h_dollars;
    a->p_dollars = c->p_dollars;
    a->t_mult = c->t_mult;
    a->t_chips = c->t_chips;
    a->x_mult = c->x_mult;
    a->h_size = c->h_size;
    a->d_size = c->d_size;
    a->bonus = c->bonus;
    a->extra = c->extra;
    for (int i = 0; i < c->config_count && a->count < BO_ABILITY_FIELDS; i++) {
        a->fields[a->count] = c->config[i].field;
        a->values[a->count] = c->config[i].number;
        a->count++;
    }
    if (center == BO_J_INVISIBLE) a->invis_rounds = 0;
    if (center == BO_J_CAINO) a->caino_xmult = 1;
    if (center == BO_J_YORICK) a->yorick_discards = (int)bo_item_get(item, BO_F_EXTRA_DISCARDS);
    if (center == BO_J_LOYALTY_CARD) {
        a->burnt_hand = 0;
        a->loyalty_remaining = (int)bo_item_get(item, BO_F_EXTRA_EVERY);
    }
    item->base_cost = c->cost ? c->cost : 1;
    a->hands_played_at_create = hands_played;
}

static int index_of(const BOAbility *a, BOField field) {
    for (int i = 0; i < a->count; i++) {
        if (a->fields[i] == field) return i;
    }
    return -1;
}

double bo_item_get(const BOItem *item, BOField field) {
    int i = index_of(&item->ability, field);
    return i < 0 ? 0 : item->ability.values[i];
}

int bo_item_has(const BOItem *item, BOField field) {
    return index_of(&item->ability, field) >= 0;
}

void bo_item_set(BOItem *item, BOField field, double value) {
    BOAbility *a = &item->ability;
    int i = index_of(a, field);
    if (i < 0) {
        if (a->count == BO_ABILITY_FIELDS) return;
        i = a->count++;
        a->fields[i] = (uint8_t)field;
    }
    a->values[i] = value;
}

void bo_item_set_cost(BOItem *item, int inflation, int discount_percent, int astronomer, int booster_ante_scaling,
                      int ante) {
    const BOCenterInfo *c = bo_item_center(item);
    item->extra_cost = inflation;
    switch (item->edition) {
    case BO_EDITION_HOLO: item->extra_cost += 3; break;
    case BO_EDITION_FOIL: item->extra_cost += 2; break;
    case BO_EDITION_POLYCHROME: item->extra_cost += 5; break;
    case BO_EDITION_NEGATIVE: item->extra_cost += 5; break;
    }
    double cost = floor((item->base_cost + item->extra_cost + 0.5) * (100 - discount_percent) / 100);
    item->cost = cost < 1 ? 1 : (int)cost;
    if (c && c->set == BO_SET_BOOSTER && booster_ante_scaling) item->cost += ante - 1;
    int celestial = c && c->set == BO_SET_BOOSTER && c->kind && strcmp(c->kind, "Celestial") == 0;
    if (c && (c->set == BO_SET_PLANET || celestial) && astronomer) item->cost = 0;
    if (item->rental) item->cost = 1;
    int sell = item->cost / 2;
    item->sell_cost = (sell < 1 ? 1 : sell) + (int)item->ability.extra_value;
    if (item->couponed) item->cost = 0;
}
