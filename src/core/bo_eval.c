// The play and the discard of a hand: a port of play_cards_from_highlighted,
// G.FUNCS.evaluate_play, and discard_cards_from_highlighted of Balatro
// 1.0.1o.

#include "bo_eval.h"

#include <math.h>
#include <string.h>

#include "bo_boss.h"
#include "bo_joker.h"
#include "bo_rules.h"
#include "bo_run.h"

#pragma STDC FP_CONTRACT OFF

BOHandRules bo_eval_rules(const BORun *run) {
    BOHandRules rules;
    rules.four_fingers = bo_run_find_joker(run, BO_J_FOUR_FINGERS) > 0;
    rules.shortcut = bo_run_find_joker(run, BO_J_SHORTCUT) > 0;
    rules.smeared = bo_run_find_joker(run, BO_J_SMEARED) > 0;
    return rules;
}

static BOContext context_for(BOContextKind kind) {
    BOContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.kind = kind;
    return ctx;
}

// The values of a playing card, as get_chip_bonus, get_chip_mult, and the
// others read them from the ability of the card.
static const BOCenterInfo *card_center(const BOCard *card) {
    return bo_center(bo_rules_enhancement_center(card->enhancement));
}

static double chip_bonus(const BOCard *card) {
    if (card->debuffed) return 0;
    const BOCenterInfo *c = card_center(card);
    if (bo_card_is_stone(card)) return c->bonus + card->perma_bonus;
    return bo_card_nominal(card) + c->bonus + card->perma_bonus;
}

static double chip_mult(BORun *run, BOCard *card) {
    if (card->debuffed) return 0;
    const BOCenterInfo *c = card_center(card);
    if (card->enhancement == BO_ENH_LUCKY) {
        if (bo_pseudorandom(&run->random, "lucky_mult") < run->probabilities / 5) {
            card->lucky_trigger = 1;
            return c->mult;
        }
        return 0;
    }
    return c->mult;
}

static double chip_x_mult(const BOCard *card) {
    if (card->debuffed) return 0;
    double x = card_center(card)->x_mult;
    return x <= 1 ? 0 : x;
}

static int p_dollars(BORun *run, BOCard *card) {
    if (card->debuffed) return 0;
    int ret = 0;
    if (card->seal == BO_SEAL_GOLD) ret += 3;
    const BOCenterInfo *c = card_center(card);
    if (c->p_dollars > 0) {
        if (card->enhancement == BO_ENH_LUCKY) {
            if (bo_pseudorandom(&run->random, "lucky_money") < run->probabilities / 15) {
                card->lucky_trigger = 1;
                ret += (int)c->p_dollars;
            }
        } else {
            ret += (int)c->p_dollars;
        }
    }
    if (ret > 0) run->dollar_buffer += ret;
    return ret;
}

static double edition_chips(int edition) {
    return edition == BO_EDITION_FOIL ? bo_center(BO_E_FOIL)->extra : 0;
}

static double edition_mult(int edition) {
    return edition == BO_EDITION_HOLO ? bo_center(BO_E_HOLO)->extra : 0;
}

static double edition_x_mult(int edition) {
    return edition == BO_EDITION_POLYCHROME ? bo_center(BO_E_POLYCHROME)->extra : 0;
}

// One effect of the list of evaluate_play, in its order: chips, mult,
// p_dollars, dollars, extra, x_mult, and edition.
typedef struct {
    int from_card;  // The eval_card of the playing card.
    double chips, mult, x_mult, h_mult;
    int p_dollars, dollars;
    int has_chips, has_mult, has_x_mult, has_h_mult, has_edition;
    double edition_chips, edition_mult, edition_x_mult;
    int extra_mult_mod, extra_chip_mod;  // An extra table with a change.
    double mult_mod, chip_mod;
    int message;
    BOItem *source;
} BOPlayEffect;

static int joker_index(const BORun *run, const BOItem *item) {
    for (int i = 0; i < run->joker_count; i++) {
        if (&run->jokers[i] == item) return i;
    }
    return 0;
}

