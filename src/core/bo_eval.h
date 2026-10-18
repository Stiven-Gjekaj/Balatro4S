// bo_eval: the score of a played hand, a port of G.FUNCS.evaluate_play of
// Balatro 1.0.1o, with the jokers, the seals, the editions, and the
// enhancements.

#ifndef BO_EVAL_H
#define BO_EVAL_H

#include "bo_hand.h"
#include "bo_score.h"

struct BORun;

// The rules of the hand that the jokers change: Four Fingers, Shortcut,
// and Smeared Joker.
BOHandRules bo_eval_rules(const struct BORun *run);

// evaluate_play for the cards in run->play. It changes the run (the round
// score, the money, the jokers) and fills score.
void bo_eval_play(struct BORun *run, BOScore *score);

// The parts of the boss blind and of the deck in the score.
int bo_eval_blind_debuffs_hand(struct BORun *run, const BOCard *played, int count, const BOHandResult *hands,
                               BOHandType text);
void bo_eval_blind_modify_hand(struct BORun *run, BOScore *score);
void bo_eval_deck_final_step(struct BORun *run, BOScore *score);

// The count of played cards that the play destroyed.
int bo_eval_count_destroyed(const struct BORun *run);

#endif
