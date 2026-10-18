#include "bo_challenge_data.h"
#include "bo_profile.h"
#include "bo_describe.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

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

// The name of a card from the text file, as localize name_text.
static const char *center_name(BOCenterId center) {
    const BOCenterInfo *c = bo_center(center);
    if (!c) return "";
    static const char *sets[] = {"Default", "Enhanced", "Edition", "Joker", "Tarot", "Planet", "Spectral",
                                 "Voucher", "Back", "Other", "Tag"};
    char path[128];
    snprintf(path, sizeof(path), "descriptions.%s.%s.name", c->set < 11 ? sets[c->set] : "Other", c->key);
    const char *word = gText ? bo_luatab_string(gText, path) : NULL;
    return word ? word : c->name;
}

#define IS_C(key) (item->center == BO_C_##key)

// The loc_vars of generate_card_ui for a Tarot, a Planet, a Spectral card,
// a pack, or a voucher.
static void other_vars(const BORun *run, const BOItem *item, BOVars *v) {
    const BOCenterInfo *c = bo_item_center(item);
    int has = 0;
    double max = bo_center_number(c, BO_F_MAX_HIGHLIGHTED, &has);
    const char *modConv = bo_center_text(c, BO_F_MOD_CONV);
    const char *suitConv = bo_center_text(c, BO_F_SUIT_CONV);
    switch (c->set) {
    case BO_SET_BOOSTER:
        var_number(v, bo_center_number(c, BO_F_CHOOSE, NULL));
        var_number(v, c->extra);
        break;
    case BO_SET_VOUCHER:
        if ((item->center == BO_V_TAROT_MERCHANT) || (item->center == BO_V_TAROT_TYCOON) || (item->center == BO_V_PLANET_MERCHANT) || (item->center == BO_V_PLANET_TYCOON)) {
            var_number(v, bo_center_number(c, BO_F_EXTRA_DISP, NULL));
        } else if ((item->center == BO_V_SEED_MONEY) || (item->center == BO_V_MONEY_TREE)) {
            var_number(v, c->extra / 5);
        } else {
            var_number(v, c->extra);
        }
        break;
    case BO_SET_SPECTRAL:
        if (IS_C(FAMILIAR) || IS_C(GRIM) || IS_C(INCANTATION) || IS_C(CRYPTID)) var_number(v, c->extra);
        if (IS_C(IMMOLATE)) {
            var_number(v, bo_item_get(item, BO_F_EXTRA_DESTROY));
            var_number(v, bo_item_get(item, BO_F_EXTRA_DOLLARS));
        }
        if (IS_C(ECTOPLASM)) var_number(v, run && run->ecto_minus ? run->ecto_minus : 1);
        break;
    case BO_SET_PLANET: {
        int type = bo_rules_hand_by_name(bo_center_text(c, BO_F_HAND_TYPE));
        if (type < 0) break;
        int level = run ? run->hands[type].level : 1;
        var_number(v, level);
        var_text(v, poker_hand(type));
        var_number(v, bo_hand_level_mult((BOHandType)type, 2) - bo_hand_level_mult((BOHandType)type, 1));
        var_number(v, bo_hand_level_chips((BOHandType)type, 2) - bo_hand_level_chips((BOHandType)type, 1));
        break;
    }
    case BO_SET_TAROT:
        if (IS_C(FOOL)) {
            BOCenterId last = run ? run->last_tarot_planet : BO_CENTER_NONE;
            var_text(v, last != BO_CENTER_NONE ? center_name(last) : bo_describe_word("k_none"));
        } else if (modConv) {
            var_number(v, max);
            var_text(v, center_name(bo_center_by_key(modConv)));
        } else if (suitConv) {
            var_number(v, max);
            var_text(v, suit_word(bo_rules_suit_by_name(suitConv), 1));
        } else if (IS_C(HIGH_PRIESTESS)) {
            var_number(v, bo_center_number(c, BO_F_PLANETS, NULL));
        } else if (IS_C(EMPEROR)) {
            var_number(v, bo_center_number(c, BO_F_TAROTS, NULL));
        } else if (IS_C(HERMIT)) {
            var_number(v, c->extra);
        } else if (IS_C(WHEEL_OF_FORTUNE)) {
            var_number(v, run ? run->probabilities : 1);
            var_number(v, c->extra);
        } else if (IS_C(STRENGTH) || IS_C(HANGED_MAN) || IS_C(DEATH)) {
            var_number(v, max);
        } else if (IS_C(TEMPERANCE)) {
            int money = 0;
            for (int i = 0; run && i < run->joker_count; i++) {
                if (bo_item_center(&run->jokers[i])->set == BO_SET_JOKER) money += run->jokers[i].sell_cost;
            }
            var_number(v, c->extra);
            var_number(v, money < c->extra ? money : c->extra);
        }
        break;
    case BO_SET_BACK:
        // The loc_args of Back:generate_UI, with the config of each deck.
        switch (item->center) {
        case BO_B_RED: var_number(v, 1); break;
        case BO_B_BLUE: var_number(v, 1); break;
        case BO_B_YELLOW: var_number(v, 10); break;
        case BO_B_GREEN: var_number(v, 2); var_number(v, 1); break;
        case BO_B_BLACK: var_number(v, 1); var_number(v, 1); break;
        case BO_B_MAGIC: var_text(v, center_name(BO_V_CRYSTAL_BALL)); var_text(v, center_name(BO_C_FOOL)); break;
        case BO_B_NEBULA: var_text(v, center_name(BO_V_TELESCOPE)); var_number(v, -1); break;
        case BO_B_ZODIAC:
            var_text(v, center_name(BO_V_TAROT_MERCHANT));
            var_text(v, center_name(BO_V_PLANET_MERCHANT));
            var_text(v, center_name(BO_V_OVERSTOCK_NORM));
            break;
        case BO_B_PAINTED: var_number(v, 2); var_number(v, -1); break;
        case BO_B_ANAGLYPH: var_text(v, center_name(BO_TAG_DOUBLE)); break;
        case BO_B_PLASMA: var_number(v, 2); break;
        default: break;
        }
        break;
    default: break;
    }
    (void)has;
}