static void apply_effect(BORun *run, BOScore *score, const BOPlayEffect *e, int cardIndex) {
    BOArea area = BO_AREA_PLAY;
    if (e->has_chips) bo_score_add_event(score, BO_EVENT_CHIPS, area, cardIndex, e->chips);
    if (e->has_mult) bo_score_add_event(score, BO_EVENT_MULT, area, cardIndex, e->mult);
    if (e->p_dollars) {
        bo_run_ease_dollars(run, e->p_dollars);
        bo_score_add_event(score, BO_EVENT_DOLLARS, area, cardIndex, e->p_dollars);
    }
    if (e->dollars) {
        bo_run_ease_dollars(run, e->dollars);
        bo_score_add_event(score, BO_EVENT_DOLLARS, area, cardIndex, e->dollars);
    }
    if (e->extra_mult_mod) bo_score_add_event(score, BO_EVENT_MULT, area, cardIndex, e->mult_mod);
    if (e->extra_chip_mod) bo_score_add_event(score, BO_EVENT_CHIPS, area, cardIndex, e->chip_mod);
    if (e->message && !e->has_chips && !e->has_mult && !e->has_x_mult && e->source) {
        bo_score_add_event(score, BO_EVENT_MESSAGE, BO_AREA_JOKER, joker_index(run, e->source), 0);
    }
    if (e->has_x_mult) bo_score_add_event(score, BO_EVENT_XMULT, area, cardIndex, e->x_mult);
    if (e->has_edition) {
        if (e->edition_chips) bo_score_add_event(score, BO_EVENT_CHIPS, area, cardIndex, e->edition_chips);
        // The original adds the mult and multiplies by the x mult each time,
        // with 0 and 1 when the edition has none.
        if (e->edition_mult) bo_score_add_event(score, BO_EVENT_MULT, area, cardIndex, e->edition_mult);
        if (e->edition_x_mult) bo_score_add_event(score, BO_EVENT_XMULT, area, cardIndex, e->edition_x_mult);
    }
}

static BOPlayEffect from_joker(const BOEffect *j) {
    BOPlayEffect e;
    memset(&e, 0, sizeof(e));
    e.source = j->card;
    if (j->has_chips) {
        e.has_chips = 1;
        e.chips = j->chips;
    }
    if (j->has_mult) {
        e.has_mult = 1;
        e.mult = j->mult;
    }
    if (j->has_x_mult) {
        e.has_x_mult = 1;
        e.x_mult = j->x_mult;
    }
    if (j->has_h_mult) {
        e.has_h_mult = 1;
        e.h_mult = j->h_mult;
    }
    e.dollars = j->dollars;
    e.message = j->message || j->extra;
    return e;
}

// The scoring of one played card, with its repetitions.
static void score_played_card(BORun *run, BOScore *score, BOCard *card, int index, BOContext *base) {
    if (card->debuffed) {
        run->blind_triggered = 1;
        bo_score_add_event(score, BO_EVENT_DEBUFFED, BO_AREA_PLAY, index, 0);
        return;
    }
    int reps = 1;
    // The red seal.
    if (card->seal == BO_SEAL_RED) reps++;
    for (int j = 0; j < run->joker_count; j++) {
        BOContext ctx = *base;
        ctx.kind = BO_CTX_REPETITION;
        ctx.cardarea = BO_IN_PLAY;
        ctx.other_card = card;
        BOEffect e = bo_joker_calculate(run, &run->jokers[j], &ctx);
        if (e.any && e.repetitions > 0) reps += e.repetitions;
    }
    for (int r = 0; r < reps; r++) {
        if (r > 0) bo_score_add_event(score, BO_EVENT_AGAIN, BO_AREA_PLAY, index, 0);
        BOPlayEffect effects[BO_MAX_JOKERS + 1];
        int count = 0;
        // eval_card of the playing card.
        BOPlayEffect own;
        memset(&own, 0, sizeof(own));
        own.from_card = 1;
        double chips = chip_bonus(card);
        if (chips > 0) {
            own.has_chips = 1;
            own.chips = chips;
        }
        double mult = chip_mult(run, card);
        if (mult > 0) {
            own.has_mult = 1;
            own.mult = mult;
        }
        double x_mult = chip_x_mult(card);
        if (x_mult > 0) {
            own.has_x_mult = 1;
            own.x_mult = x_mult;
        }
        own.p_dollars = p_dollars(run, card);
        if (!card->debuffed && card->edition != BO_EDITION_NONE) {
            own.has_edition = 1;
            own.edition_chips = edition_chips(card->edition);
            own.edition_mult = edition_mult(card->edition);
            own.edition_x_mult = edition_x_mult(card->edition);
        }
        effects[count++] = own;
        for (int k = 0; k < run->joker_count; k++) {
            BOContext ctx = *base;
            ctx.kind = BO_CTX_INDIVIDUAL;
            ctx.cardarea = BO_IN_PLAY;
            ctx.other_card = card;
            BOEffect e = bo_joker_calculate(run, &run->jokers[k], &ctx);
            if (e.any) effects[count++] = from_joker(&e);
        }
        card->lucky_trigger = 0;
        for (int i = 0; i < count; i++) apply_effect(run, score, &effects[i], index);
    }
}

