// bo_tag: the tags, as Tag:apply_to_run and add_tag of Balatro 1.0.1o,
// and the skip of a blind.

#ifndef BO_TAG_H
#define BO_TAG_H

#include "bo_run.h"

// The type of the blind that gives a tag, as the blind_type of Tag:init.
enum {
    BO_TAG_FROM_NONE = -1,
    BO_TAG_FROM_SMALL = 0,
    BO_TAG_FROM_BIG = 1,
};

// add_tag(Tag(key, nil, blind_type)): Double Tag copies the new tag.
void bo_tag_add(BORun *run, BOCenterId center, int blind_type);

// The orbital_choices of the ante: the hand of an Orbital Tag for the
// Small, the Big, and the Boss blind. The blind select makes them.
void bo_tag_orbital_choices(BORun *run);

// The contexts of Tag:apply_to_run.
void bo_tag_immediate(BORun *run);
// The first tag that acts: a free pack or a new boss. Returns 1 when one
// acted.
int bo_tag_new_blind_choice(BORun *run);
void bo_tag_round_start_bonus(BORun *run);
// Investment Tag after a boss: each tag gives a row of money. Fills the
// dollars and the place of each tag in run->tags, and returns the count.
int bo_tag_eval(BORun *run, int *dollars, int *places, int max);
void bo_tag_shop_start(BORun *run);
// Rare Tag and Uncommon Tag make the card. Returns 1 when a tag made it.
int bo_tag_store_joker_create(BORun *run, BOShopCard *out);
void bo_tag_store_joker_modify(BORun *run, BOItem *card);
void bo_tag_voucher_add(BORun *run);
void bo_tag_shop_final_pass(BORun *run);

// G.FUNCS.skip_blind: the tag of the blind, and the next blind.
int bo_run_can_skip_blind(const BORun *run);
int bo_run_skip_blind(BORun *run);

#endif
