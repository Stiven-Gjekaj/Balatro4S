// bo_score: scores a played hand, as G.FUNCS.evaluate_play does in
// Balatro 1.0.1o. It gives the final chips and mult, and a list of events
// in the order of the original, so that the screen can show each step.

#ifndef BO_SCORE_H
#define BO_SCORE_H

#include "bo_card.h"
#include "bo_hand.h"

typedef enum {
    BO_EVENT_CHIPS,   // value chips were added.
    BO_EVENT_MULT,    // value mult was added.
    BO_EVENT_XMULT,   // The mult was multiplied by value.
    BO_EVENT_DOLLARS, // value dollars were earned.
} BOEventKind;

typedef enum {
    BO_AREA_PLAY,   // A played card.
    BO_AREA_HAND,   // A card held in the hand.
    BO_AREA_JOKER,  // A joker.
} BOArea;

typedef struct {
    uint8_t kind;
    uint8_t area;
    uint8_t index;  // The index of the card in its area.
    double value;
    double chips;   // The chips after this event.
    double mult;    // The mult after this event.
} BOScoreEvent;

#define BO_MAX_EVENTS 256

typedef struct {
    BOHandType type;
    int royal;
    uint8_t scoring;  // A bit for each played card that scores.
    double base_chips;
    double base_mult;
    double chips;
    double mult;
    double total;     // math.floor(chips * mult).
    int dollars;
    int event_count;
    BOScoreEvent events[BO_MAX_EVENTS];
} BOScore;

// The state of the run that the score reads.
typedef struct {
    int levels[BO_HAND_COUNT];  // The level of each hand. 1 is the start.
    BOHandRules rules;
} BOScoreContext;

void bo_score_context_init(BOScoreContext *context);

// Scores the played cards. held is the cards that stay in the hand.
void bo_score_play(const BOScoreContext *context, const BOCard *played, int played_count,
                   const BOCard *held, int held_count, BOScore *out);

#endif
