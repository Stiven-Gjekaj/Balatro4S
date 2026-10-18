// bo_run: the state of one run, and its steps: select a blind, play,
// discard, and cash out. The screen calls these steps. The order of the
// random numbers is the order of Balatro 1.0.1o, so a seed gives the same
// cards as in the original.

#ifndef BO_RUN_H
#define BO_RUN_H

#include "bo_blind.h"
#include "bo_card.h"
#include "bo_random.h"
#include "bo_score.h"

#define BO_MAX_CARDS 256
#define BO_MAX_HAND 64

typedef enum {
    BO_STATE_BLIND_SELECT,
    BO_STATE_SELECTING,   // A blind is on. The player selects cards.
    BO_STATE_ROUND_EVAL,  // The blind is beaten. The cash out waits.
    BO_STATE_GAME_OVER,
} BORunState;

typedef enum {
    BO_SLOT_SMALL,
    BO_SLOT_BIG,
    BO_SLOT_BOSS,
} BOBlindSlot;

typedef struct {
    int blind;     // The reward of the blind.
    int hands;     // $1 for each hand that is left.
    int interest;  // $1 for each $5, up to the cap.
    int total;
} BORoundEval;

typedef struct {
    BORandom random;
    BORunState state;
    int stake;
    int win_ante;
    int won;         // The boss of the win ante is beaten.
    int ante;
    int round;
    BOBlindSlot slot;  // The blind on deck.
    BOBlindId boss;    // The boss of this ante.
    BOBlindId blind;   // The blind that is on, or that was on last.
    double blind_chips;
    int blind_dollars;
    double chips;      // The score of this round.
    int hands_left;
    int discards_left;
    int hands_per_round;
    int discards_per_round;
    int hand_size;
    int dollars;
    int interest_cap;
    int bosses_used[BO_BLIND_COUNT];
    int hands_played[BO_HAND_COUNT];  // For each poker hand, in this run.
    BOScoreContext context;            // The levels of the hands.
    int sort_by_suit;
    uint32_t next_uid;
    BORoundEval eval;

    // The deck. The top is the last card.
    BOCard deck[BO_MAX_CARDS];
    int deck_count;
    BOCard hand[BO_MAX_HAND];
    int hand_count;
    BOCard discard[BO_MAX_CARDS];
    int discard_count;
} BORun;

// Starts a run of the Red Deck at stake 1 with seed, for example
// "ABCDEFGH".
void bo_run_start(BORun *run, const char *seed);

// The blind of a slot in this ante.
BOBlindId bo_run_slot_blind(const BORun *run, BOBlindSlot slot);

// The chip target of the blind of a slot in this ante.
double bo_run_slot_chips(const BORun *run, BOBlindSlot slot);

// Starts the blind on deck: the deck shuffles, and the hand fills.
void bo_run_select_blind(BORun *run);

// Plays the cards of the hand that mask selects (bit i is hand[i]). score
// gets the result. Returns 0 when the play is not allowed.
int bo_run_play(BORun *run, uint64_t mask, BOScore *score);

// Discards the cards that mask selects. Returns 0 when it is not allowed.
int bo_run_discard(BORun *run, uint64_t mask);

// Takes the money of the round and goes to the next blind.
void bo_run_cash_out(BORun *run);

// Sorts the hand by rank, or by suit, from high to low.
void bo_run_sort_hand(BORun *run, int by_suit);

#endif