// The scoring of one card that stays in the hand, with its repetitions.
static void score_held_card(BORun *run, BOScore *score, BOCard *card, int index, BOContext *base) {
    int reps = 1;
    for (int j = 0; j < reps; j++) {
        if (j > 0) bo_score_add_event(score, BO_EVENT_AGAIN, BO_AREA_HAND, index, 0);
        BOPlayEffect effects[BO_MAX_JOKERS + 1];
        int count = 0;
        BOPlayEffect own;
        memset(&own, 0, sizeof(own));
        own.from_card = 1;
        int ownAny = 0;
        if (!card->debuffed) {
            const BOCenterInfo *c = card_center(card);
            if (c->h_mult > 0) {
                own.has_h_mult = 1;
                own.h_mult = c->h_mult;
                ownAny = 1;
            }
            if (c->h_x_mult > 0) {
                own.has_x_mult = 1;
                own.x_mult = c->h_x_mult;
                ownAny = 1;
            }
        }
        effects[count++] = own;
        for (int k = 0; k < run->joker_count; k++) {
            BOContext ctx = *base;
            ctx.kind = BO_CTX_INDIVIDUAL;
            ctx.cardarea = BO_IN_HAND;
            ctx.other_card = card;
            BOEffect e = bo_joker_calculate(run, &run->jokers[k], &ctx);
            if (e.any) effects[count++] = from_joker(&e);
        }
        if (j == 0) {
            int any = ownAny || count > 1;
            if (card->seal == BO_SEAL_RED && !card->debuffed && any) reps++;
            for (int k = 0; k < run->joker_count; k++) {
                BOContext ctx = *base;
                ctx.kind = BO_CTX_REPETITION;
                ctx.cardarea = BO_IN_HAND;
                ctx.other_card = card;
                ctx.card_effects_any = any;
                BOEffect e = bo_joker_calculate(run, &run->jokers[k], &ctx);
                if (e.any && e.repetitions > 0) reps += e.repetitions;
            }
        }
        for (int i = 0; i < count; i++) {
            const BOPlayEffect *e = &effects[i];
            if (e->dollars) {
                bo_run_ease_dollars(run, e->dollars);
                bo_score_add_event(score, BO_EVENT_DOLLARS, BO_AREA_HAND, index, e->dollars);
            }
            if (e->has_h_mult) bo_score_add_event(score, BO_EVENT_MULT, BO_AREA_HAND, index, e->h_mult);
            if (e->has_x_mult) bo_score_add_event(score, BO_EVENT_XMULT, BO_AREA_HAND, index, e->x_mult);
        }
    }
}

static void apply_joker_main(BORun *run, BOScore *score, BOArea area, int index, const BOEffect *e) {
    if (e->has_mult_mod) bo_score_add_event(score, BO_EVENT_MULT, area, index, e->mult_mod);
    if (e->has_chip_mod) bo_score_add_event(score, BO_EVENT_CHIPS, area, index, e->chip_mod);
    if (e->has_xmult_mod) bo_score_add_event(score, BO_EVENT_XMULT, area, index, e->xmult_mod);
    if (e->dollars) bo_score_add_event(score, BO_EVENT_DOLLARS, area, index, 0);
    if (!e->has_mult_mod && !e->has_chip_mod && !e->has_xmult_mod && e->message) {
        bo_score_add_event(score, BO_EVENT_MESSAGE, area, index, 0);
    }
    (void)run;
}

