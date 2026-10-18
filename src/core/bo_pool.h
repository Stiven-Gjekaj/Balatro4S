// bo_pool: the pools of cards and the creation of cards, as
// get_current_pool and create_card of Balatro 1.0.1o.

#ifndef BO_POOL_H
#define BO_POOL_H

#include "bo_data.h"
#include "bo_item.h"
#include "bo_run.h"

// A pool type that is not a set: the Tarot and Planet cards together.
#define BO_POOL_TAROT_PLANET 100

// get_current_pool: fills pool with each card of the type in its order, or
// BO_CENTER_NONE for a card that is not available. available gets the
// count of the cards that are. Returns the size of the pool.
int bo_pool_build(const struct BORun *run, int type, int rarity, BOCenterId *pool, int *available);

// poll_edition. Returns a BOEdition.
int bo_pool_poll_edition(struct BORun *run, const char *key, double mod, int no_negative, int guaranteed);

// The Soul or Black Hole in place of a normal card, as create_card with
// soulable.
BOCenterId bo_pool_forced_soul(struct BORun *run, int type);

// create_card_for_shop.
BOShopCard bo_pool_create_shop_card(struct BORun *run);

// get_next_voucher_key and get_next_tag_key.
BOCenterId bo_pool_next_voucher(struct BORun *run, int from_tag);
BOCenterId bo_pool_next_tag(struct BORun *run, const char *append);

// get_pack: a booster pack. key_base is as "shop_pack". kind is NULL or as
// "Arcana".
BOCenterId bo_pool_get_pack(struct BORun *run, const char *key_base, const char *kind);

// A card with a center, as Card() makes it, with its sort_id and its cost.
BOItem bo_pool_make_item(struct BORun *run, BOCenterId center);

#endif
