// A port of Card:calculate_joker of card.lua in Balatro 1.0.1o. The order
// of the tests is the order of the original, because the first test that
// returns wins.

#include "bo_joker.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "bo_rules.h"
#include "bo_run.h"

#pragma STDC FP_CONTRACT OFF

#define X(field) bo_item_get(self, BO_F_EXTRA_##field)
#define SET_X(field, value) bo_item_set(self, BO_F_EXTRA_##field, (value))
#define IS(key) (self->center == BO_J_##key)
#define A (&self->ability)

static BOEffect none(void) {
    BOEffect e;
    memset(&e, 0, sizeof(e));
    return e;
}

static BOEffect message(BOItem *card) {
    BOEffect e = none();
    e.any = 1;
    e.message = 1;
    e.card = card;
    return e;
}

static BOEffect mult_mod(double value) {
    BOEffect e = none();
    e.any = 1;
    e.has_mult_mod = 1;
    e.mult_mod = value;
    return e;
}

static BOEffect chip_mod(double value) {
    BOEffect e = none();
    e.any = 1;
    e.has_chip_mod = 1;
    e.chip_mod = value;
    return e;
}

static BOEffect xmult_mod(double value) {
    BOEffect e = none();
    e.any = 1;
    e.has_xmult_mod = 1;
    e.xmult_mod = value;
    return e;
}

static BOEffect card_chips(BOItem *card, double chips) {
    BOEffect e = none();
    e.any = 1;
    e.has_chips = 1;
    e.chips = chips;
    e.card = card;
    return e;
}

static BOEffect card_mult(BOItem *card, double mult) {
    BOEffect e = none();
    e.any = 1;
    e.has_mult = 1;
    e.mult = mult;
    e.card = card;
    return e;
}

static BOEffect card_x_mult(BOItem *card, double x_mult) {
    BOEffect e = none();
    e.any = 1;
    e.has_x_mult = 1;
    e.x_mult = x_mult;
    e.card = card;
    return e;
}

static BOEffect card_dollars(BOItem *card, int dollars) {
    BOEffect e = none();
    e.any = 1;
    e.dollars = dollars;
    e.card = card;
    return e;
}

static BOEffect repeat(BOItem *card, int repetitions) {
    BOEffect e = none();
    e.any = 1;
    e.message = 1;
    e.repetitions = repetitions;
    e.card = card;
    return e;
}

static int consumable_room(BORun *run) {
    return run->consumable_count + run->consumeable_buffer < run->consumable_slots;
}

// G.E_MANAGER events that make a card in the consumable area. The original
// makes the card later, in an event. Here the card comes at once. The keys
// of the random state do not depend on the order.
static void make_consumable(BORun *run, int set, const char *append) {
    BOItem item = bo_run_create_item(run, set, 0, 0, 0, BO_CENTER_NONE, append, 0);
    bo_run_add_consumable(run, &item);
    run->consumeable_buffer = 0;
}

static int joker_index(const BORun *run, const BOItem *item) {
    for (int i = 0; i < run->joker_count; i++) {
        if (&run->jokers[i] == item) return i;
    }
    return -1;
}

// The jokers that a pseudorandom_element of jokers sorts by sort_id.
static int pick_by_sort_id(BORun *run, BOItem **list, int count, const char *key) {
    for (int i = 1; i < count; i++) {
        BOItem *t = list[i];
        int j = i - 1;
        while (j >= 0 && list[j]->sort_id > t->sort_id) {
            list[j + 1] = list[j];
            j--;
        }
        list[j + 1] = t;
    }
    return bo_random_index(&run->random, bo_pseudoseed(&run->random, key), count);
}

static int hand_visible(const BORun *run, int type) {
    return run->hands[type].visible;
}

// To Do List: a new hand from the visible hands, in the order that pairs()
// of LuaJIT gives for G.GAME.hands.
static void reset_to_do(BORun *run, BOItem *self, int skip_current, const char *key) {
    int list[BO_HAND_COUNT], count = 0;
    for (int i = 0; i < BO_HAND_COUNT; i++) {
        int type = bo_rules_hands_pairs_order(i);
        if (!hand_visible(run, type)) continue;
        if (skip_current && type == A->to_do_hand) continue;
        list[count++] = type;
    }
    if (count == 0) return;
    A->to_do_hand = list[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), count)];
}

void bo_joker_set_to_do(BORun *run, BOItem *self, int title) {
    // set_ability: pick a hand that is not the old hand, again and again.
    int old = A->to_do_hand;
    int list[BO_HAND_COUNT], count = 0;
    for (int i = 0; i < BO_HAND_COUNT; i++) {
        int type = bo_rules_hands_pairs_order(i);
        if (hand_visible(run, type)) list[count++] = type;
    }
    if (count == 0) return;
    const char *key = title ? "false_to_do" : "to_do";
    for (int guard = 0; guard < 1000; guard++) {
        int pick = list[bo_random_index(&run->random, bo_pseudoseed(&run->random, key), count)];
        if (pick != old || count == 1) {
            A->to_do_hand = pick;
            return;
        }
    }
}

static BOEffect calculate(BORun *run, BOItem *self, BOContext *ctx);

BOEffect bo_joker_calculate(BORun *run, BOItem *self, BOContext *ctx) {
    return calculate(run, self, ctx);
}

static BOEffect copy_other(BORun *run, BOItem *self, BOItem *other, BOContext *ctx) {
    if (!other || other == self) return none();
    int saved = ctx->blueprint;
    BOItem *savedCard = ctx->blueprint_card;
    ctx->blueprint = ctx->blueprint ? ctx->blueprint + 1 : 1;
    if (!ctx->blueprint_card) ctx->blueprint_card = self;
    if (ctx->blueprint > run->joker_count + 1) {
        ctx->blueprint = saved;
        ctx->blueprint_card = savedCard;
        return none();
    }
    BOEffect ret = calculate(run, other, ctx);
    // The original leaves context.blueprint set after the call. The callers
    // make a new context for each joker, so this does not change a result.
    if (ret.any) ret.card = ctx->blueprint_card ? ctx->blueprint_card : self;
    return ret;
}

