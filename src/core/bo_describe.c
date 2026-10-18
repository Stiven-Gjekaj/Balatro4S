#include "bo_describe.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bo_rules.h"
#include "bo_run.h"

#pragma STDC FP_CONTRACT OFF

static BOLuaValue *gText;

void bo_describe_set_text(BOLuaValue *root) {
    gText = root;
}

const char *bo_describe_word(const char *key) {
    char path[128];
    snprintf(path, sizeof(path), "misc.dictionary.%s", key);
    const char *word = gText ? bo_luatab_string(gText, path) : NULL;
    return word ? word : key;
}

void bo_describe_number(double value, char *out, int size) {
    // tostring of LuaJIT: "%.14g".
    snprintf(out, (size_t)size, "%.14g", value);
}

static const char *poker_hand(int type) {
    const char *name = bo_hand_name((BOHandType)type, 0);
    char path[96];
    snprintf(path, sizeof(path), "misc.poker_hands.%s", name);
    const char *word = gText ? bo_luatab_string(gText, path) : NULL;
    return word ? word : name;
}

static const char *suit_word(int suit, int plural) {
    static const char *names[4] = {"Spades", "Hearts", "Clubs", "Diamonds"};
    if (suit < 0 || suit > 3) return "";
    char path[96];
    snprintf(path, sizeof(path), "misc.%s.%s", plural ? "suits_plural" : "suits_singular", names[suit]);
    const char *word = gText ? bo_luatab_string(gText, path) : NULL;
    return word ? word : names[suit];
}

