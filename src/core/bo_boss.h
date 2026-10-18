// bo_boss: the effects of the blinds, as the Blind object of Balatro
// 1.0.1o (blind.lua): set_blind, debuff_card, press_play, debuff_hand,
// modify_hand, drawn_to_hand, stay_flipped, disable, and defeat.

#ifndef BO_BOSS_H
#define BO_BOSS_H

#include "bo_card.h"
#include "bo_hand.h"

struct BORun;

// Blind:set_blind for the blind in run->blind, at the start of a round.
void bo_boss_set_blind(struct BORun *run);

// Blind:set_blind(nil, true): the debuffs of the playing cards again, as
// after a joker comes or goes.
void bo_boss_reset_debuffs(struct BORun *run);

// Blind:debuff_card for one playing card.
void bo_boss_debuff_card(struct BORun *run, BOCard *card);

// Blind:press_play: the effects when a hand is played. The Hook discards
// two cards of the hand. Returns 1 when the blind acted.
int bo_boss_press_play(struct BORun *run);

// Blind:debuff_hand. With check, the blind only tells if the hand is not
// allowed, as for the text of the selected hand.
int bo_boss_debuff_hand(struct BORun *run, int count, const BOHandResult *hands, BOHandType text, int check);

// Blind:modify_hand: The Flint halves the base chips and mult.
int bo_boss_modify_hand(struct BORun *run, double *mult, double *chips);

// Blind:stay_flipped: the card comes into the hand face down.
int bo_boss_stay_flipped(struct BORun *run, const BOCard *card);

// Blind:drawn_to_hand: after each draw.
void bo_boss_drawn_to_hand(struct BORun *run);

// The Serpent: the number of cards that the next draw takes, or -1 for
// the normal draw.
int bo_boss_draw_count(const struct BORun *run);

// Blind:disable and Blind:defeat.
void bo_boss_disable(struct BORun *run);
void bo_boss_defeat(struct BORun *run);

#endif
