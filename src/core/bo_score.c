#include "bo_score.h"

#include <string.h>

#pragma STDC FP_CONTRACT OFF

void bo_score_context_init(BOScoreContext *context) {
    memset(context, 0, sizeof(*context));
    for (int i = 0; i < BO_HAND_COUNT; i++) context->levels[i] = 1;
}

double bo_score_cap_chips(const BOScore *score, double chips) {
    if (!score->chips_cap) return chips;
    double cap = score->cap_dollars > 0 ? score->cap_dollars : 0;
    return chips < cap ? chips : cap;
}

void bo_score_add_event(BOScore *score, BOEventKind kind, BOArea area, int index, double value) {
    switch (kind) {
    case BO_EVENT_CHIPS: score->chips += value; break;
    case BO_EVENT_MULT: score->mult += value; break;
    case BO_EVENT_XMULT: score->mult *= value; break;
    case BO_EVENT_DOLLARS: score->dollars += (int)value; break;
    case BO_EVENT_SWAP: {
        double t = score->chips;
        score->chips = score->mult;
        score->mult = t;
        break;
    }
    default: break;
    }
    // mod_chips: the cap of Rich get Richer.
    if (score->chips_cap) score->chips = bo_score_cap_chips(score, score->chips);
    if (score->event_count == BO_MAX_EVENTS) return;
    BOScoreEvent *event = &score->events[score->event_count++];
    event->kind = (uint8_t)kind;
    event->area = (uint8_t)area;
    event->index = (uint8_t)index;
    event->value = value;
    event->chips = score->chips;
    event->mult = score->mult;
}
