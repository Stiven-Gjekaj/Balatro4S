// bo_consume: the use of a Tarot, a Planet, or a Spectral card, as
// Card:use_consumeable and Card:can_use_consumeable of Balatro 1.0.1o.

#ifndef BO_CONSUME_H
#define BO_CONSUME_H

#include <stdint.h>

#include "bo_item.h"

struct BORun;

// Where the card is when it is used.
typedef enum {
    BO_USE_FROM_CONSUMABLES,  // The consumable slots.
    BO_USE_FROM_PACK,         // An open booster pack.
} BOUseFrom;

// Card:can_use_consumeable. highlighted selects the cards of the hand:
// bit i is hand[i]. The state of the run tells if a hand is on.
int bo_consume_can_use(const struct BORun *run, const BOItem *item, uint64_t highlighted, BOUseFrom from);

// G.FUNCS.use_card for a consumable that already left its area:
// use_consumeable, then the using_consumeable context of the jokers.
// highlighted selects the cards of the hand.
void bo_consume_use(struct BORun *run, const BOItem *item, uint64_t highlighted);

// Card:update for the money that Temperance gives.
int bo_consume_temperance_money(const struct BORun *run, const BOItem *item);

#endif
