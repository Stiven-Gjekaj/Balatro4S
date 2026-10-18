// bo_item: a joker, a consumable, a voucher, or a pack: a card with a
// center and an ability, as Card:set_ability makes it in Balatro 1.0.1o.

#ifndef BO_ITEM_H
#define BO_ITEM_H

#include <stdint.h>
#include "bo_card.h"
#include "bo_data.h"

#define BO_ABILITY_FIELDS 12

// The ability table of a card. The standard fields have their own place.
// The other config fields, as config.extra.odds, are in values.
typedef struct {
    double mult, h_mult, h_x_mult, h_dollars, p_dollars, t_mult, t_chips, x_mult, h_size, d_size, bonus;
    double extra;        // config.extra when it is a number.
    double extra_value;  // The sell value that Gift Card and Egg add.
    int count;
    uint8_t fields[BO_ABILITY_FIELDS];
    double values[BO_ABILITY_FIELDS];
    // The special fields of some jokers.
    int to_do_hand;       // To Do List: a BOHandType.
    int invis_rounds;     // Invisible Joker.
    double caino_xmult;   // Canio.
    int yorick_discards;  // Yorick.
    int loyalty_remaining;
    int burnt_hand;
    int hands_played_at_create;
    // The tallies that Card:update counts each frame. They do not change in
    // the middle of a play.
    int steel_tally, stone_tally, driver_tally, nine_tally;
} BOAbility;

typedef struct {
    BOCenterId center;
    uint32_t sort_id;   // The order of creation, for the sorts of Balatro.
    uint32_t id;        // The node ID. unique_val is 1 - id / 1603301.
    uint8_t edition;    // A BOEdition.
    uint8_t eternal;
    uint8_t perishable;
    uint8_t perish_tally;
    uint8_t rental;
    uint8_t pinned;
    uint8_t debuffed;
    uint8_t getting_sliced;
    uint8_t couponed;   // Coupon Tag: the card costs $0 in this shop.
    int base_cost;
    int extra_cost;
    int cost;
    int sell_cost;
    BOAbility ability;
} BOItem;

// Sets the ability of item from its center, as set_ability does. The
// special fields that need the random state (To Do List) are left to the run.
void bo_item_set_ability(BOItem *item, BOCenterId center, int hands_played);

// Gets and sets a config field of the ability. get gives 0 when the field
// is not there.
double bo_item_get(const BOItem *item, BOField field);
void bo_item_set(BOItem *item, BOField field, double value);
int bo_item_has(const BOItem *item, BOField field);

// set_cost: the cost and the sell value, with the inflation and the
// discount of the run.
void bo_item_set_cost(BOItem *item, int inflation, int discount_percent, int astronomer, int booster_ante_scaling,
                      int ante);

const BOCenterInfo *bo_item_center(const BOItem *item);

#endif