// Replaces each #n# of line with the variable n.
static void fill_line(const char *line, const BOVars *vars, char *out, int size) {
    int o = 0;
    int boxed = 0;  // The part has an X control: a box with a colour.
    for (const char *p = line; *p && o < size - 1;) {
        if (*p == '{') {
            // The markup goes to out as it is. "{}" ends the part.
            const char *end = strchr(p, '}');
            if (end) {
                const char *x = strstr(p, "X:");
                boxed = x && x < end;
                while (p <= end && o < size - 1) out[o++] = *p++;
                continue;
            }
        }
        // loc_parse_string removes the spaces from the text of a box.
        if (boxed && (*p == ' ' || *p == '\t')) {
            p++;
            continue;
        }
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
    if (c->set == BO_SET_JOKER) {
        joker_vars(run, item, &vars);
    } else {
        other_vars(run, item, &vars);
    }
    BOLuaValue *text = bo_luatab_get(entry, "text");
    int lines = bo_luatab_length(text);
    for (int i = 0; i < lines && out->line_count < BO_DESCRIBE_LINES; i++) {
        BOLuaValue *line = bo_luatab_item(text, i);
        if (!line || line->type != BO_LUA_STRING) continue;
        fill_line(line->string, &vars, out->lines[out->line_count++], BO_DESCRIBE_LINE_LENGTH);
    }
    if (item->center == BO_J_MISPRINT && out->line_count > 0) {
        // main_start of Misprint: "+" and a number from min to max that
        // changes each half second, then "Mult".
        struct timeval now;
        gettimeofday(&now, NULL);
        unsigned step = (unsigned)(now.tv_sec * 2 + now.tv_usec / 500000);
        step = step * 2654435761u >> 16;
        int low = (int)bo_item_get(item, BO_F_EXTRA_MIN), high = (int)bo_item_get(item, BO_F_EXTRA_MAX);
        int value = high > low ? low + (int)(step % (unsigned)(high - low + 1)) : low;
        snprintf(out->lines[0], BO_DESCRIBE_LINE_LENGTH, "{C:mult}+%d{} Mult", value);
    }
    if (item->center == BO_C_FOOL && out->line_count < BO_DESCRIBE_LINES) {
        // main_end of The Fool: the last Tarot or Planet card in a box, green,
        // or red when there is none or it is The Fool.
        BOCenterId last = run ? run->last_tarot_planet : BO_CENTER_NONE;
        int none = last == BO_CENTER_NONE || last == BO_C_FOOL;
        snprintf(out->lines[out->line_count++], BO_DESCRIBE_LINE_LENGTH, "{X:%s,C:white}%s{}", none ? "red" : "green",
                 last != BO_CENTER_NONE ? center_name(last) : bo_describe_word("k_none"));
    }
    return 1;
}

int bo_describe_blind(const BORun *run, int blind, BODescription *out) {
    memset(out, 0, sizeof(*out));
    const BOBlindInfo *info = bo_blind_info((BOBlindId)blind);
    snprintf(out->name, sizeof(out->name), "%s", info->name);
    if (!gText) return 0;
    char path[128];
    snprintf(path, sizeof(path), "descriptions.Blind.%s", info->key);
    BOLuaValue *entry = bo_luatab_path(gText, path);
    if (!entry) return 0;
    const char *name = bo_luatab_string(entry, "name");
    if (name) snprintf(out->name, sizeof(out->name), "%s", name);
    BOVars vars;
    memset(&vars, 0, sizeof(vars));
    // The Ox names the hand that was played most.
    if (blind == BO_BL_OX) var_text(&vars, poker_hand(run ? run->most_played_hand : BO_HAND_HIGH_CARD));
    BOLuaValue *text = bo_luatab_get(entry, "text");
    int lines = bo_luatab_length(text);
    for (int i = 0; i < lines && out->line_count < BO_DESCRIBE_LINES; i++) {
        BOLuaValue *line = bo_luatab_item(text, i);
        if (!line || line->type != BO_LUA_STRING) continue;
        fill_line(line->string, &vars, out->lines[out->line_count++], BO_DESCRIBE_LINE_LENGTH);
    }
    return 1;
}

int bo_describe_tag(const BORun *run, int tag, int orbital_hand, BODescription *out) {
    memset(out, 0, sizeof(*out));
    const BOCenterInfo *c = bo_center((BOCenterId)tag);
    if (!c) return 0;
    snprintf(out->name, sizeof(out->name), "%s", c->name);
    if (!gText) return 0;
    char path[128];
    snprintf(path, sizeof(path), "descriptions.Tag.%s", c->key);
    BOLuaValue *entry = bo_luatab_path(gText, path);
    if (!entry) return 0;
    const char *name = bo_luatab_string(entry, "name");
    if (name) snprintf(out->name, sizeof(out->name), "%s", name);
    // The loc_vars of Tag:get_uibox_table.
    BOVars vars;
    memset(&vars, 0, sizeof(vars));
    switch (tag) {
    case BO_TAG_INVESTMENT: var_number(&vars, bo_center_number(c, BO_F_DOLLARS, NULL)); break;
    case BO_TAG_HANDY: {
        double per = bo_center_number(c, BO_F_DOLLARS_PER_HAND, NULL);
        var_number(&vars, per);
        var_number(&vars, per * (run ? run->hands_played_total : 0));
        break;
    }
    case BO_TAG_GARBAGE: {
        double per = bo_center_number(c, BO_F_DOLLARS_PER_DISCARD, NULL);
        var_number(&vars, per);
        var_number(&vars, per * (run ? run->unused_discards : 0));
        break;
    }
    case BO_TAG_JUGGLE: var_number(&vars, c->h_size); break;
    case BO_TAG_TOP_UP: var_number(&vars, bo_center_number(c, BO_F_SPAWN_JOKERS, NULL)); break;
    case BO_TAG_SKIP: {
        double bonus = bo_center_number(c, BO_F_SKIP_BONUS, NULL);
        var_number(&vars, bonus);
        var_number(&vars, bonus * ((run ? run->skips : 0) + 1));
        break;
    }
    case BO_TAG_ORBITAL:
        var_text(&vars, poker_hand(orbital_hand));
        var_number(&vars, bo_center_number(c, BO_F_LEVELS, NULL));
        break;
    case BO_TAG_ECONOMY: var_number(&vars, bo_center_number(c, BO_F_MAX, NULL)); break;
    default: break;
    }
    BOLuaValue *text = bo_luatab_get(entry, "text");
    int lines = bo_luatab_length(text);
    for (int i = 0; i < lines && out->line_count < BO_DESCRIBE_LINES; i++) {
        BOLuaValue *line = bo_luatab_item(text, i);
        if (!line || line->type != BO_LUA_STRING) continue;
        fill_line(line->string, &vars, out->lines[out->line_count++], BO_DESCRIBE_LINE_LENGTH);
    }
    return 1;
}

// The name of a deck or a stake from the text, as localize name_text.
static const char *text_name(const char *set, const char *key, const char *fallback) {
    char path[128];
    snprintf(path, sizeof(path), "descriptions.%s.%s.name", set, key);
    const char *name = gText ? bo_luatab_string(gText, path) : NULL;
    return name ? name : fallback;
}

int bo_describe_locked_back(int back, BODescription *out) {
    memset(out, 0, sizeof(*out));
    const BOCenterInfo *c = bo_center((BOCenterId)back);
    if (!c || !c->unlock_type) return 0;
    BOVars vars;
    memset(&vars, 0, sizeof(vars));
    const char *key;
    static const char *stakeKeys[8] = {"stake_white", "stake_red", "stake_green", "stake_black",
                                        "stake_blue", "stake_purple", "stake_orange", "stake_gold"};
    if (strcmp(c->unlock_type, "win_deck") == 0) {
        BOCenterId other = bo_center_by_key(c->unlock_text);
        // The name of the other deck shows only when it is unlocked.
        var_text(&vars, bo_profile_unlocked(other) ? text_name("Back", c->unlock_text, bo_center(other)->name)
                                                   : bo_describe_word("k_unknown"));
        key = "deck_locked_win";
    } else if (strcmp(c->unlock_type, "discover_amount") == 0) {
        var_number(&vars, c->unlock_number);
        key = "deck_locked_discover";
    } else if (strcmp(c->unlock_type, "win_stake") == 0) {
        int stake = (int)c->unlock_number;
        var_text(&vars, text_name("Stake", stakeKeys[stake >= 1 && stake <= 8 ? stake - 1 : 0], "Stake"));
        key = "deck_locked_stake";
    } else {
        return 0;
    }
    snprintf(out->name, sizeof(out->name), "%s", "Locked");
    if (!gText) return 0;
    char path[128];
    snprintf(path, sizeof(path), "descriptions.Other.%s", key);
    BOLuaValue *entry = bo_luatab_path(gText, path);
    if (!entry) return 0;
    const char *name = bo_luatab_string(entry, "name");
    if (name) snprintf(out->name, sizeof(out->name), "%s", name);
    BOLuaValue *text = bo_luatab_get(entry, "text");
    int lines = bo_luatab_length(text);
    for (int i = 0; i < lines && out->line_count < BO_DESCRIBE_LINES; i++) {
        BOLuaValue *line = bo_luatab_item(text, i);
        if (!line || line->type != BO_LUA_STRING) continue;
        fill_line(line->string, &vars, out->lines[out->line_count++], BO_DESCRIBE_LINE_LENGTH);
    }
    return 1;
}

// number_format: a whole number with commas, as "1,000,000".
static void comma_number(double value, char *out, int size) {
    char digits[32];
    snprintf(digits, sizeof(digits), "%.0f", value);
    int len = (int)strlen(digits), o = 0;
    for (int i = 0; i < len && o < size - 1; i++) {
        out[o++] = digits[i];
        if ((len - i - 1) % 3 == 0 && i < len - 1 && o < size - 1) out[o++] = ',';
    }
    out[o] = 0;
}

// The lines of a text entry at path, as localize.
static int fill_from(const char *path, const char *lines_key, const BOVars *vars, BODescription *out) {
    BOLuaValue *entry = gText ? bo_luatab_path(gText, path) : NULL;
    if (!entry) return 0;
    BOLuaValue *text = bo_luatab_get(entry, lines_key);
    int lines = bo_luatab_length(text);
    for (int i = 0; i < lines && out->line_count < BO_DESCRIBE_LINES; i++) {
        BOLuaValue *line = bo_luatab_item(text, i);
        if (!line || line->type != BO_LUA_STRING) continue;
        fill_line(line->string, vars, out->lines[out->line_count++], BO_DESCRIBE_LINE_LENGTH);
    }
    return 1;
}

int bo_describe_locked(int center, BODescription *out) {
    memset(out, 0, sizeof(*out));
    const BOCenterInfo *c = bo_center((BOCenterId)center);
    if (!c) return 0;
    snprintf(out->name, sizeof(out->name), "%s", bo_describe_word("k_locked"));
    BOVars vars;
    memset(&vars, 0, sizeof(vars));
    const char *type = c->unlock_type ? c->unlock_type : "";
    char text[64];
    if (strcmp(type, "modify_deck") == 0 && c->unlock_kind == 1) {
        var_number(&vars, c->unlock_number);
        char path[64];
        snprintf(path, sizeof(path), "misc.suits_singular.%s", c->unlock_text);
        const char *suit = gText ? bo_luatab_string(gText, path) : NULL;
        var_text(&vars, suit ? suit : c->unlock_text);
    } else if (strcmp(type, "modify_deck") == 0 && c->unlock_kind == 2) {
        var_number(&vars, c->unlock_number);
        var_text(&vars, text_name("Enhanced", c->unlock_text, c->unlock_text));
    } else if (strcmp(type, "chip_score") == 0) {
        comma_number(c->unlock_number, text, sizeof(text));
        var_text(&vars, text);
    } else if (c->unlock_text && strcmp(type, "win_deck") != 0) {
        var_text(&vars, c->unlock_text);
    } else {
        var_number(&vars, c->unlock_number);
    }
    // The career stats and the Blank redeems show how far the player is.
    const BOProfile *p = bo_profile_active();
    if (p && strncmp(type, "c_", 2) == 0) {
        static const char *stats[BO_STAT_COUNT] = {
            "c_round_interest_cap_streak", "c_dollars_earned", "c_shop_dollars_spent", "c_tarots_bought",
            "c_planets_bought", "c_playing_cards_bought", "c_vouchers_bought", "c_tarot_reading_used",
            "c_planetarium_used", "c_shop_rerolls", "c_cards_played", "c_cards_discarded", "c_losses", "c_wins",
            "c_rounds", "c_hands_played", "c_face_cards_played", "c_jokers_sold", "c_cards_sold",
            "c_single_hand_round_streak",
        };
        for (int i = 0; i < BO_STAT_COUNT; i++) {
            if (strcmp(type, stats[i]) == 0) var_number(&vars, p->stats[i]);
        }
    } else if (p && strcmp(type, "interest_streak") == 0) {
        var_number(&vars, p->stats[BO_STAT_ROUND_INTEREST_CAP_STREAK]);
    } else if (p && strcmp(type, "blank_redeems") == 0) {
        var_number(&vars, p->blank_redeems);
    }
    char path[128];
    // A legendary joker comes only from The Soul.
    if (c->set == BO_SET_JOKER && c->rarity == 4) {
        return fill_from("descriptions.Other.joker_locked_legendary", "text", &vars, out);
    }
    snprintf(path, sizeof(path), "descriptions.%s.%s", set_key(c->set), c->key);
    return fill_from(path, "unlock", &vars, out);
}

int bo_describe_undiscovered(int center, BODescription *out) {
    memset(out, 0, sizeof(*out));
    const BOCenterInfo *c = bo_center((BOCenterId)center);
    if (!c) return 0;
    static const char *keys[] = {"joker", "tarot", "planet", "spectral", "voucher", "booster", "edition", "tag"};
    static const int sets[] = {BO_SET_JOKER, BO_SET_TAROT, BO_SET_PLANET, BO_SET_SPECTRAL, BO_SET_VOUCHER,
                               BO_SET_BOOSTER, BO_SET_EDITION, BO_SET_TAG};
    const char *key = NULL;
    for (int i = 0; i < 8; i++) {
        if (c->set == sets[i]) key = keys[i];
    }
    if (!key) return 0;
    char path[96];
    snprintf(path, sizeof(path), "descriptions.Other.undiscovered_%s", key);
    snprintf(out->name, sizeof(out->name), "%s", "Not Discovered");
    BOLuaValue *entry = gText ? bo_luatab_path(gText, path) : NULL;
    const char *name = entry ? bo_luatab_string(entry, "name") : NULL;
    if (name) snprintf(out->name, sizeof(out->name), "%s", name);
    BOVars vars;
    memset(&vars, 0, sizeof(vars));
    return fill_from(path, "text", &vars, out);
}

int bo_describe_challenge(int index, BODescription *out) {
    memset(out, 0, sizeof(*out));
    const BOChallengeInfo *ch = bo_challenge(index);
    if (!ch) return 0;
    char path[128];
    snprintf(path, sizeof(path), "misc.challenge_names.%s", ch->id);
    const char *name = gText ? bo_luatab_string(gText, path) : NULL;
    snprintf(out->name, sizeof(out->name), "%s", name ? name : ch->name);
    if (!gText) return 0;
    for (int pass = 0; pass < 2; pass++) {
        const BOChallengeRule *rules = pass == 0 ? ch->custom : ch->modifiers;
        int count = pass == 0 ? ch->custom_count : ch->modifier_count;
        for (int i = 0; i < count && out->line_count < BO_DESCRIBE_LINES; i++) {
            snprintf(path, sizeof(path), "misc.v_text.%s_%s", pass == 0 ? "ch_c" : "ch_m", rules[i].id);
            BOLuaValue *lines = bo_luatab_path(gText, path);
            BOLuaValue *line = lines ? bo_luatab_item(lines, 0) : NULL;
            if (!line || line->type != BO_LUA_STRING) continue;
            BOVars vars;
            memset(&vars, 0, sizeof(vars));
            if (rules[i].text) {
                var_text(&vars, rules[i].text);
            } else {
                var_number(&vars, rules[i].number);
            }
            fill_line(line->string, &vars, out->lines[out->line_count++], BO_DESCRIBE_LINE_LENGTH);
        }
    }
    return 1;
}