static int is_face(BORun *run, const BOCard *card) {
    return bo_rules_is_face(run, card, 0);
}

static int is_suit(BORun *run, const BOCard *card, int suit) {
    return bo_rules_is_suit(run, card, suit, 0, 0);
}

static int get_id(const BOCard *card) {
    return bo_rules_get_id(card);
}

static BOEffect calculate(BORun *run, BOItem *self, BOContext *ctx) {
    if (self->debuffed) return none();
    const BOCenterInfo *center = bo_item_center(self);
    if (!center) return none();

    if (center->set == BO_SET_PLANET) {
        if (ctx->kind == BO_CTX_JOKER_MAIN && run->used_vouchers[BO_V_OBSERVATORY] &&
            center->hand_type && bo_rules_hand_by_name(center->hand_type) == ctx->scoring_name) {
            const BOCenterInfo *obs = bo_center(BO_V_OBSERVATORY);
            return xmult_mod(obs->extra);
        }
    }
    if (center->set != BO_SET_JOKER) return none();

    if (IS(BLUEPRINT)) {
        int me = joker_index(run, self);
        BOItem *other = me >= 0 && me + 1 < run->joker_count ? &run->jokers[me + 1] : NULL;
        if (other && other != self) {
            BOEffect ret = copy_other(run, self, other, ctx);
            if (ret.any || ctx->blueprint > run->joker_count + 1) return ret;
        }
    }
    if (IS(BRAINSTORM)) {
        BOItem *other = run->joker_count > 0 ? &run->jokers[0] : NULL;
        if (other && other != self) {
            BOEffect ret = copy_other(run, self, other, ctx);
            if (ret.any || ctx->blueprint > run->joker_count + 1) return ret;
        }
    }

    BOItem *shown = ctx->blueprint_card ? ctx->blueprint_card : self;

    switch (ctx->kind) {
    case BO_CTX_OPEN_BOOSTER:
        if (IS(HALLUCINATION) && consumable_room(run)) {
            char key[24];
            snprintf(key, sizeof(key), "halu%d", run->ante);
            if (bo_pseudorandom(&run->random, key) < run->probabilities / A->extra) {
                run->consumeable_buffer++;
                make_consumable(run, BO_SET_TAROT, "hal");
            }
        }
        return none();

    case BO_CTX_BUYING_CARD:
        return none();

    case BO_CTX_SELLING_SELF:
        if (IS(LUCHADOR)) {
            if (!run->blind_disabled && run->state == BO_STATE_SELECTING && bo_blind_info(run->blind)->boss) {
                bo_rules_disable_blind(run);
            }
        }
        if (IS(DIET_COLA)) bo_rules_add_tag(run, BO_TAG_DOUBLE);
        if (IS(INVISIBLE) && A->invis_rounds >= A->extra && !ctx->blueprint) {
            BOItem *jokers[BO_MAX_JOKERS];
            int count = 0;
            for (int i = 0; i < run->joker_count; i++) {
                if (&run->jokers[i] != self) jokers[count++] = &run->jokers[i];
            }
            if (count > 0 && run->joker_count <= run->joker_slots) {
                BOItem *chosen = jokers[pick_by_sort_id(run, jokers, count, "invisible")];
                BOItem copy = bo_rules_copy_item(run, chosen, chosen->edition == BO_EDITION_NEGATIVE);
                copy.ability.invis_rounds = 0;
                bo_run_add_joker(run, &copy);
            }
        }
        return none();

    case BO_CTX_SELLING_CARD:
        if (IS(CAMPFIRE) && !ctx->blueprint) A->x_mult += A->extra;
        return none();

    case BO_CTX_REROLL_SHOP:
        if (IS(FLASH) && !ctx->blueprint) A->mult += A->extra;
        return none();

    case BO_CTX_ENDING_SHOP:
        if (IS(PERKEO)) {
            if (run->consumable_count > 0) {
                BOItem *list[BO_MAX_CONSUMABLES];
                for (int i = 0; i < run->consumable_count; i++) list[i] = &run->consumables[i];
                BOItem *chosen = list[pick_by_sort_id(run, list, run->consumable_count, "perkeo")];
                BOItem copy = bo_rules_copy_item(run, chosen, 0);
                copy.edition = BO_EDITION_NEGATIVE;
                bo_run_add_consumable(run, &copy);
            }
        }
        return none();

    case BO_CTX_SKIP_BLIND:
        return none();

    case BO_CTX_SKIPPING_BOOSTER:
        if (IS(RED_CARD) && !ctx->blueprint) A->mult += A->extra;
        return none();

    case BO_CTX_PLAYING_CARD_ADDED:
        if (self->getting_sliced) return none();
        if (IS(HOLOGRAM) && !ctx->blueprint && ctx->full_count > 0) {
            A->x_mult += ctx->full_count * A->extra;
        }
        return none();

    case BO_CTX_FIRST_HAND_DRAWN:
        if (IS(CERTIFICATE)) bo_rules_certificate(run);
        return none();

    case BO_CTX_SETTING_BLIND:
        if (self->getting_sliced) return none();
        if (IS(CHICOT) && !ctx->blueprint && ctx->blind_boss) bo_rules_disable_blind(run);
        if (IS(MADNESS) && !ctx->blueprint && !ctx->blind_boss) {
            A->x_mult += A->extra;
            BOItem *list[BO_MAX_JOKERS];
            int count = 0;
            for (int i = 0; i < run->joker_count; i++) {
                BOItem *j = &run->jokers[i];
                if (j != self && !j->eternal && !j->getting_sliced) list[count++] = j;
            }
            if (count > 0) {
                BOItem *victim = list[pick_by_sort_id(run, list, count, "madness")];
                if (!shown->getting_sliced) victim->getting_sliced = 1;
            }
        }
        if (IS(BURGLAR) && !shown->getting_sliced) {
            run->discards_left -= run->discards_left;
            run->hands_left += (int)A->extra;
        }
        if (IS(RIFF_RAFF) && !shown->getting_sliced && run->joker_count + run->joker_buffer < run->joker_slots) {
            int make = run->joker_slots - (run->joker_count + run->joker_buffer);
            if (make > 2) make = 2;
            run->joker_buffer += make;
            for (int i = 0; i < make; i++) {
                BOItem joker = bo_run_create_item(run, BO_SET_JOKER, 1, 0, 0, BO_CENTER_NONE, "rif", 0);
                bo_run_add_joker(run, &joker);
                run->joker_buffer = 0;
            }
        }
        if (IS(CARTOMANCER) && !shown->getting_sliced && consumable_room(run)) {
            run->consumeable_buffer++;
            make_consumable(run, BO_SET_TAROT, "car");
        }
        if (IS(CEREMONIAL) && !ctx->blueprint) {
            int me = joker_index(run, self);
            if (me >= 0 && me + 1 < run->joker_count && !self->getting_sliced) {
                BOItem *next = &run->jokers[me + 1];
                if (!next->eternal && !next->getting_sliced) {
                    next->getting_sliced = 1;
                    A->mult += next->sell_cost * 2;
                }
            }
        }
        if (IS(MARBLE) && !shown->getting_sliced) bo_rules_add_stone_card(run);
        return none();

    case BO_CTX_DESTROYING_CARD:
        if (ctx->blueprint) return none();
        if (IS(SIXTH_SENSE) && ctx->full_count == 1 && get_id(&ctx->full_hand[0]) == 6 && run->hands_played == 0) {
            if (consumable_room(run)) {
                run->consumeable_buffer++;
                make_consumable(run, BO_SET_SPECTRAL, "sixth");
            }
            BOEffect e = none();
            e.any = 1;
            e.destroyed = 1;
            return e;
        }
        return none();

    case BO_CTX_CARDS_DESTROYED:
        if (IS(CAINO) && !ctx->blueprint) {
            if (ctx->faces_removed > 0) A->caino_xmult += ctx->faces_removed * A->extra;
            return none();
        }
        if (IS(GLASS) && !ctx->blueprint) {
            if (ctx->glass_shattered > 0) A->x_mult += A->extra * ctx->glass_shattered;
            return none();
        }
        return none();

    case BO_CTX_REMOVE_PLAYING_CARDS:
        if (IS(CAINO) && !ctx->blueprint) {
            if (ctx->faces_removed > 0) A->caino_xmult += ctx->faces_removed * A->extra;
            return none();
        }
        if (IS(GLASS) && !ctx->blueprint) {
            if (ctx->glass_shattered > 0) A->x_mult += A->extra * ctx->glass_shattered;
            return none();
        }
        return none();

    case BO_CTX_USING_CONSUMEABLE: {
        const BOCenterInfo *used = bo_item_center(ctx->consumeable);
        if (IS(GLASS) && !ctx->blueprint && ctx->consumeable->center == BO_C_HANGED_MAN) {
            if (ctx->highlighted_glass > 0) A->x_mult += A->extra * ctx->highlighted_glass;
            return none();
        }
        if (IS(CONSTELLATION) && !ctx->blueprint && used && used->set == BO_SET_PLANET) {
            A->x_mult += A->extra;
            return none();
        }
        return none();
    }

    case BO_CTX_DEBUFFED_HAND:
        if (IS(MATADOR) && run->blind_triggered) {
            bo_run_ease_dollars(run, (int)A->extra);
            BOEffect e = message(NULL);
            e.dollars = (int)A->extra;
            return e;
        }
        return none();

    case BO_CTX_PRE_DISCARD:
        if (IS(BURNT) && run->discards_used <= 0 && !ctx->hook) {
            bo_run_level_up_hand(run, ctx->scoring_name, 1);
        }
        return none();

    case BO_CTX_DISCARD:
        if (IS(RAMEN) && !ctx->blueprint) {
            if (A->x_mult - A->extra <= 1) {
                self->getting_sliced = 1;
                return message(NULL);
            }
            A->x_mult -= A->extra;
            return message(NULL);
        }
        if (IS(YORICK) && !ctx->blueprint) {
            if (A->yorick_discards <= 1) {
                A->yorick_discards = (int)X(DISCARDS);
                A->x_mult += X(XMULT_LC);
                return message(NULL);
            }
            A->yorick_discards--;
            return none();
        }
        if (IS(TRADING) && !ctx->blueprint && run->discards_used <= 0 && ctx->full_count == 1) {
            bo_run_ease_dollars(run, (int)A->extra);
            BOEffect e = message(self);
            e.remove = 1;
            return e;
        }
        if (IS(CASTLE) && !ctx->other_card->debuffed && is_suit(run, ctx->other_card, run->castle_suit) &&
            !ctx->blueprint) {
            SET_X(CHIPS, X(CHIPS) + X(CHIP_MOD));
            return message(self);
        }
        if (IS(MAIL) && !ctx->other_card->debuffed && get_id(ctx->other_card) == run->mail_rank) {
            bo_run_ease_dollars(run, (int)A->extra);
            return message(self);
        }
        if (IS(HIT_THE_ROAD) && !ctx->other_card->debuffed && get_id(ctx->other_card) == 11 && !ctx->blueprint) {
            A->x_mult += A->extra;
            return message(self);
        }
        if (IS(GREEN_JOKER) && !ctx->blueprint && ctx->other_card == &ctx->full_hand[ctx->full_count - 1]) {
            double prev = A->mult;
            A->mult = fmax(0, A->mult - X(DISCARD_SUB));
            if (A->mult != prev) return message(self);
        }
        if (IS(FACELESS) && ctx->other_card == &ctx->full_hand[ctx->full_count - 1]) {
            int faces = 0;
            for (int i = 0; i < ctx->full_count; i++) {
                if (is_face(run, &ctx->full_hand[i])) faces++;
            }
            if (faces >= X(FACES)) {
                bo_run_ease_dollars(run, (int)X(DOLLARS));
                return none();
            }
        }
        return none();

    case BO_CTX_END_OF_ROUND:
        if (ctx->individual) return none();
        if (ctx->repetition) {
            if (ctx->cardarea == BO_IN_HAND && IS(MIME) && ctx->card_effects_any) return repeat(self, (int)A->extra);
            return none();
        }
        if (ctx->blueprint) return none();
        if (IS(CAMPFIRE) && bo_blind_info(run->blind)->boss && A->x_mult > 1) {
            A->x_mult = 1;
            return message(NULL);
        }
        if (IS(ROCKET) && bo_blind_info(run->blind)->boss) {
            SET_X(DOLLARS, X(DOLLARS) + X(INCREASE));
            return message(NULL);
        }
        if (IS(TURTLE_BEAN)) {
            if (X(H_SIZE) - X(H_MOD) <= 0) {
                self->getting_sliced = 1;
                return message(NULL);
            }
            SET_X(H_SIZE, X(H_SIZE) - X(H_MOD));
            bo_rules_change_hand_size(run, -(int)X(H_MOD));
            return message(NULL);
        }
        if (IS(INVISIBLE)) {
            A->invis_rounds++;
            return message(NULL);
        }
        if (IS(POPCORN)) {
            if (A->mult - A->extra <= 0) {
                self->getting_sliced = 1;
                return message(NULL);
            }
            A->mult -= A->extra;
            return message(NULL);
        }
        if (IS(TODO_LIST)) {
            reset_to_do(run, self, 1, "to_do");
            return message(NULL);
        }
        if (IS(EGG)) {
            A->extra_value += A->extra;
            bo_rules_set_cost(run, self);
            return message(NULL);
        }
        if (IS(GIFT)) {
            for (int i = 0; i < run->joker_count; i++) {
                run->jokers[i].ability.extra_value += A->extra;
                bo_rules_set_cost(run, &run->jokers[i]);
            }
            for (int i = 0; i < run->consumable_count; i++) {
                run->consumables[i].ability.extra_value += A->extra;
                bo_rules_set_cost(run, &run->consumables[i]);
            }
            return message(NULL);
        }
        if (IS(HIT_THE_ROAD) && A->x_mult > 1) {
            A->x_mult = 1;
            return message(NULL);
        }
        if (IS(GROS_MICHEL) || IS(CAVENDISH)) {
            const char *key = IS(CAVENDISH) ? "cavendish" : "gros_michel";
            if (bo_pseudorandom(&run->random, key) < run->probabilities / X(ODDS)) {
                self->getting_sliced = 1;
                if (IS(GROS_MICHEL)) run->gros_michel_extinct = 1;
            }
            return message(NULL);
        }
        if (IS(MR_BONES) && ctx->game_over && run->chips / run->blind_chips >= 0.25) {
            self->getting_sliced = 1;
            BOEffect e = message(NULL);
            e.saved = 1;
            return e;
        }
        return none();

    case BO_CTX_INDIVIDUAL:
        if (ctx->cardarea == BO_IN_PLAY) {
            BOCard *other = ctx->other_card;
            if (IS(HIKER)) {
                other->perma_bonus += (int16_t)A->extra;
                BOEffect e = message(self);
                e.extra = 1;
                return e;
            }
            if (IS(LUCKY_CAT) && other->lucky_trigger && !ctx->blueprint) {
                A->x_mult += A->extra;
                BOEffect e = message(self);
                e.extra = 1;
                return e;
            }
            if (IS(WEE) && get_id(other) == 2 && !ctx->blueprint) {
                SET_X(CHIPS, X(CHIPS) + X(CHIP_MOD));
                BOEffect e = message(self);
                e.extra = 1;
                return e;
            }
            if (IS(PHOTOGRAPH)) {
                BOCard *first = NULL;
                for (int i = 0; i < ctx->scoring_count; i++) {
                    if (is_face(run, ctx->scoring[i])) {
                        first = ctx->scoring[i];
                        break;
                    }
                }
                if (other == first) return card_x_mult(self, A->extra);
            }
            if (IS(8_BALL) && consumable_room(run)) {
                if (get_id(other) == 8 && bo_pseudorandom(&run->random, "8ball") < run->probabilities / A->extra) {
                    run->consumeable_buffer++;
                    make_consumable(run, BO_SET_TAROT, "8ba");
                    BOEffect e = message(self);
                    e.extra = 1;
                    return e;
                }
            }
            if (IS(IDOL) && get_id(other) == run->idol_rank && is_suit(run, other, run->idol_suit)) {
                return card_x_mult(self, A->extra);
            }
            if (IS(SCARY_FACE) && is_face(run, other)) return card_chips(self, A->extra);
            if (IS(SMILEY) && is_face(run, other)) return card_mult(self, A->extra);
            if (IS(TICKET) && other->enhancement == BO_ENH_GOLD) {
                run->dollar_buffer += (int)A->extra;
                return card_dollars(self, (int)A->extra);
            }
            if (IS(SCHOLAR) && get_id(other) == 14) {
                BOEffect e = card_chips(self, X(CHIPS));
                e.has_mult = 1;
                e.mult = X(MULT);
                return e;
            }
            if (IS(WALKIE_TALKIE) && (get_id(other) == 10 || get_id(other) == 4)) {
                BOEffect e = card_chips(self, X(CHIPS));
                e.has_mult = 1;
                e.mult = X(MULT);
                return e;
            }
            if (IS(BUSINESS) && is_face(run, other) &&
                bo_pseudorandom(&run->random, "business") < run->probabilities / A->extra) {
                run->dollar_buffer += 2;
                return card_dollars(self, 2);
            }
            if (IS(FIBONACCI)) {
                int id = get_id(other);
                if (id == 2 || id == 3 || id == 5 || id == 8 || id == 14) return card_mult(self, A->extra);
            }
            if (IS(EVEN_STEVEN)) {
                int id = get_id(other);
                if (id <= 10 && id >= 0 && id % 2 == 0) return card_mult(self, A->extra);
            }
            if (IS(ODD_TODD)) {
                int id = get_id(other);
                if ((id <= 10 && id >= 0 && id % 2 == 1) || id == 14) return card_chips(self, A->extra);
            }
            if (center->effect && strcmp(center->effect, "Suit Mult") == 0) {
                int suit = bo_rules_suit_by_name(bo_center_text(center, BO_F_EXTRA_SUIT));
                if (is_suit(run, other, suit)) return card_mult(self, X(S_MULT));
            }
            if (IS(ROUGH_GEM) && is_suit(run, other, BO_DIAMONDS)) {
                run->dollar_buffer += (int)A->extra;
                return card_dollars(self, (int)A->extra);
            }
            if (IS(ONYX_AGATE) && is_suit(run, other, BO_CLUBS)) return card_mult(self, A->extra);
            if (IS(ARROWHEAD) && is_suit(run, other, BO_SPADES)) return card_chips(self, A->extra);
            if (IS(BLOODSTONE) && is_suit(run, other, BO_HEARTS) &&
                bo_pseudorandom(&run->random, "bloodstone") < run->probabilities / X(ODDS)) {
                return card_x_mult(self, X(XMULT));
            }
            if (IS(ANCIENT) && is_suit(run, other, run->ancient_suit)) return card_x_mult(self, A->extra);
            if (IS(TRIBOULET) && (get_id(other) == 12 || get_id(other) == 13)) return card_x_mult(self, A->extra);
        }
        if (ctx->cardarea == BO_IN_HAND) {
            BOCard *other = ctx->other_card;
            if (IS(SHOOT_THE_MOON) && get_id(other) == 12) {
                if (other->debuffed) return message(self);
                BOEffect e = none();
                e.any = 1;
                e.has_h_mult = 1;
                e.h_mult = 13;
                e.card = self;
                return e;
            }
            if (IS(BARON) && get_id(other) == 13) {
                if (other->debuffed) return message(self);
                return card_x_mult(self, A->extra);
            }
            if (IS(RESERVED_PARKING) && is_face(run, other) &&
                bo_pseudorandom(&run->random, "parking") < run->probabilities / X(ODDS)) {
                if (other->debuffed) return message(self);
                run->dollar_buffer += (int)X(DOLLARS);
                return card_dollars(self, (int)X(DOLLARS));
            }
            if (IS(RAISED_FIST)) {
                int tempMult = 15, tempId = 15;
                BOCard *raised = NULL;
                for (int i = 0; i < run->hand_count; i++) {
                    BOCard *c = &run->hand[i];
                    if (tempId >= c->rank && !bo_card_is_stone(c)) {
                        tempMult = bo_card_nominal(c);
                        tempId = c->rank;
                        raised = c;
                    }
                }
                if (raised == other) {
                    if (other->debuffed) return message(self);
                    BOEffect e = none();
                    e.any = 1;
                    e.has_h_mult = 1;
                    e.h_mult = 2 * tempMult;
                    e.card = self;
                    return e;
                }
            }
        }
        return none();

    case BO_CTX_REPETITION:
        if (ctx->cardarea == BO_IN_PLAY) {
            BOCard *other = ctx->other_card;
            if (IS(SOCK_AND_BUSKIN) && is_face(run, other)) return repeat(self, (int)A->extra);
            if (IS(HANGING_CHAD) && ctx->scoring_count > 0 && other == ctx->scoring[0]) {
                return repeat(self, (int)A->extra);
            }
            if (IS(DUSK) && run->hands_left == 0) return repeat(self, (int)A->extra);
            if (IS(SELZER)) return repeat(self, 1);
            if (IS(HACK)) {
                int id = get_id(other);
                if (id == 2 || id == 3 || id == 4 || id == 5) return repeat(self, (int)A->extra);
            }
        }
        if (ctx->cardarea == BO_IN_HAND) {
            if (IS(MIME) && ctx->card_effects_any) return repeat(self, (int)A->extra);
        }
        return none();

    case BO_CTX_OTHER_JOKER:
        if (IS(BASEBALL) && bo_item_center(ctx->other_joker)->rarity == 2 && self != ctx->other_joker) {
            return xmult_mod(A->extra);
        }
        return none();

    case BO_CTX_BEFORE:
    case BO_CTX_AFTER:
    case BO_CTX_JOKER_MAIN:
        break;
    }

    if (ctx->cardarea != BO_IN_JOKERS) return none();
    const BOHandResult *hands = ctx->poker_hands;

    if (ctx->kind == BO_CTX_BEFORE) {
        if (IS(TROUSERS) && (bo_hand_contains(hands, BO_HAND_TWO_PAIR) || bo_hand_contains(hands, BO_HAND_FULL_HOUSE)) &&
            !ctx->blueprint) {
            A->mult += A->extra;
            return message(self);
        }
        if (IS(SPACE) && bo_pseudorandom(&run->random, "space") < run->probabilities / A->extra) {
            BOEffect e = message(self);
            e.level_up = 1;
            return e;
        }
        if (IS(SQUARE) && ctx->full_count == 4 && !ctx->blueprint) {
            SET_X(CHIPS, X(CHIPS) + X(CHIP_MOD));
            return message(self);
        }
        if (IS(RUNNER) && bo_hand_contains(hands, BO_HAND_STRAIGHT) && !ctx->blueprint) {
            SET_X(CHIPS, X(CHIPS) + X(CHIP_MOD));
            return message(self);
        }
        if (IS(MIDAS_MASK) && !ctx->blueprint) {
            int faces = 0;
            for (int i = 0; i < ctx->scoring_count; i++) {
                if (is_face(run, ctx->scoring[i])) {
                    faces++;
                    bo_rules_set_enhancement(run, ctx->scoring[i], BO_ENH_GOLD);
                }
            }
            if (faces > 0) return message(self);
        }
        if (IS(VAMPIRE) && !ctx->blueprint) {
            int enhanced = 0;
            for (int i = 0; i < ctx->scoring_count; i++) {
                BOCard *c = ctx->scoring[i];
                if (c->enhancement != BO_ENH_NONE && !c->debuffed && !c->vampired) {
                    enhanced++;
                    c->vampired = 1;
                    bo_rules_set_enhancement(run, c, BO_ENH_NONE);
                }
            }
            if (enhanced > 0) {
                A->x_mult += A->extra * enhanced;
                return message(self);
            }
        }
        if (IS(TODO_LIST) && ctx->scoring_name == A->to_do_hand) {
            bo_run_ease_dollars(run, (int)X(DOLLARS));
            run->dollar_buffer += (int)X(DOLLARS);
            BOEffect e = message(NULL);
            e.dollars = (int)X(DOLLARS);
            return e;
        }
        if (IS(DNA) && run->hands_played == 0) {
            if (ctx->full_count == 1) {
                bo_rules_dna_copy(run, &ctx->full_hand[0]);
                BOEffect e = message(self);
                e.playing_cards_created = 1;
                return e;
            }
        }
        if (IS(RIDE_THE_BUS) && !ctx->blueprint) {
            int faces = 0;
            for (int i = 0; i < ctx->scoring_count; i++) {
                if (is_face(run, ctx->scoring[i])) faces = 1;
            }
            if (faces) {
                double last = A->mult;
                A->mult = 0;
                if (last > 0) return message(self);
            } else {
                A->mult += A->extra;
            }
        }
        if (IS(OBELISK) && !ctx->blueprint) {
            int reset = 1;
            int playMoreThan = run->hands[ctx->scoring_name].played;
            for (int i = 0; i < BO_HAND_COUNT; i++) {
                if (i != (int)ctx->scoring_name && run->hands[i].played >= playMoreThan && run->hands[i].visible) {
                    reset = 0;
                }
            }
            if (reset) {
                if (A->x_mult > 1) {
                    A->x_mult = 1;
                    return message(self);
                }
            } else {
                A->x_mult += A->extra;
            }
        }
        if (IS(GREEN_JOKER) && !ctx->blueprint) {
            A->mult += X(HAND_ADD);
            return message(self);
        }
        return none();
    }

    if (ctx->kind == BO_CTX_AFTER) {
        if (IS(ICE_CREAM) && !ctx->blueprint) {
            if (X(CHIPS) - X(CHIP_MOD) <= 0) {
                self->getting_sliced = 1;
                return message(NULL);
            }
            SET_X(CHIPS, X(CHIPS) - X(CHIP_MOD));
            return message(NULL);
        }
        if (IS(SELZER) && !ctx->blueprint) {
            if (A->extra - 1 <= 0) {
                self->getting_sliced = 1;
                return message(NULL);
            }
            A->extra -= 1;
            return message(NULL);
        }
        return none();
    }

    // joker_main.
    if (IS(LOYALTY_CARD)) {
        double every = X(EVERY);
        double left = every - 1 - (run->hands_played_total - A->hands_played_at_create);
        // The % of Lua: the result has the sign of the divisor.
        double m = every + 1;
        A->loyalty_remaining = (int)(left - floor(left / m) * m);
        if (A->loyalty_remaining == every) return xmult_mod(X(XMULT));
    }
    if (!IS(SEEING_DOUBLE) && A->x_mult > 1) {
        int type = center->hand_type ? bo_rules_hand_by_name(center->hand_type) : -1;
        if (!center->hand_type || bo_hand_contains(hands, (BOHandType)type)) return xmult_mod(A->x_mult);
    }
    if (A->t_mult > 0 && center->hand_type &&
        bo_hand_contains(hands, (BOHandType)bo_rules_hand_by_name(center->hand_type))) {
        return mult_mod(A->t_mult);
    }
    if (A->t_chips > 0 && center->hand_type &&
        bo_hand_contains(hands, (BOHandType)bo_rules_hand_by_name(center->hand_type))) {
        return chip_mod(A->t_chips);
    }
    if (IS(HALF) && ctx->full_count <= X(SIZE)) return mult_mod(X(MULT));
    if (IS(ABSTRACT)) {
        int x = 0;
        for (int i = 0; i < run->joker_count; i++) {
            if (bo_item_center(&run->jokers[i])->set == BO_SET_JOKER) x++;
        }
        return mult_mod(x * A->extra);
    }
    if (IS(ACROBAT) && run->hands_left == 0) return xmult_mod(A->extra);
    if (IS(MYSTIC_SUMMIT) && run->discards_left == X(D_REMAINING)) return mult_mod(X(MULT));
    if (IS(MISPRINT)) {
        int value = bo_pseudorandom_int(&run->random, "misprint", (int)X(MIN), (int)X(MAX));
        return mult_mod(value);
    }
    if (IS(BANNER) && run->discards_left > 0) return chip_mod(run->discards_left * A->extra);
    if (IS(STUNTMAN)) return chip_mod(X(CHIP_MOD));
    if (IS(MATADOR)) {
        if (run->blind_triggered) {
            bo_run_ease_dollars(run, (int)A->extra);
            run->dollar_buffer += (int)A->extra;
            BOEffect e = message(NULL);
            e.dollars = (int)A->extra;
            return e;
        }
    }
    if (IS(SUPERNOVA)) return mult_mod(run->hands[ctx->scoring_name].played);
    if (IS(CEREMONIAL) && A->mult > 0) return mult_mod(A->mult);
    if (IS(VAGABOND) && consumable_room(run)) {
        if (run->dollars <= A->extra) {
            run->consumeable_buffer++;
            make_consumable(run, BO_SET_TAROT, "vag");
            return message(self);
        }
    }
    if (IS(SUPERPOSITION) && consumable_room(run)) {
        int aces = 0;
        for (int i = 0; i < ctx->scoring_count; i++) {
            if (get_id(ctx->scoring[i]) == 14) aces++;
        }
        if (aces >= 1 && bo_hand_contains(hands, BO_HAND_STRAIGHT)) {
            run->consumeable_buffer++;
            make_consumable(run, BO_SET_TAROT, "sup");
            return message(self);
        }
    }
    if (IS(SEANCE) && consumable_room(run)) {
        if (bo_hand_contains(hands, (BOHandType)bo_rules_hand_by_name(bo_center_text(center, BO_F_EXTRA_POKER_HAND)))) {
            run->consumeable_buffer++;
            make_consumable(run, BO_SET_SPECTRAL, "sea");
            return message(self);
        }
    }
    if (IS(FLOWER_POT)) {
        // The order of the tests is Hearts, Diamonds, Spades, Clubs.
        static const int order[4] = {BO_HEARTS, BO_DIAMONDS, BO_SPADES, BO_CLUBS};
        int suits[4] = {0};
        for (int pass = 0; pass < 2; pass++) {
            for (int i = 0; i < ctx->scoring_count; i++) {
                BOCard *c = ctx->scoring[i];
                int wild = c->enhancement == BO_ENH_WILD;
                if (wild != pass) continue;
                for (int s = 0; s < 4; s++) {
                    if (bo_rules_is_suit(run, c, order[s], !wild, 0) && suits[s] == 0) {
                        suits[s]++;
                        break;
                    }
                }
            }
        }
        if (suits[0] > 0 && suits[1] > 0 && suits[2] > 0 && suits[3] > 0) return xmult_mod(A->extra);
    }
    if (IS(SEEING_DOUBLE)) {
        // Hearts, Diamonds, Spades, Clubs. A wild card tests Clubs first.
        int hearts = 0, diamonds = 0, spades = 0, clubs = 0;
        for (int i = 0; i < ctx->scoring_count; i++) {
            BOCard *c = ctx->scoring[i];
            if (c->enhancement == BO_ENH_WILD) continue;
            if (is_suit(run, c, BO_HEARTS)) hearts++;
            if (is_suit(run, c, BO_DIAMONDS)) diamonds++;
            if (is_suit(run, c, BO_SPADES)) spades++;
            if (is_suit(run, c, BO_CLUBS)) clubs++;
        }
        for (int i = 0; i < ctx->scoring_count; i++) {
            BOCard *c = ctx->scoring[i];
            if (c->enhancement != BO_ENH_WILD) continue;
            if (is_suit(run, c, BO_CLUBS) && clubs == 0) clubs++;
            else if (is_suit(run, c, BO_DIAMONDS) && diamonds == 0) diamonds++;
            else if (is_suit(run, c, BO_SPADES) && spades == 0) spades++;
            else if (is_suit(run, c, BO_HEARTS) && hearts == 0) hearts++;
        }
        if ((hearts > 0 || diamonds > 0 || spades > 0) && clubs > 0) return xmult_mod(A->extra);
    }
    if (IS(WEE)) return chip_mod(X(CHIPS));
    if (IS(CASTLE) && X(CHIPS) > 0) return chip_mod(X(CHIPS));
    if (IS(BLUE_JOKER) && run->deck_count > 0) return chip_mod(A->extra * run->deck_count);
    if (IS(EROSION)) {
        int missing = run->starting_deck_size - bo_rules_playing_card_count(run);
        if (missing > 0) return mult_mod(A->extra * missing);
    }
    if (IS(SQUARE)) return chip_mod(X(CHIPS));
    if (IS(RUNNER)) return chip_mod(X(CHIPS));
    if (IS(ICE_CREAM)) return chip_mod(X(CHIPS));
    if (IS(STONE) && A->stone_tally > 0) return chip_mod(A->extra * A->stone_tally);
    if (IS(STEEL_JOKER) && A->steel_tally > 0) return xmult_mod(1 + A->extra * A->steel_tally);
    if (IS(BULL) && run->dollars + run->dollar_buffer > 0) {
        return chip_mod(A->extra * fmax(0, run->dollars + run->dollar_buffer));
    }
    if (IS(DRIVERS_LICENSE)) {
        if (A->driver_tally >= 16) return xmult_mod(A->extra);
    }
    if (IS(BLACKBOARD)) {
        int black = 0, all = 0;
        for (int i = 0; i < run->hand_count; i++) {
            all++;
            if (bo_rules_is_suit(run, &run->hand[i], BO_CLUBS, 0, 1) ||
                bo_rules_is_suit(run, &run->hand[i], BO_SPADES, 0, 1)) {
                black++;
            }
        }
        if (black == all) return xmult_mod(A->extra);
    }
    if (IS(STENCIL)) {
        if (run->joker_slots - run->joker_count > 0) return xmult_mod(A->x_mult);
    }
    if (IS(SWASHBUCKLER) && A->mult > 0) return mult_mod(A->mult);
    if (IS(JOKER)) return mult_mod(A->mult);
    if (IS(TROUSERS) && A->mult > 0) return mult_mod(A->mult);
    if (IS(RIDE_THE_BUS) && A->mult > 0) return mult_mod(A->mult);
    if (IS(FLASH) && A->mult > 0) return mult_mod(A->mult);
    if (IS(POPCORN) && A->mult > 0) return mult_mod(A->mult);
    if (IS(GREEN_JOKER) && A->mult > 0) return mult_mod(A->mult);
    if (IS(FORTUNE_TELLER) && run->tarots_used > 0) return mult_mod(run->tarots_used);
    if (IS(GROS_MICHEL)) return mult_mod(X(MULT));
    if (IS(CAVENDISH)) return xmult_mod(X(XMULT));
    if (IS(RED_CARD) && A->mult > 0) return mult_mod(A->mult);
    if (IS(CARD_SHARP) && run->hands[ctx->scoring_name].played_this_round > 1) return xmult_mod(X(XMULT));
    if (IS(BOOTSTRAPS)) {
        double steps = floor((run->dollars + run->dollar_buffer) / X(DOLLARS));
        if (steps >= 1) return mult_mod(X(MULT) * steps);
    }
    if (IS(CAINO) && A->caino_xmult > 1) return xmult_mod(A->caino_xmult);
    return none();
}