void bo_eval_play(BORun *run, BOScore *score) {
    memset(score, 0, sizeof(*score));
    // Card:update keeps some values of the jokers each frame.
    for (int i = 0; i < run->joker_count; i++) bo_joker_update(run, &run->jokers[i]);
    run->defer_dollars = 1;
    run->pending_dollars = 0;
    BOCard *played = run->play;
    int count = run->play_count;
    BOHandRules rules = bo_eval_rules(run);
    BOHandResult hands = bo_hand_evaluate(played, count, &rules);
    int splash = bo_run_find_joker(run, BO_J_SPLASH) > 0;
    uint8_t mask = bo_hand_scoring_cards(&hands, played, count, splash);
    BOHandType text = hands.type;
    score->type = text;
    score->royal = hands.royal;
    score->scoring = mask;

    BOCard *scoring[BO_MAX_PLAY + 8];
    int scoringIndex[BO_MAX_PLAY + 8];
    int scoringCount = 0;
    for (int i = 0; i < count; i++) {
        if ((mask >> i) & 1) {
            scoringIndex[scoringCount] = i;
            scoring[scoringCount++] = &played[i];
        }
    }

    run->hands[text].played++;
    run->hands[text].played_this_round++;
    run->last_hand_played = text;
    run->hands[text].visible = 1;

    BOContext base = context_for(BO_CTX_BEFORE);
    base.cardarea = BO_IN_JOKERS;
    base.full_hand = played;
    base.full_count = count;
    base.scoring = scoring;
    base.scoring_count = scoringCount;
    base.scoring_name = text;
    base.poker_hands = &hands;

    int debuffed = bo_eval_blind_debuffs_hand(run, played, count, &hands, text);
    score->debuffed = debuffed;
    if (!debuffed) {
        // The jokers before the score.
        for (int i = 0; i < run->joker_count; i++) {
            BOContext ctx = base;
            ctx.kind = BO_CTX_BEFORE;
            BOEffect e = bo_joker_calculate(run, &run->jokers[i], &ctx);
            if (e.any) {
                if (e.level_up) {
                    bo_run_level_up_hand(run, text, 1);
                    bo_score_add_event(score, BO_EVENT_LEVEL_UP, BO_AREA_JOKER, i, 1);
                } else if (e.message) {
                    bo_score_add_event(score, BO_EVENT_MESSAGE, BO_AREA_JOKER, i, 0);
                }
                if (e.playing_cards_created) bo_rules_playing_cards_added(run, 1);
            }
        }
        score->chips = score->base_chips = bo_hand_level_chips(text, run->hands[text].level);
        score->mult = score->base_mult = bo_hand_level_mult(text, run->hands[text].level);
        bo_eval_blind_modify_hand(run, score);

        for (int i = 0; i < scoringCount; i++) {
            score_played_card(run, score, scoring[i], scoringIndex[i], &base);
        }
        for (int i = 0; i < run->hand_count; i++) {
            score_held_card(run, score, &run->hand[i], i, &base);
        }
        // The jokers, then the consumables.
        int total = run->joker_count + run->consumable_count;
        for (int i = 0; i < total; i++) {
            int isJoker = i < run->joker_count;
            BOItem *card = isJoker ? &run->jokers[i] : &run->consumables[i - run->joker_count];
            BOArea area = isJoker ? BO_AREA_JOKER : BO_AREA_CONSUMABLE;
            int index = isJoker ? i : i - run->joker_count;
            int edition = card->debuffed ? BO_EDITION_NONE : card->edition;
            if (edition_chips(edition)) bo_score_add_event(score, BO_EVENT_CHIPS, area, index, edition_chips(edition));
            if (edition_mult(edition)) bo_score_add_event(score, BO_EVENT_MULT, area, index, edition_mult(edition));
            BOContext ctx = base;
            ctx.kind = BO_CTX_JOKER_MAIN;
            BOEffect e = bo_joker_calculate(run, card, &ctx);
            if (e.any) apply_joker_main(run, score, area, index, &e);
            for (int k = 0; k < run->joker_count; k++) {
                BOContext other = base;
                other.kind = BO_CTX_OTHER_JOKER;
                other.cardarea = 0;
                other.other_joker = card;
                BOEffect o = bo_joker_calculate(run, &run->jokers[k], &other);
                if (o.any) apply_joker_main(run, score, BO_AREA_JOKER, k, &o);
            }
            if (edition_x_mult(edition)) bo_score_add_event(score, BO_EVENT_XMULT, area, index, edition_x_mult(edition));
        }
        bo_eval_deck_final_step(run, score);

        // The cards that the play destroys: Sixth Sense and Glass Cards.
        int shattered = 0, faces = 0;
        for (int i = 0; i < scoringCount; i++) {
            BOCard *card = scoring[i];
            int destroyed = 0;
            for (int j = 0; j < run->joker_count; j++) {
                BOContext ctx = context_for(BO_CTX_DESTROYING_CARD);
                ctx.full_hand = played;
                ctx.full_count = count;
                ctx.other_card = card;
                BOEffect e = bo_joker_calculate(run, &run->jokers[j], &ctx);
                if (e.any && e.destroyed) {
                    destroyed = 1;
                    break;
                }
            }
            if (card->enhancement == BO_ENH_GLASS && !card->debuffed &&
                bo_pseudorandom(&run->random, "glass") < run->probabilities / bo_center(BO_M_GLASS)->extra) {
                destroyed = 1;
            }
            if (destroyed) {
                if (card->enhancement == BO_ENH_GLASS) {
                    card->shattered = 1;
                    shattered++;
                } else {
                    card->destroyed = 1;
                }
                if (bo_rules_is_face(run, card, 0)) faces++;
            }
        }
        if (shattered || faces || bo_eval_count_destroyed(run)) {
            for (int j = 0; j < run->joker_count; j++) {
                BOContext ctx = context_for(BO_CTX_REMOVE_PLAYING_CARDS);
                ctx.cardarea = BO_IN_JOKERS;
                ctx.glass_shattered = shattered;
                ctx.faces_removed = faces;
                bo_joker_calculate(run, &run->jokers[j], &ctx);
            }
        }
    } else {
        score->chips = 0;
        score->mult = 0;
        for (int i = 0; i < run->joker_count; i++) {
            BOContext ctx = base;
            ctx.kind = BO_CTX_DEBUFFED_HAND;
            BOEffect e = bo_joker_calculate(run, &run->jokers[i], &ctx);
            if (e.any && e.dollars) bo_score_add_event(score, BO_EVENT_DOLLARS, BO_AREA_JOKER, i, e.dollars);
        }
    }
    score->total = floor(score->chips * score->mult);
    run->chips += score->total;

    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx = base;
        ctx.kind = BO_CTX_AFTER;
        BOEffect e = bo_joker_calculate(run, &run->jokers[i], &ctx);
        if (e.any && e.message) bo_score_add_event(score, BO_EVENT_MESSAGE, BO_AREA_JOKER, i, 0);
    }
    run->dollar_buffer = 0;
    run->defer_dollars = 0;
    run->dollars += run->pending_dollars;
    run->pending_dollars = 0;
}

int bo_eval_count_destroyed(const BORun *run) {
    int count = 0;
    for (int i = 0; i < run->play_count; i++) count += run->play[i].destroyed || run->play[i].shattered;
    return count;
}

int bo_eval_blind_debuffs_hand(BORun *run, const BOCard *played, int count, const BOHandResult *hands,
                               BOHandType text) {
    (void)played;
    return bo_boss_debuff_hand(run, count, hands, text, 0);
}

void bo_eval_blind_modify_hand(BORun *run, BOScore *score) {
    bo_boss_modify_hand(run, &score->mult, &score->chips);
}

// The decks come in 0.8. Until then, the Red Deck has no last step.
void bo_eval_deck_final_step(BORun *run, BOScore *score) {
    (void)run;
    (void)score;
}
