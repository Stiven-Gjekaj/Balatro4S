#include "bo_score.h"

#include <math.h>
#include <string.h>

void bo_score_context_init(BOScoreContext *context) {
    memset(context, 0, sizeof(*context));
    for (int i = 0; i < BO_HAND_COUNT; i++) context->levels[i] = 1;
}

static void add_event(BOScore *score, BOEventKind kind, BOArea area, int index, double value) {
    switch (kind) {
    case BO_EVENT_CHIPS: score->chips += value; break;
    case BO_EVENT_MULT: score->mult += value; break;
    case BO_EVENT_XMULT: score->mult *= value; break;
    case BO_EVENT_DOLLARS: score->dollars += (int)value; break;
    }
    if (score->event_count == BO_MAX_EVENTS) return;
    BOScoreEvent *event = &score->events[score->event_count++];
    event->kind = (uint8_t)kind;
    event->area = (uint8_t)area;
    event->index = (uint8_t)index;
    event->value = value;
    event->chips = score->chips;
    event->mult = score->mult;
}

// eval_card for a played card: the chips of the card (get_chip_bonus).
static void score_played_card(BOScore *score, const BOCard *card, int index) {
    if (card->debuffed) return;
    int chips = bo_card_is_stone(card) ? 50 : bo_card_nominal(card);
    chips += card->perma_bonus;
    if (chips) add_event(score, BO_EVENT_CHIPS, BO_AREA_PLAY, index, chips);
}

void bo_score_play(const BOScoreContext *context, const BOCard *played, int played_count,
                   const BOCard *held, int held_count, BOScore *out) {
    (void)held;
    (void)held_count;
    memset(out, 0, sizeof(*out));
    if (played_count > BO_MAX_PLAY) played_count = BO_MAX_PLAY;
    BOHandResult result = bo_hand_evaluate(played, played_count, &context->rules);
    out->type = result.type;
    out->royal = result.royal;
    if (result.type == BO_HAND_NONE) return;
    out->scoring = bo_hand_scoring_cards(&result, played, played_count, 0);

    int level = context->levels[result.type];
    out->base_chips = out->chips = bo_hand_level_chips(result.type, level);
    out->base_mult = out->mult = bo_hand_level_mult(result.type, level);

    // The scoring cards go from left to right, in the order of play.
    for (int i = 0; i < played_count; i++) {
        if ((out->scoring >> i) & 1) score_played_card(out, &played[i], i);
    }
    out->total = floor(out->chips * out->mult);
}