void bo_joker_add_to_deck(BORun *run, BOItem *self) {
    const BOCenterInfo *center = bo_item_center(self);
    if (!center) return;
    if (A->h_size != 0) bo_rules_change_hand_size(run, (int)A->h_size);
    if (A->d_size > 0) {
        run->discards_per_round += (int)A->d_size;
        run->discards_left += (int)A->d_size;
    }
    if (IS(CREDIT_CARD)) run->bankrupt_at -= (int)A->extra;
    if (IS(CHICOT) && run->state == BO_STATE_SELECTING && bo_blind_info(run->blind)->boss && !run->blind_disabled) {
        bo_rules_disable_blind(run);
    }
    if (IS(CHAOS)) {
        run->free_rerolls++;
        bo_rules_calculate_reroll_cost(run, 1);
    }
    if (IS(TURTLE_BEAN)) bo_rules_change_hand_size(run, (int)X(H_SIZE));
    if (IS(OOPS)) run->probabilities *= 2;
    if (IS(TO_THE_MOON)) run->interest_amount += (int)A->extra;
    if (IS(ASTRONOMER)) bo_rules_set_all_costs(run);
    if (IS(TROUBADOUR)) {
        bo_rules_change_hand_size(run, (int)X(H_SIZE));
        run->hands_per_round += (int)X(H_PLAYS);
    }
    if (IS(STUNTMAN)) bo_rules_change_hand_size(run, -(int)X(H_SIZE));
    if (self->edition == BO_EDITION_NEGATIVE) {
        if (center->consumeable) {
            run->consumable_slots++;
        } else {
            run->joker_slots++;
        }
    }
}

