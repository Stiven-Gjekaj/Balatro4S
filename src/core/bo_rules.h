// bo_rules: the small rules of Balatro 1.0.1o that the jokers, the shop,
// and the scoring share: the id, the face, and the suit of a card, the
// counts of the deck, the costs, and the size of the hand.

#ifndef BO_RULES_H
#define BO_RULES_H

#include "bo_card.h"
#include "bo_data.h"
#include "bo_hand.h"
#include "bo_item.h"

struct BORun;

// The hand at place i of pairs(G.GAME.hands) in LuaJIT. To Do List picks
// from the hands in this order.
int bo_rules_hands_pairs_order(int i);

// "Pair" gives BO_HAND_PAIR. Returns -1 for an unknown name.
int bo_rules_hand_by_name(const char *name);
// "Hearts" gives BO_HEARTS. Returns -1 for an unknown name.
int bo_rules_suit_by_name(const char *name);

// Card:get_id: the rank, or a negative number for a Stone card that no
// rank matches.
int bo_rules_get_id(const BOCard *card);
// Card:is_face.
int bo_rules_is_face(const struct BORun *run, const BOCard *card, int from_boss);
// Card:is_suit.
int bo_rules_is_suit(const struct BORun *run, const BOCard *card, int suit, int bypass_debuff, int flush_calc);

// The center of the enhancement of a playing card, as BO_M_GLASS.
BOCenterId bo_rules_enhancement_center(int enhancement);
int bo_rules_enhancement_of(BOCenterId center);

// set_ability of a playing card with a new enhancement.
void bo_rules_set_enhancement(struct BORun *run, BOCard *card, int enhancement);

// The counts over G.playing_cards: the deck, the hand, the discard pile,
// and the cards in play.
int bo_rules_playing_card_count(const struct BORun *run);
int bo_rules_count_enhancement(const struct BORun *run, int enhancement);
int bo_rules_count_enhanced(const struct BORun *run);
int bo_rules_count_rank(const struct BORun *run, int rank);

// G.hand:change_size.
void bo_rules_change_hand_size(struct BORun *run, int delta);

// Card:set_cost with the state of the run.
void bo_rules_set_cost(struct BORun *run, BOItem *item);
void bo_rules_set_all_costs(struct BORun *run);
void bo_rules_calculate_reroll_cost(struct BORun *run, int skip_increment);

// copy_card for a joker or a consumable.
BOItem bo_rules_copy_item(struct BORun *run, const BOItem *other, int strip_edition);

// Blind:disable.
void bo_rules_disable_blind(struct BORun *run);
// add_tag.
void bo_rules_add_tag(struct BORun *run, BOCenterId tag);

// Certificate: a new card with a seal in the hand.
void bo_rules_certificate(struct BORun *run);
// Marble Joker: a new Stone card in the deck.
void bo_rules_add_stone_card(struct BORun *run);
// DNA: a copy of the played card in the hand.
void bo_rules_dna_copy(struct BORun *run, const BOCard *card);

// The effect of new playing cards on the jokers (Hologram).
void bo_rules_playing_cards_added(struct BORun *run, int count);

// pseudorandom_element of G.P_CARDS: a card front from the sorted keys.
BOCard bo_rules_random_front(struct BORun *run, const char *key);

// Adds a playing card to the run with a new sort_id.
BOCard bo_rules_new_playing_card(struct BORun *run, int rank, int suit);

#endif
