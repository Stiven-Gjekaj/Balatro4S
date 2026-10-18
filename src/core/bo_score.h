// bo_score: the result of a played hand, as G.FUNCS.evaluate_play gives
// it in Balatro 1.0.1o: the final chips and mult, and a list of events in
// the order of the original, so that the screen can show each step.

#ifndef BO_SCORE_H
#define BO_SCORE_H

#include "bo_card.h"
#include "bo_hand.h"

typedef enum {
    BO_EVENT_CHIPS,     // value chips were added.
    BO_EVENT_MULT,      // value mult was added.
    BO_EVENT_XMULT,     // The mult was multiplied by value.
    BO_EVENT_DOLLARS,   // value dollars were earned.
    BO_EVENT_AGAIN,     // The card scores again ("Again!").
    BO_EVENT_MESSAGE,   // A joker changed, as "Upgrade!".
    BO_EVENT_LEVEL_UP,  // The hand went up one level.
    BO_EVENT_SWAP,      // The chips and the mult changed places.
    BO_EVENT_DEBUFFED,  // The card is debuffed and does not score.
    BO_EVENT_BALANCED,  // Plasma Deck: the chips and the mult are the same.
} BOEventKind;

typedef enum {
    BO_AREA_PLAY,        // A played card.
    BO_AREA_HAND,        // A card held in the hand.
    BO_AREA_JOKER,       // A joker.
    BO_AREA_CONSUMABLE,  // A consumable (Observatory).
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
    int debuffed;     // The boss blind does not allow the hand.
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

// The rules of the hands that the jokers change.
typedef struct {
    int levels[BO_HAND_COUNT];
    BOHandRules rules;
} BOScoreContext;

void bo_score_context_init(BOScoreContext *context);

// Adds an event and changes the chips, the mult, or the dollars of score.
void bo_score_add_event(BOScore *score, BOEventKind kind, BOArea area, int index, double value);

#endif