void bo_joker_remove_from_deck(BORun *run, BOItem *self) {
    const BOCenterInfo *center = bo_item_center(self);
    if (!center) return;
    if (A->h_size != 0) bo_rules_change_hand_size(run, -(int)A->h_size);
    if (A->d_size > 0) {
        run->discards_per_round -= (int)A->d_size;
        run->discards_left -= (int)A->d_size;
    }
    if (IS(CREDIT_CARD)) run->bankrupt_at += (int)A->extra;
    if (IS(CHAOS)) {
        run->free_rerolls--;
        bo_rules_calculate_reroll_cost(run, 1);
    }
    if (IS(TURTLE_BEAN)) bo_rules_change_hand_size(run, -(int)X(H_SIZE));
    if (IS(OOPS)) run->probabilities /= 2;
    if (IS(TO_THE_MOON)) run->interest_amount -= (int)A->extra;
    if (IS(ASTRONOMER)) bo_rules_set_all_costs(run);
    if (IS(TROUBADOUR)) {
        bo_rules_change_hand_size(run, -(int)X(H_SIZE));
        run->hands_per_round -= (int)X(H_PLAYS);
    }
    if (IS(STUNTMAN)) bo_rules_change_hand_size(run, (int)X(H_SIZE));
    if (self->edition == BO_EDITION_NEGATIVE) {
        if (center->consumeable) {
            run->consumable_slots--;
        } else {
            run->joker_slots--;
        }
    }
}

