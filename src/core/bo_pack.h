// bo_pack: the booster packs, as Card:open, G.FUNCS.use_card in a pack,
// G.FUNCS.skip_booster, and G.FUNCS.end_consumeable of Balatro 1.0.1o.

#ifndef BO_PACK_H
#define BO_PACK_H

#include <stdint.h>

#include "bo_run.h"

// Opens a pack: the run goes to BO_STATE_PACK and the cards of the pack
// come. cost is the price to pay, or 0 for a pack from a tag.
int bo_pack_open(BORun *run, BOCenterId center, int cost);

// The card at index of the open pack can be taken or used now.
// highlighted selects the cards of the hand for a Tarot or Spectral card.
int bo_pack_can_choose(const BORun *run, int index, uint64_t highlighted);

// Takes or uses card index of the pack. The pack closes after the last
// choice. Returns 0 when the card cannot be chosen.
int bo_pack_choose(BORun *run, int index, uint64_t highlighted);

// Closes the pack with no more choices.
int bo_pack_can_skip(const BORun *run);
void bo_pack_skip(BORun *run);

#endif