static const char *rank_word(int rank) {
    static const char *names[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace"};
    if (rank < 2 || rank > 14) return "";
    char path[96];
    snprintf(path, sizeof(path), "misc.ranks.%s", names[rank]);
    const char *word = gText ? bo_luatab_string(gText, path) : NULL;
    return word ? word : names[rank];
}

typedef struct {
    char values[8][48];
    int count;
} BOVars;

static void fill_line(const char *line, const BOVars *vars, char *out, int size);

static void var_number(BOVars *v, double value) {
    if (v->count < 8) bo_describe_number(value, v->values[v->count++], 48);
}

static void var_text(BOVars *v, const char *text) {
    if (v->count < 8) snprintf(v->values[v->count++], 48, "%s", text ? text : "");
}

#define X(field) bo_item_get(item, BO_F_EXTRA_##field)

// The loc_vars of generate_UIBox_ability_table for a joker.
static void joker_vars(const BORun *run, const BOItem *item, BOVars *v) {
    const BOAbility *a = &item->ability;
    const BOCenterInfo *c = bo_item_center(item);
    double odds = run ? run->probabilities : 1;
    switch (item->center) {
    case BO_J_JOKER: var_number(v, a->mult); break;
    case BO_J_JOLLY: case BO_J_ZANY: case BO_J_MAD: case BO_J_CRAZY: case BO_J_DROLL:
        var_number(v, a->t_mult);
        var_text(v, poker_hand(bo_rules_hand_by_name(c->hand_type)));
        break;
    case BO_J_SLY: case BO_J_WILY: case BO_J_CLEVER: case BO_J_DEVIOUS: case BO_J_CRAFTY:
        var_number(v, a->t_chips);
        var_text(v, poker_hand(bo_rules_hand_by_name(c->hand_type)));
        break;
    case BO_J_HALF: var_number(v, X(MULT)); var_number(v, X(SIZE)); break;
    case BO_J_FORTUNE_TELLER: var_number(v, a->extra); var_number(v, run ? run->tarots_used : 0); break;
    case BO_J_STEEL_JOKER: var_number(v, a->extra); var_number(v, 1 + a->extra * a->steel_tally); break;
    case BO_J_CHAOS: var_number(v, a->extra); break;
    case BO_J_SPACE: var_number(v, odds); var_number(v, a->extra); break;
    case BO_J_STONE: var_number(v, a->extra); var_number(v, a->extra * a->stone_tally); break;
    case BO_J_DRUNKARD: var_number(v, a->d_size); break;
    case BO_J_GREEN_JOKER: var_number(v, X(HAND_ADD)); var_number(v, X(DISCARD_SUB)); var_number(v, a->mult); break;
    case BO_J_CREDIT_CARD: var_number(v, a->extra); break;
    case BO_J_GREEDY_JOKER: case BO_J_LUSTY_JOKER: case BO_J_WRATHFUL_JOKER: case BO_J_GLUTTENOUS_JOKER:
        var_number(v, X(S_MULT));
        var_text(v, suit_word(bo_rules_suit_by_name(bo_center_text(c, BO_F_EXTRA_SUIT)), 0));
        break;
    case BO_J_BLUE_JOKER: var_number(v, a->extra); var_number(v, a->extra * (run ? run->deck_count : 52)); break;
    case BO_J_HACK: var_number(v, a->extra + 1); break;
    case BO_J_FACELESS: var_number(v, X(DOLLARS)); var_number(v, X(FACES)); break;
    case BO_J_JUGGLER: var_number(v, a->h_size); break;
    case BO_J_GOLDEN: var_number(v, a->extra); break;
    case BO_J_STENCIL: var_number(v, a->x_mult); break;
    case BO_J_CEREMONIAL: var_number(v, a->mult); break;
    case BO_J_BANNER: var_number(v, a->extra); break;
    case BO_J_MYSTIC_SUMMIT: var_number(v, X(MULT)); var_number(v, X(D_REMAINING)); break;
    case BO_J_LOYALTY_CARD: {
        var_number(v, X(XMULT));
        var_number(v, X(EVERY) + 1);
        // localize{type = 'variable', key = 'loyalty_active' or 'loyalty_inactive'}.
        const char *form = gText ? bo_luatab_string(gText, a->loyalty_remaining == 0 ? "misc.v_dictionary.loyalty_active"
                                                                                      : "misc.v_dictionary.loyalty_inactive")
                                 : NULL;
        BOVars inner;
        memset(&inner, 0, sizeof(inner));
        var_number(&inner, a->loyalty_remaining);
        char text[48];
        fill_line(form ? form : "#1# remaining", &inner, text, sizeof(text));
        var_text(v, text);
        break;
    }
    case BO_J_8_BALL: var_number(v, odds); var_number(v, a->extra); break;
    case BO_J_DUSK: var_number(v, a->extra + 1); break;
    case BO_J_FIBONACCI: case BO_J_SCARY_FACE: case BO_J_DELAYED_GRAT: case BO_J_EVEN_STEVEN: case BO_J_ODD_TODD:
        var_number(v, a->extra);
        break;
    case BO_J_ABSTRACT: var_number(v, a->extra); var_number(v, (run ? run->joker_count : 0) * a->extra); break;
    case BO_J_GROS_MICHEL: var_number(v, X(MULT)); var_number(v, odds); var_number(v, X(ODDS)); break;
    case BO_J_SCHOLAR: var_number(v, X(MULT)); var_number(v, X(CHIPS)); break;
    case BO_J_BUSINESS: var_number(v, odds); var_number(v, a->extra); break;
    case BO_J_TROUSERS: var_number(v, a->extra); var_text(v, poker_hand(BO_HAND_TWO_PAIR)); var_number(v, a->mult); break;
    case BO_J_SUPERPOSITION: case BO_J_EGG: case BO_J_BURGLAR: case BO_J_DNA: case BO_J_HIKER:
        var_number(v, a->extra);
        break;
    case BO_J_RIDE_THE_BUS: var_number(v, a->extra); var_number(v, a->mult); break;
    case BO_J_BLACKBOARD: var_number(v, a->extra); var_text(v, suit_word(BO_SPADES, 1)); var_text(v, suit_word(BO_CLUBS, 1)); break;
    case BO_J_RUNNER: case BO_J_ICE_CREAM: case BO_J_WEE: case BO_J_SQUARE:
        var_number(v, X(CHIPS));
        var_number(v, X(CHIP_MOD));
        break;
    case BO_J_CONSTELLATION: case BO_J_THROWBACK: case BO_J_GLASS: case BO_J_HIT_THE_ROAD: case BO_J_MADNESS:
    case BO_J_VAMPIRE: case BO_J_HOLOGRAM: case BO_J_OBELISK: case BO_J_LUCKY_CAT: case BO_J_CAMPFIRE:
        var_number(v, a->extra);
        var_number(v, a->x_mult);
        break;
    case BO_J_TODO_LIST: var_number(v, X(DOLLARS)); var_text(v, poker_hand(a->to_do_hand)); break;
    case BO_J_ASTRONOMER: case BO_J_TICKET: case BO_J_ACROBAT: case BO_J_CERTIFICATE: case BO_J_HANGING_CHAD:
    case BO_J_ROUGH_GEM: case BO_J_ARROWHEAD: case BO_J_ONYX_AGATE: case BO_J_FLOWER_POT: case BO_J_SEEING_DOUBLE:
    case BO_J_MATADOR: case BO_J_RIFF_RAFF: case BO_J_VAGABOND: case BO_J_BARON: case BO_J_PHOTOGRAPH: case BO_J_GIFT:
    case BO_J_TO_THE_MOON: case BO_J_BASEBALL: case BO_J_TRADING: case BO_J_SELZER: case BO_J_SMILEY:
    case BO_J_SHOOT_THE_MOON: case BO_J_TRIBOULET: case BO_J_PERKEO:
        var_number(v, a->extra);
        break;
    case BO_J_SOCK_AND_BUSKIN: var_number(v, a->extra + 1); break;
    case BO_J_SWASHBUCKLER: var_number(v, a->mult); break;
    case BO_J_TROUBADOUR: var_number(v, X(H_SIZE)); var_number(v, -X(H_PLAYS)); break;
    case BO_J_BLOODSTONE: var_number(v, odds); var_number(v, X(ODDS)); var_number(v, X(XMULT)); break;
    case BO_J_MERRY_ANDY: var_number(v, a->d_size); var_number(v, a->h_size); break;
    case BO_J_IDOL:
        var_number(v, a->extra);
        var_text(v, rank_word(run ? run->idol_rank : 14));
        var_text(v, suit_word(run ? run->idol_suit : BO_SPADES, 1));
        break;
    case BO_J_DUO: case BO_J_TRIO: case BO_J_FAMILY: case BO_J_ORDER: case BO_J_TRIBE:
        var_number(v, a->x_mult);
        var_text(v, poker_hand(bo_rules_hand_by_name(c->hand_type)));
        break;
    case BO_J_CAVENDISH: var_number(v, X(XMULT)); var_number(v, odds); var_number(v, X(ODDS)); break;
    case BO_J_CARD_SHARP: var_number(v, X(XMULT)); break;
    case BO_J_RED_CARD: case BO_J_FLASH: var_number(v, a->extra); var_number(v, a->mult); break;
    case BO_J_SEANCE: var_text(v, poker_hand(bo_rules_hand_by_name(bo_center_text(c, BO_F_EXTRA_POKER_HAND)))); break;
    case BO_J_CLOUD_9: var_number(v, a->extra); var_number(v, a->extra * a->nine_tally); break;
    case BO_J_ROCKET: var_number(v, X(DOLLARS)); var_number(v, X(INCREASE)); break;
    case BO_J_TURTLE_BEAN: var_number(v, X(H_SIZE)); var_number(v, X(H_MOD)); break;
    case BO_J_EROSION: {
        int missing = run ? run->starting_deck_size - bo_rules_playing_card_count(run) : 0;
        var_number(v, a->extra);
        var_number(v, fmax(0, a->extra * missing));
        var_number(v, run ? run->starting_deck_size : 52);
        break;
    }
    case BO_J_RESERVED_PARKING: var_number(v, X(DOLLARS)); var_number(v, odds); var_number(v, X(ODDS)); break;
    case BO_J_MAIL: var_number(v, a->extra); var_text(v, rank_word(run ? run->mail_rank : 14)); break;
    case BO_J_HALLUCINATION: var_number(v, odds); var_number(v, a->extra); break;
    case BO_J_BULL: var_number(v, a->extra); var_number(v, a->extra * fmax(0, run ? run->dollars : 0)); break;
    case BO_J_DIET_COLA: var_text(v, "Double Tag"); break;
    case BO_J_POPCORN: var_number(v, a->mult); var_number(v, a->extra); break;
    case BO_J_RAMEN: var_number(v, a->x_mult); var_number(v, a->extra); break;
    case BO_J_ANCIENT: var_number(v, a->extra); var_text(v, suit_word(run ? run->ancient_suit : BO_SPADES, 0)); break;
    case BO_J_WALKIE_TALKIE: var_number(v, X(CHIPS)); var_number(v, X(MULT)); break;
    case BO_J_CASTLE:
        var_number(v, X(CHIP_MOD));
        var_text(v, suit_word(run ? run->castle_suit : BO_SPADES, 0));
        var_number(v, X(CHIPS));
        break;
    case BO_J_STUNTMAN: var_number(v, X(CHIP_MOD)); var_number(v, X(H_SIZE)); break;
    case BO_J_INVISIBLE: var_number(v, a->extra); var_number(v, a->invis_rounds); break;
    case BO_J_SATELLITE: var_number(v, a->extra); var_number(v, (run ? run->planets_used_unique : 0) * a->extra); break;
    case BO_J_DRIVERS_LICENSE: var_number(v, a->extra); var_number(v, a->driver_tally); break;
    case BO_J_BOOTSTRAPS: {
        double steps = run ? floor((run->dollars + run->dollar_buffer) / X(DOLLARS)) : 0;
        var_number(v, X(MULT));
        var_number(v, X(DOLLARS));
        var_number(v, X(MULT) * steps);
        break;
    }
    case BO_J_CAINO: var_number(v, a->extra); var_number(v, a->caino_xmult); break;
    case BO_J_YORICK:
        var_number(v, X(XMULT_LC));
        var_number(v, X(DISCARDS));
        var_number(v, a->yorick_discards);
        var_number(v, a->x_mult);
        break;
    default: break;
    }
}

static const char *set_key(int set) {
    switch (set) {
    case BO_SET_JOKER: return "Joker";
    case BO_SET_TAROT: return "Tarot";
    case BO_SET_PLANET: return "Planet";
    case BO_SET_SPECTRAL: return "Spectral";
    case BO_SET_VOUCHER: return "Voucher";
    case BO_SET_ENHANCED: return "Enhanced";
    case BO_SET_EDITION: return "Edition";
    case BO_SET_BOOSTER: return "Other";
    case BO_SET_TAG: return "Tag";
    case BO_SET_BACK: return "Back";
    default: return "Other";
    }
}

// Replaces each #n# of line with the variable n.
static void fill_line(const char *line, const BOVars *vars, char *out, int size) {
    int o = 0;
    for (const char *p = line; *p && o < size - 1;) {
        if (*p == '#') {
            const char *end = strchr(p + 1, '#');
            if (end) {
                int index = atoi(p + 1) - 1;
                const char *value = index >= 0 && index < vars->count ? vars->values[index] : "ERROR";
                for (const char *q = value; *q && o < size - 1; q++) out[o++] = *q;
                p = end + 1;
                continue;
            }
        }
        out[o++] = *p++;
    }
    out[o] = 0;
}

int bo_describe_item(const BORun *run, const BOItem *item, BODescription *out) {
    memset(out, 0, sizeof(*out));
    const BOCenterInfo *c = bo_item_center(item);
    if (!c) return 0;
    snprintf(out->name, sizeof(out->name), "%s", c->name);
    if (!gText) return 0;
    char path[128];
    const char *key = c->key;
    // The packs share one text for each kind and size, as p_arcana_normal.
    char packKey[64];
    if (c->set == BO_SET_BOOSTER) {
        snprintf(packKey, sizeof(packKey), "%s", c->key);
        char *last = strrchr(packKey, '_');
        if (last) *last = 0;
        key = packKey;
    }
    snprintf(path, sizeof(path), "descriptions.%s.%s", set_key(c->set), key);
    BOLuaValue *entry = bo_luatab_path(gText, path);
    if (!entry) return 0;
    const char *name = bo_luatab_string(entry, "name");
    if (name) snprintf(out->name, sizeof(out->name), "%s", name);
    BOVars vars;
    memset(&vars, 0, sizeof(vars));
    if (c->set == BO_SET_JOKER) joker_vars(run, item, &vars);
    BOLuaValue *text = bo_luatab_get(entry, "text");
    int lines = bo_luatab_length(text);
    for (int i = 0; i < lines && out->line_count < BO_DESCRIBE_LINES; i++) {
        BOLuaValue *line = bo_luatab_item(text, i);
        if (!line || line->type != BO_LUA_STRING) continue;
        fill_line(line->string, &vars, out->lines[out->line_count++], BO_DESCRIBE_LINE_LENGTH);
    }
    return 1;
}