int bo_joker_dollar_bonus(BORun *run, BOItem *self, int *has) {
    *has = 0;
    if (self->debuffed) return 0;
    if (bo_item_center(self)->set != BO_SET_JOKER) return 0;
    if (IS(GOLDEN)) {
        *has = 1;
        return (int)A->extra;
    }
    if (IS(CLOUD_9)) {
        if (A->nine_tally > 0) {
            *has = 1;
            return (int)A->extra * A->nine_tally;
        }
        return 0;
    }
    if (IS(ROCKET)) {
        *has = 1;
        return (int)X(DOLLARS);
    }
    if (IS(SATELLITE)) {
        if (run->planets_used_unique == 0) return 0;
        *has = 1;
        return (int)A->extra * run->planets_used_unique;
    }
    if (IS(DELAYED_GRAT) && run->discards_used == 0 && run->discards_left > 0) {
        *has = 1;
        return run->discards_left * (int)A->extra;
    }
    return 0;
}

void bo_joker_update(BORun *run, BOItem *self) {
    if (IS(DRIVERS_LICENSE)) A->driver_tally = bo_rules_count_enhanced(run);
    if (IS(STEEL_JOKER)) A->steel_tally = bo_rules_count_enhancement(run, BO_ENH_STEEL);
    if (IS(CLOUD_9)) A->nine_tally = bo_rules_count_rank(run, 9);
    if (IS(STONE)) A->stone_tally = bo_rules_count_enhancement(run, BO_ENH_STONE);
    if (IS(THROWBACK)) A->x_mult = 1 + run->skips * A->extra;
    if (IS(STENCIL)) {
        A->x_mult = run->joker_slots - run->joker_count;
        for (int i = 0; i < run->joker_count; i++) {
            if (run->jokers[i].center == BO_J_STENCIL) A->x_mult += 1;
        }
    }
    if (IS(SWASHBUCKLER)) {
        int sell = 0;
        for (int i = 0; i < run->joker_count; i++) {
            if (&run->jokers[i] != self) sell += run->jokers[i].sell_cost;
        }
        A->mult = sell;
    }
}
