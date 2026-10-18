#include "bo_game.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bo_blind.h"
#include "bo_boss.h"
#include "bo_card.h"
#include "bo_debug.h"
#include "bo_describe.h"
#include "bo_font.h"
#include "bo_gfx.h"
#include "bo_hand.h"
#include "bo_image.h"
#include "bo_joker.h"
#include "bo_luatab.h"
#include "bo_motion.h"
#include "bo_pack.h"
#include "bo_rich.h"
#include "bo_rules.h"
#include "bo_tag.h"
#include "bo_run.h"
#include "bo_profile.h"
#include "bo_save.h"
#include "bo_score.h"
#include "bo_ui.h"
#include "bo_voucher.h"
#include "bo_zip.h"

// The width of one card in the 2x atlases of Balatro.
static const float kAtlasCardW = 142;

// The colours of Balatro, from globals.lua.
#define BO_C_BLUE 0x009dff
#define BO_C_RED 0xfe5f55
#define BO_C_ORANGE 0xfda200
#define BO_C_GREEN 0x4bc292
#define BO_C_BLACK 0x374244
#define BO_C_L_BLACK 0x4f6367
#define BO_C_GREY 0x5f7377
#define BO_C_DARK 0x1e2b2d
#define BO_C_MONEY 0xf3b958
#define BO_C_WHITE 0xffffff
#define BO_C_TEXT_DARK 0x4f6367

// The ids of the widgets.
enum {
    BO_ID_PLAY = 1,
    BO_ID_DISCARD,
    BO_ID_SORT_RANK,
    BO_ID_SORT_SUIT,
    BO_ID_SELECT_BLIND,
    BO_ID_CASH_OUT,
    BO_ID_NEW_RUN,
    BO_ID_ENDLESS,
    BO_ID_NEXT_ROUND,
    BO_ID_REROLL,
    BO_ID_ACTION,      // The first button of the focused card: Buy, Use, or Sell.
    BO_ID_ACTION_2,    // The second button: Sell.
    BO_ID_SKIP,
    BO_ID_REROLL_BOSS,
    BO_ID_SKIP_BLIND,
    BO_ID_RUN_INFO,
    BO_ID_SETUP_PREV,  // The run setup: the deck before this one.
    BO_ID_SETUP_NEXT,
    BO_ID_SETUP_PLAY,
    BO_ID_STAKE_PREV,
    BO_ID_STAKE_NEXT,
    BO_ID_INFO_NEW_RUN,  // Run Info: leave this run for a new one.
    BO_ID_SETUP_BACK,    // The run setup: back to the run that is on.
    BO_ID_UNLOCK_ALL,    // The run setup: unlock all cards, with a second tap.
    BO_ID_COLLECTION,    // The run setup: open the collection.
    BO_ID_COL_CLOSE,
    BO_ID_COL_PREV,
    BO_ID_COL_NEXT,
    BO_ID_COL_TAB = 600,   // A tab of the collection.
    BO_ID_COL_CARD = 620,  // A card of the collection page.
    BO_ID_BACKGROUND,  // A tap on nothing: the focus goes away.
    BO_ID_HAND_CARD = 100,
    BO_ID_JOKER = 200,
    BO_ID_CONSUMABLE = 300,
    BO_ID_SHOP = 400,
    BO_ID_VOUCHER = 450,
    BO_ID_PACK = 460,
    BO_ID_PACK_CARD = 470,
    BO_ID_BLIND_TAG = 480,
    BO_ID_HELD_TAG = 490,
    BO_ID_TAG_VOUCHER = 530,
};

// The areas that a card can be in, for the focus.
typedef enum {
    BO_FOCUS_NONE,
    BO_FOCUS_JOKER,
    BO_FOCUS_CONSUMABLE,
    BO_FOCUS_SHOP,
    BO_FOCUS_VOUCHER,
    BO_FOCUS_PACK,       // A pack for sale in the shop.
    BO_FOCUS_PACK_CARD,  // A card of the open pack.
    BO_FOCUS_BLIND_TAG,  // The tag of a blind on the blind select.
    BO_FOCUS_HELD_TAG,   // A tag that waits.
    BO_FOCUS_TAG_VOUCHER,  // A voucher of a Voucher Tag.
} BOFocusArea;

typedef struct {
    BOCard card;
    BOMotion m;      // The place that shows, in the units of the game.
    float tx, ty;    // The place that the card goes to, in pixels.
    float tr;        // The angle that the card goes to.
    float delay;     // A new card waits on the deck for this time.
    int fan;         // The card is in the hand: it fans and sways.
    int selected;
} BOCardView;

// The place of a joker or a consumable on the screen. It follows its card
// by the sort_id, so that a joker keeps its place when another one leaves.
typedef struct {
    uint32_t sort_id;
    float x, y;  // The place that shows, in pixels.
    BOMotion m;
} BOItemView;

typedef struct {
    char text[24];
    float x, y;
    float age;
    unsigned int color;
} BOPopup;

typedef enum {
    BO_FLOW_IDLE,   // The screen follows the state of the run.
    BO_FLOW_SCORE,  // The played cards score, one event at a time.
    BO_FLOW_CLEAR,  // The played cards leave.
} BOFlow;

static struct {
    char error[256];
    int width, height;
    float cardW, cardH;
    BOTexture deck;
    BOTexture centers;   // Enhancers.png: fronts, backs, seals.
    BOTexture chips;     // BlindChips.png: 21 frames for each blind.
    BOTexture jokers;    // Jokers.png.
    BOTexture tarots;    // Tarots.png: Tarot, Planet, and Spectral cards.
    BOTexture vouchers;  // Vouchers.png.
    BOTexture boosters;  // boosters.png.
    BOTexture tags;      // tags.png.
    BOTexture stickers;  // stickers.png: Eternal, Perishable, Rental, and the stakes.
    BOTexture stakes;    // chips.png: the chip of each stake.
    int effects;         // The shaders of the editions compiled.
    // The background: its three colours, its contrast, and its spin.
    BOColor backC, backL, backD;
    float backContrast, backSpin, backSpinTime;
    BOLuaValue *text;    // localization/en-us.lua.

    BORun *run;
    BOCardView hand[BO_MAX_HAND];
    int handCount;
    BOCardView play[BO_MAX_PLAY + 8];
    int playCount;
    BOItemView jokerViews[BO_MAX_JOKERS];
    int jokerViewCount;
    BOItemView consumableViews[BO_MAX_CONSUMABLES];
    int consumableViewCount;

    BOScore score;
    int nextEvent;
    BOFlow flow;
    double wait;
    double shownChips, shownMult;
    BOMotion chipsBox, multBox;  // The boxes shake when their number changes.
    float jiggle;                // G.ROOM.jiggle: the shake of the room.
    const char *shownHand;
    double shownRound;  // The round score on the screen. It counts up.
    double shownDollars;
    BOPopup popups[24];
    int endless;  // The player goes on after the win.

    BOFocusArea focus;
    int focusIndex;
    int showInfo;  // The Run Info screen is open.
    int setup;              // The run setup is open: the choice of the deck.
    BOCenterId setupBack;   // The deck of the next run.
    int setupStake;         // The stake of the next run, from 1 to 8.
    char savePath[1024];    // save.b4s beside Balatro.love.
    BORun *saved;           // The run as the save file has it.
    int savedEndless;
    BOProfile *profile;     // The profile of the player: profile.b4p.
    BOProfile *savedProfile;
    char profilePath[1024];
    BOCenterId notice;      // A card that the profile unlocked, on the screen.
    float noticeAge;
    float unlockConfirm;    // Unlock All waits for a second tap this long.
    int collection;         // The collection is open.
    int colTab, colPage, colFocus;  // The tab, the page, and the card that shows its text.
    float blindTagX[2], blindTagY[2];  // The tags of the blind select.

    // A drag of a joker.
    int dragIndex;  // -1 when no joker is held.
    int dragging;
    float dragStartX, dragStartY, dragX;

    double time;
    int frames;
    double fpsTime;
} G = {.dragIndex = -1};

static void fail(const char *message) {
    snprintf(G.error, sizeof(G.error), "%s", message);
}

const char *bo_game_error(void) {
    return G.error;
}

static BOTexture load_texture(BOZip *zip, const char *name, float scale) {
    size_t size = 0;
    unsigned char *png = bo_zip_read(zip, name, &size);
    BOTexture texture = {0, 0, 0};
    if (png) {
        texture = bo_image_texture(png, size, scale);
        free(png);
    }
    return texture;
}

// --- The areas of the screen ---------------------------------------------

static float panel_width(void) {
    return G.width * 0.25f;
}

static float hand_top(void) {
    return G.height * 0.60f;
}

static float deck_x(void) {
    return G.width - G.cardW - G.width * 0.02f;
}

static BORect main_area(void) {
    float left = panel_width() + G.width * 0.02f;
    return bo_rect(left, 0, deck_x() - G.width * 0.02f - left, G.height);
}

static BORect joker_area(void) {
    float left = panel_width() + G.width * 0.02f;
    return bo_rect(left, G.height * 0.02f, G.width * 0.72f - left, G.cardH);
}

static BORect consumable_area(void) {
    BORect jokers = joker_area();
    float left = jokers.x + jokers.w + G.width * 0.02f;
    return bo_rect(left, jokers.y, G.width - G.width * 0.01f - left, G.cardH);
}

// The x of card i of count in an area, as CardArea:align_cards spreads them.
static float slot_x(BORect area, int i, int count) {
    float step = count > 1 ? (area.w - G.cardW) / (count - 1) : 0;
    if (step > G.cardW * 1.05f) step = G.cardW * 1.05f;
    float total = step * (count - 1) + G.cardW;
    return area.x + (area.w - total) / 2 + step * i;
}

// The pixels of one unit of the game.
static float unit(void) {
    return G.cardW / (float)BO_MOTION_CARD_W;
}

// With fan, the cards of the hand turn and drop as align_cards puts them.
static void layout_row(BOCardView *cards, int count, BORect area, float y, float lift, int fan) {
    for (int i = 0; i < count; i++) {
        cards[i].tx = slot_x(area, i, count);
        cards[i].ty = y - (cards[i].selected ? lift : 0);
        cards[i].tr = 0;
        cards[i].fan = fan;
        if (fan) {
            cards[i].ty += bo_motion_fan_drop(i, count) * unit();
            cards[i].tr = bo_motion_fan_angle(i, count, 0.2f);
        }
    }
}

static void layout(void) {
    layout_row(G.hand, G.handCount, main_area(), hand_top(), G.cardH * 0.2f, 1);
    if (G.flow != BO_FLOW_CLEAR) layout_row(G.play, G.playCount, main_area(), G.height * 0.30f, G.cardH * 0.12f, 0);
}

// Moves a card one frame. A card of the hand sways a little, as in
// align_cards.
static void move_view(BOCardView *v, double dt) {
    float u = unit();
    if (v->delay > 0) {
        v->delay -= (float)dt;
        return;
    }
    float tx = v->tx / u, ty = v->ty / u, tr = v->tr;
    if (v->fan) {
        tr += 0.02f * sinf(2 * (float)G.time + tx);
        ty += 0.03f * sinf(0.666f * (float)G.time + tx);
    }
    bo_motion_step(&v->m, tx, ty, tr, 1, dt, G.time);
}

// Makes the cards on the screen follow the hand of the run. A card that
// stays keeps its place, and a new card comes from the deck.
static void update_preview(void);

static void sync_hand(void) {
    BOCardView views[BO_MAX_HAND];
    int count = 0, drawn = 0;
    for (int i = 0; i < G.run->hand_count; i++) {
        BOCardView view;
        memset(&view, 0, sizeof(view));
        view.card = G.run->hand[i];
        // A new card comes from the deck. The draws come one after another.
        bo_motion_place(&view.m, deck_x() / unit(), hand_top() / unit(), 0);
        view.delay = 0.001f + 0.07f * drawn;
        for (int j = 0; j < G.handCount; j++) {
            if (G.hand[j].card.uid == view.card.uid) {
                view = G.hand[j];
                view.card = G.run->hand[i];
                break;
            }
        }
        if (view.delay > 0) drawn++;
        // Cerulean Bell: the forced card stays selected.
        if (view.card.forced_selection) view.selected = 1;
        views[count++] = view;
    }
    memcpy(G.hand, views, sizeof(BOCardView) * count);
    G.handCount = count;
    if (G.flow == BO_FLOW_IDLE) update_preview();
    layout();
}

// Keeps a view for each joker and consumable, in the order of the run.
static void sync_items(BOItemView *views, int *viewCount, const BOItem *items, int count, BORect area) {
    BOItemView next[BO_MAX_JOKERS];
    for (int i = 0; i < count && i < BO_MAX_JOKERS; i++) {
        BOItemView view = {items[i].sort_id, slot_x(area, i, count), area.y, {0}};
        bo_motion_place(&view.m, view.x / unit(), view.y / unit(), 0);
        for (int j = 0; j < *viewCount; j++) {
            if (views[j].sort_id == items[i].sort_id) {
                view = views[j];
                break;
            }
        }
        next[i] = view;
    }
    memcpy(views, next, sizeof(BOItemView) * count);
    *viewCount = count;
}

static void popup(const char *text, float x, float y, unsigned int color) {
    for (int i = 0; i < 24; i++) {
        if (G.popups[i].age <= 0 || G.popups[i].age > 1.2f) {
            snprintf(G.popups[i].text, sizeof(G.popups[i].text), "%s", text);
            G.popups[i].x = x;
            G.popups[i].y = y;
            G.popups[i].age = 0.001f;
            G.popups[i].color = color;
            return;
        }
    }
}

// The views of the screen for the run in G.run, after a new start or a
// load.
static void reset_views(void) {
    G.handCount = 0;
    G.playCount = 0;
    G.jokerViewCount = 0;
    G.consumableViewCount = 0;
    G.flow = BO_FLOW_IDLE;
    G.shownHand = NULL;
    G.shownRound = G.run->chips;
    G.shownDollars = G.run->dollars;
    G.focus = BO_FOCUS_NONE;
    bo_motion_place(&G.chipsBox, 0, 0, 0);
    bo_motion_place(&G.multBox, 0, 0, 0);
}

// Writes the profile when it changed.
static void save_profile(void) {
    if (!G.profile || !G.profilePath[0]) return;
    if (G.savedProfile && memcmp(G.savedProfile, G.profile, sizeof(BOProfile)) == 0) return;
    if (!G.savedProfile) G.savedProfile = malloc(sizeof(BOProfile));
    if (!G.savedProfile) return;
    memcpy(G.savedProfile, G.profile, sizeof(BOProfile));
    if (!bo_profile_write(G.profile, G.profilePath)) fprintf(stderr, "profile: cannot write %s\n", G.profilePath);
}

// Writes the run when it changed and no animation is on, as save_run.
// A lost run has no save.
static void autosave(void) {
    save_profile();
    if (!G.savePath[0] || G.setup || G.flow != BO_FLOW_IDLE || G.dragging) return;
    if (G.saved && memcmp(G.saved, G.run, sizeof(BORun)) == 0 && G.savedEndless == G.endless) return;
    if (!G.saved) G.saved = malloc(sizeof(BORun));
    if (!G.saved) return;
    memcpy(G.saved, G.run, sizeof(BORun));
    G.savedEndless = G.endless;
    if (G.run->state == BO_STATE_GAME_OVER) {
        bo_save_remove(G.savePath);
        return;
    }
    BOSaveExtra extra = {G.endless};
    if (!bo_save_write(G.run, &extra, G.savePath)) fprintf(stderr, "save: cannot write %s\n", G.savePath);
}

static void new_run(void) {
    // Balatro makes the seed from the place and the time of the cursor.
    // Here the time of the clock gives the number.
    char seed[9];
    bo_random_string((double)time(NULL) * 0.33411983 + fmod(G.time, 1000) * 0.412311010, seed);
    bo_run_start_deck(G.run, seed, G.setupBack ? G.setupBack : BO_B_RED, G.setupStake ? G.setupStake : 1);
    reset_views();
    G.endless = 0;
    fprintf(stderr, "new run: seed %s, %s, stake %d\n", seed, bo_center(G.run->back)->key, G.run->stake);
}

int bo_game_start(const char *love_path, int width, int height) {
    freopen("/tmp/Balatro4S.log", "w", stderr);
    G.width = width;
    G.height = height;
    BOZip *zip = bo_zip_open(love_path);
    if (!zip) {
        fail("Copy Balatro.love from your copy of Balatro to\n/var/mobile/Media/Balatro4S/Balatro.love\n\n"
             "Balatro4S needs it for the art, the sounds, and the font.");
        return 0;
    }
    char *version = (char *)bo_zip_read(zip, "version.jkr", NULL);
    fprintf(stderr, "Balatro.love version: %s\n", version ? version : "(none)");
    free(version);

    if (!bo_gfx_init()) {
        fail("OpenGL ES 2.0 does not start.");
        bo_zip_close(zip);
        return 0;
    }

    // Balatro sizes the room in tiles. A card is 2.05 tiles wide. A tile
    // is 1/20 of the width of the screen.
    float tile = width / 20.0f;
    G.cardW = roundf(tile * 2.4f * 35 / 41);
    G.cardH = roundf(tile * 2.4f * 47 / 41);
    float scale = G.cardW / kAtlasCardW;
    G.deck = load_texture(zip, "resources/textures/2x/8BitDeck.png", scale);
    G.centers = load_texture(zip, "resources/textures/2x/Enhancers.png", scale);
    G.chips = load_texture(zip, "resources/textures/2x/BlindChips.png", 1);
    G.jokers = load_texture(zip, "resources/textures/2x/Jokers.png", scale);
    G.tarots = load_texture(zip, "resources/textures/2x/Tarots.png", scale);
    G.vouchers = load_texture(zip, "resources/textures/2x/Vouchers.png", scale);
    G.boosters = load_texture(zip, "resources/textures/2x/boosters.png", scale);
    G.tags = load_texture(zip, "resources/textures/2x/tags.png", scale);
    G.stickers = load_texture(zip, "resources/textures/2x/stickers.png", scale);
    G.stakes = load_texture(zip, "resources/textures/2x/chips.png", 1);

    size_t fontSize = 0;
    unsigned char *font = bo_zip_read(zip, "resources/fonts/m6x11plus.ttf", &fontSize);
    int fontLoaded = font && bo_font_load(font, fontSize, 48);
    free(font);

    // The shaders of the editions and the shines, from Balatro.love.
    static const struct {
        BOShaderEffect effect;
        const char *name;
    } kEffects[] = {{BO_FX_FOIL, "foil"}, {BO_FX_HOLO, "holo"}, {BO_FX_POLYCHROME, "polychrome"},
                    {BO_FX_NEGATIVE, "negative"}, {BO_FX_NEGATIVE_SHINE, "negative_shine"},
                    {BO_FX_VOUCHER, "voucher"}, {BO_FX_BOOSTER, "booster"},
                    {BO_FX_BACKGROUND, "background"}};
    int loaded = 0;
    for (size_t i = 0; i < sizeof(kEffects) / sizeof(kEffects[0]); i++) {
        char path[96];
        snprintf(path, sizeof(path), "resources/shaders/%s.fs", kEffects[i].name);
        char *source = (char *)bo_zip_read(zip, path, NULL);
        if (source && bo_gfx_load_effect(kEffects[i].effect, kEffects[i].name, source)) loaded++;
        free(source);
    }
    G.effects = loaded == (int)(sizeof(kEffects) / sizeof(kEffects[0]));
    fprintf(stderr, "effects: %d compiled\n", loaded);

    char *text = (char *)bo_zip_read(zip, "localization/en-us.lua", NULL);
    if (text) {
        G.text = bo_luatab_parse(text);
        free(text);
    }
    bo_describe_set_text(G.text);
    bo_zip_close(zip);
    if (!G.deck.id || !G.centers.id || !G.chips.id || !G.jokers.id || !fontLoaded) {
        fail("Balatro.love does not have the textures or the font that Balatro4S needs.");
        return 0;
    }
    fprintf(stderr, "text: %s\n", G.text ? "loaded" : "missing");
    G.run = calloc(1, sizeof(BORun));
    G.setupBack = BO_B_RED;
    G.setupStake = 1;
    // The save and the profile are beside Balatro.love. A saved run goes on
    // at once.
    const char *slash = strrchr(love_path, '/');
    if (slash) snprintf(G.savePath, sizeof(G.savePath), "%.*s/save.b4s", (int)(slash - love_path), love_path);
    if (slash) {
        snprintf(G.profilePath, sizeof(G.profilePath), "%.*s/profile.b4p", (int)(slash - love_path), love_path);
    }
    G.profile = malloc(sizeof(BOProfile));
    if (G.profile) {
        if (!G.profilePath[0] || !bo_profile_read(G.profile, G.profilePath)) bo_profile_init(G.profile);
        bo_profile_set_active(G.profile);
    }
    G.notice = BO_CENTER_NONE;
    BOSaveExtra extra = {0};
    if (G.savePath[0] && bo_save_read(G.run, &extra, G.savePath)) {
        G.endless = extra.endless;
        G.setupBack = G.run->back;
        G.setupStake = G.run->stake;
        reset_views();
        if (G.run->state == BO_STATE_SELECTING || G.run->state == BO_STATE_PACK) sync_hand();
        fprintf(stderr, "save: resumed %s, ante %d\n", G.run->random.seed, G.run->ante);
        BOUnlockArgs resumed = {.type = BO_UNLOCK_CONTINUE_GAME};
        bo_profile_check(G.run, &resumed);
    } else {
        G.setup = 1;
        new_run();
    }
    return 1;
}

// --- The drawing of cards -------------------------------------------------

// The row of each suit in 8BitDeck.png.
static int deck_row(int suit) {
    switch (suit) {
    case BO_HEARTS: return 0;
    case BO_CLUBS: return 1;
    case BO_DIAMONDS: return 2;
    default: return 3;
    }
}

static void draw_cell(const BOTexture *atlas, int columns, int rows, int col, int row, float x, float y, float w,
                      float h, BOColor tint) {
    if (!atlas->id || col < 0 || row < 0) return;
    float cw = atlas->width / (float)columns, ch = atlas->height / (float)rows;
    bo_gfx_sprite(atlas, cw * col, ch * row, cw, ch, x, y, w, h, 0, tint);
}

// The sprites that show an edition, until the shaders of 0.7: a colour
// over the card.
static void draw_edition(int edition, float x, float y, float w, float h) {
    switch (edition) {
    case BO_EDITION_FOIL: bo_gfx_round_rect(x, y, w, h, w * 0.06f, bo_rgba(0x9ad6ff, 0.28f)); break;
    case BO_EDITION_HOLO: bo_gfx_round_rect(x, y, w, h, w * 0.06f, bo_rgba(0xff8ad2, 0.25f)); break;
    case BO_EDITION_POLYCHROME: {
        float t = (float)G.time;
        BOColor c = {0.5f + 0.5f * sinf(t * 2), 0.5f + 0.5f * sinf(t * 2 + 2.1f), 0.5f + 0.5f * sinf(t * 2 + 4.2f), 0.28f};
        bo_gfx_round_rect(x, y, w, h, w * 0.06f, c);
        break;
    }
    case BO_EDITION_NEGATIVE: bo_gfx_round_rect(x, y, w, h, w * 0.06f, bo_rgba(0x2a1a5e, 0.45f)); break;
    default: break;
    }
}

// One sprite of a card: the cell (col, row) of an atlas of columns x rows.
typedef struct {
    const BOTexture *texture;
    int columns, rows, col, row;
} BOLayer;

static void draw_layer(const BOLayer *l, float x, float y, float w, float h) {
    draw_cell(l->texture, l->columns, l->rows, l->col, l->row, x, y, w, h, bo_rgb(BO_C_WHITE));
}

static void effect_layer(BOShaderEffect effect, const BOLayer *l, float x, float y, float w, float h) {
    // Card:draw sends (time / 28, time) to the shader.
    float t = (float)G.time;
    bo_gfx_effect_sprite(effect, l->texture, l->columns, l->rows, l->col, l->row, x, y, w, h, t / 28, t, t);
}

// The base layers of a card with its edition, as Card:draw: a Negative card
// draws with its shader, and the other editions draw over the card.
static void draw_with_edition(const BOLayer *layers, int count, int edition, int shine, float x, float y, float w,
                              float h) {
    if (!G.effects) {
        for (int i = 0; i < count; i++) draw_layer(&layers[i], x, y, w, h);
        draw_edition(edition, x, y, w, h);
        return;
    }
    for (int i = 0; i < count; i++) {
        if (edition == BO_EDITION_NEGATIVE) {
            effect_layer(BO_FX_NEGATIVE, &layers[i], x, y, w, h);
        } else {
            draw_layer(&layers[i], x, y, w, h);
        }
    }
    if (shine >= 0) effect_layer((BOShaderEffect)shine, &layers[0], x, y, w, h);
    BOShaderEffect over = BO_FX_COUNT;
    if (edition == BO_EDITION_HOLO) over = BO_FX_HOLO;
    if (edition == BO_EDITION_FOIL) over = BO_FX_FOIL;
    if (edition == BO_EDITION_POLYCHROME) over = BO_FX_POLYCHROME;
    if (over != BO_FX_COUNT) {
        for (int i = 0; i < count; i++) effect_layer(over, &layers[i], x, y, w, h);
    }
    if (edition == BO_EDITION_NEGATIVE) effect_layer(BO_FX_NEGATIVE_SHINE, &layers[0], x, y, w, h);
}

static void draw_shadow(float x, float y, float w, float h) {
    bo_gfx_round_rect(x + G.cardW * 0.04f, y + G.cardH * 0.04f, w, h, G.cardW * 0.06f, bo_rgba(0x000000, 0.25f));
}

// The back of the deck of the run, from its pos in Enhancers.png.
static void draw_back(float x, float y, float w, float h) {
    const BOCenterInfo *back = bo_center(G.run->back);
    int col = back ? back->pos_x : 0, row = back ? back->pos_y : 0;
    draw_cell(&G.centers, 7, 5, col, row, x, y, w, h, bo_rgb(BO_C_WHITE));
}

static void draw_playing_card(const BOCard *card, float x, float y, float w, float h) {
    draw_shadow(x, y, w, h);
    if (card->face_down) {
        draw_back(x, y, w, h);
        return;
    }
    const BOCenterInfo *center = bo_center(bo_rules_enhancement_center(card->enhancement));
    BOColor white = bo_rgb(BO_C_WHITE);
    BOLayer layers[2] = {{&G.centers, 7, 5, center->pos_x, center->pos_y},
                         {&G.deck, 13, 4, card->rank - 2, deck_row(card->suit)}};
    draw_with_edition(layers, bo_card_is_stone(card) ? 1 : 2, card->edition, -1, x, y, w, h);
    // The seals of shared_seals in game.lua. The Gold Seal shines.
    static const int sealPos[5][2] = {{-1, -1}, {2, 0}, {5, 4}, {6, 4}, {4, 4}};
    if (card->seal) {
        BOLayer seal = {&G.centers, 7, 5, sealPos[card->seal][0], sealPos[card->seal][1]};
        draw_cell(&G.centers, 7, 5, seal.col, seal.row, x, y, w, h, white);
        if (card->seal == BO_SEAL_GOLD && G.effects) effect_layer(BO_FX_VOUCHER, &seal, x, y, w, h);
    }
    if (card->debuffed) bo_gfx_round_rect(x, y, w, h, w * 0.06f, bo_rgba(0x000000, 0.45f));
}

static void draw_view(const BOCardView *view) {
    float u = unit(), x = view->m.x * u, y = view->m.y * u;
    bo_gfx_transform(view->m.r, view->m.scale, x + G.cardW / 2, y + G.cardH / 2);
    draw_playing_card(&view->card, x, y, G.cardW, G.cardH);
    bo_gfx_transform(0, 1, 0, 0);
}

static void draw_item(const BOItem *item, float x, float y, float w, float h) {
    const BOCenterInfo *c = bo_item_center(item);
    if (!c) return;
    draw_shadow(x, y, w, h);
    if (item->face_down) {
        draw_back(x, y, w, h);
        return;
    }
    BOColor white = bo_rgb(BO_C_WHITE);
    BOLayer layer = {NULL, 1, 1, c->pos_x, c->pos_y};
    int shine = -1;
    switch (c->set) {
    case BO_SET_JOKER: layer = (BOLayer){&G.jokers, 10, 16, c->pos_x, c->pos_y}; break;
    case BO_SET_TAROT:
    case BO_SET_PLANET:
    case BO_SET_SPECTRAL:
        layer = (BOLayer){&G.tarots, 10, 6, c->pos_x, c->pos_y};
        // The Spectral cards and The Soul shine as the vouchers do.
        if (c->set == BO_SET_SPECTRAL) shine = BO_FX_BOOSTER;
        break;
    case BO_SET_VOUCHER:
        layer = (BOLayer){&G.vouchers, 9, 4, c->pos_x, c->pos_y};
        shine = BO_FX_VOUCHER;
        break;
    case BO_SET_BOOSTER:
        layer = (BOLayer){&G.boosters, 4, 9, c->pos_x, c->pos_y};
        shine = BO_FX_BOOSTER;
        break;
    default: break;
    }
    if (layer.texture) draw_with_edition(&layer, 1, item->edition, shine, x, y, w, h);
    // The soul of a legendary joker and of The Soul floats over the card.
    if (c->soul_x >= 0 && layer.texture) {
        float bob = sinf((float)G.time * 2) * h * 0.02f;
        draw_cell(layer.texture, layer.columns, layer.rows, c->soul_x, c->soul_y, x, y - h * 0.02f + bob, w, h, white);
    }
    if (item->debuffed) bo_gfx_round_rect(x, y, w, h, w * 0.06f, bo_rgba(0x000000, 0.45f));
    // The stickers of the stakes: shared_sticker_eternal, _perishable, and
    // _rental in stickers.png.
    if (item->eternal) draw_cell(&G.stickers, 5, 3, 0, 0, x, y, w, h, white);
    if (item->perishable) draw_cell(&G.stickers, 5, 3, 0, 2, x, y, w, h, white);
    if (item->rental) draw_cell(&G.stickers, 5, 3, 1, 2, x, y, w, h, white);
}

// Draws a joker or a consumable with the turn and the size of its motion.
static void draw_item_view(const BOItem *item, const BOItemView *v) {
    bo_gfx_transform(v->m.r, v->m.scale, v->x + G.cardW / 2, v->y + G.cardH / 2);
    draw_item(item, v->x, v->y, G.cardW, G.cardH);
    bo_gfx_transform(0, 1, 0, 0);
}

static float tag_size(void) {
    return G.height * 0.075f;
}

static void draw_tag(BOCenterId tag, float x, float y, float size) {
    const BOCenterInfo *c = bo_center(tag);
    draw_cell(&G.tags, 6, 5, c->pos_x, c->pos_y, x, y, size, size, bo_rgb(BO_C_WHITE));
}

// The place of a tag that waits: in rows under the deck.
static void held_tag_place(int index, float *x, float *y) {
    float size = tag_size();
    int perRow = (int)((G.width - deck_x()) / (size + 4));
    if (perRow < 1) perRow = 1;
    *x = deck_x() - 4 + (index % perRow) * (size + 4);
    *y = G.height * 0.86f - (index / perRow) * (size + 4);
}

static void draw_held_tags(void) {
    for (int i = 0; i < G.run->tag_count; i++) {
        float x, y;
        held_tag_place(i, &x, &y);
        draw_tag(G.run->tags[i].center, x, y, tag_size());
        bo_ui_area(BO_ID_HELD_TAG + i, bo_rect(x, y, tag_size(), tag_size()));
    }
}

static void draw_deck(void) {
    float x = deck_x(), y = hand_top();
    int count = G.run->deck_count;
    // A few cards under the top show the height of the pile.
    int layers = count > 0 ? 1 + count / 13 : 0;
    for (int i = layers - 1; i >= 0; i--) draw_back(x - i * 2, y - i * 2, G.cardW, G.cardH);
    // The run keeps each card in the deck, the hand, or the discard pile.
    int total = count + G.run->hand_count + G.run->discard_count;
    char text[16];
    snprintf(text, sizeof(text), "%d/%d", count, total);
    bo_font_draw(text, x + G.cardW / 2, y + G.cardH + G.height * 0.01f, G.height * 0.04f, bo_rgb(BO_C_WHITE),
                 BO_ALIGN_CENTER);
}

// Writes a number as Balatro does: with commas, as "12,500".
static void format_number(double value, char *text, size_t size) {
    char digits[32];
    snprintf(digits, sizeof(digits), "%.0f", floor(value));
    int length = (int)strlen(digits), out = 0;
    for (int i = 0; i < length && out < (int)size - 2; i++) {
        if (i > 0 && digits[i - 1] != '-' && (length - i) % 3 == 0) text[out++] = ',';
        text[out++] = digits[i];
    }
    text[out] = 0;
}

static void draw_blind_chip(BOBlindId blind, float x, float y, float size) {
    float frame = G.chips.width / 21.0f;
    int index = (int)(G.time * 10) % 21;
    bo_gfx_sprite(&G.chips, frame * index, frame * bo_blind_info(blind)->sprite_row, frame, frame, x, y, size, size,
                  0, bo_rgb(BO_C_WHITE));
}

// --- The side panel -------------------------------------------------------

static void draw_label_value(const char *label, const char *value, BORect r, unsigned int valueColor) {
    bo_ui_box(r, BO_C_DARK);
    bo_font_draw(label, r.x + r.w / 2, r.y + r.h * 0.08f, r.h * 0.3f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    BORect inner = bo_rect(r.x + r.w * 0.1f, r.y + r.h * 0.45f, r.w * 0.8f, r.h * 0.48f);
    bo_ui_box(inner, BO_C_L_BLACK);
    bo_font_draw(value, r.x + r.w / 2, inner.y + inner.h * 0.06f, inner.h * 0.9f, bo_rgb(valueColor),
                 BO_ALIGN_CENTER);
}

static void draw_panel(void) {
    BORun *run = G.run;
    float panelW = panel_width(), pad = G.width * 0.012f, inner = panelW - pad * 2;
    bo_gfx_rect(0, 0, panelW, G.height, bo_rgb(BO_C_BLACK));
    char text[48], value[32];

    // The blind, or the title of the blind select or the shop.
    BORect blindBox = bo_rect(pad, pad, inner, G.height * 0.2f);
    bo_ui_box(blindBox, BO_C_DARK);
    if ((run->state == BO_STATE_SELECTING || run->state == BO_STATE_ROUND_EVAL || run->state == BO_STATE_GAME_OVER) ||
        G.flow != BO_FLOW_IDLE) {
        const BOBlindInfo *info = bo_blind_info(run->blind);
        BORect head = bo_rect(blindBox.x, blindBox.y, blindBox.w, blindBox.h * 0.3f);
        bo_ui_box(head, info->colour);
        bo_font_draw(info->name, head.x + head.w / 2, head.y + head.h * 0.15f, head.h * 0.7f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_CENTER);
        float chip = blindBox.h * 0.58f;
        draw_blind_chip(run->blind, blindBox.x + pad * 0.5f, head.y + head.h + pad * 0.5f, chip);
        float tx = blindBox.x + chip + pad;
        bo_font_draw("Score at least", tx, head.y + head.h + pad * 0.6f, blindBox.h * 0.12f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_LEFT);
        format_number(run->blind_chips, value, sizeof(value));
        bo_font_draw(value, tx, head.y + head.h + blindBox.h * 0.22f, blindBox.h * 0.2f, bo_rgb(BO_C_RED),
                     BO_ALIGN_LEFT);
        snprintf(text, sizeof(text), "Reward: %.*s", run->blind_dollars, "$$$$$$$$$$");
        bo_font_draw(text, tx, head.y + head.h + blindBox.h * 0.46f, blindBox.h * 0.12f, bo_rgb(BO_C_MONEY),
                     BO_ALIGN_LEFT);
    } else {
        int shop = run->state == BO_STATE_SHOP || run->state == BO_STATE_PACK;
        bo_font_draw(shop ? "Improve" : "Choose your", panelW / 2, blindBox.y + blindBox.h * 0.22f,
                     blindBox.h * 0.22f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
        bo_font_draw(shop ? "your run!" : "next Blind", panelW / 2, blindBox.y + blindBox.h * 0.5f, blindBox.h * 0.22f,
                     bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    }

    // The round score.
    float y = G.height * 0.24f, h = G.height * 0.09f;
    BORect scoreBox = bo_rect(pad, y, inner, h);
    bo_ui_box(scoreBox, BO_C_DARK);
    bo_font_draw("Round", pad * 2, y + h * 0.12f, h * 0.32f, bo_rgb(BO_C_WHITE), BO_ALIGN_LEFT);
    bo_font_draw("score", pad * 2, y + h * 0.5f, h * 0.32f, bo_rgb(BO_C_WHITE), BO_ALIGN_LEFT);
    BORect scoreValue = bo_rect(pad + inner * 0.38f, y + h * 0.12f, inner * 0.58f, h * 0.76f);
    bo_ui_box(scoreValue, BO_C_L_BLACK);
    format_number(G.shownRound, value, sizeof(value));
    bo_font_draw(value, scoreValue.x + scoreValue.w / 2, scoreValue.y + scoreValue.h * 0.1f, scoreValue.h * 0.8f,
                 bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);

    // The hand and its chips and mult.
    y = G.height * 0.345f;
    h = G.height * 0.17f;
    bo_ui_box(bo_rect(pad, y, inner, h), BO_C_DARK);
    if (G.shownHand) {
        bo_font_draw(G.shownHand, panelW / 2, y + h * 0.06f, h * 0.28f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    }
    float half = (inner - pad * 3) / 2;
    float rowY = y + h * 0.42f, rowH = h * 0.48f;
    bo_gfx_transform(G.chipsBox.r, G.chipsBox.scale, pad * 2 + half / 2, rowY + rowH / 2);
    bo_ui_box(bo_rect(pad * 2, rowY, half, rowH), BO_C_BLUE);
    if (G.shownHand) {
        format_number(G.shownChips, value, sizeof(value));
        bo_font_draw(value, pad * 2 + half - pad * 0.6f, rowY + rowH * 0.12f, rowH * 0.8f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_RIGHT);
    }
    bo_gfx_transform(G.multBox.r, G.multBox.scale, pad * 3 + half * 1.5f, rowY + rowH / 2);
    bo_ui_box(bo_rect(pad * 3 + half, rowY, half, rowH), BO_C_RED);
    if (G.shownHand) {
        // The mult can have a part after the point, as 15.5.
        double m = G.shownMult;
        if (m == floor(m)) {
            format_number(m, value, sizeof(value));
        } else {
            snprintf(value, sizeof(value), m >= 100 ? "%.0f" : m >= 10 ? "%.1f" : "%.2f", m);
        }
        bo_font_draw(value, pad * 3 + half + pad * 0.6f, rowY + rowH * 0.12f, rowH * 0.8f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_LEFT);
    }
    bo_gfx_transform(0, 1, 0, 0);
    bo_font_draw("X", panelW / 2, rowY + rowH * 0.2f, rowH * 0.6f, bo_rgb(BO_C_RED), BO_ALIGN_CENTER);

    // The hands, the discards, the money, the ante, and the round.
    float colW = (inner - pad) / 2;
    y = G.height * 0.53f;
    h = G.height * 0.11f;
    snprintf(value, sizeof(value), "%d", run->hands_left);
    draw_label_value("Hands", value, bo_rect(pad, y, colW, h), BO_C_BLUE);
    snprintf(value, sizeof(value), "%d", run->discards_left);
    draw_label_value("Discards", value, bo_rect(pad * 2 + colW, y, colW, h), BO_C_RED);

    y = G.height * 0.655f;
    BORect money = bo_rect(pad, y, inner, h * 0.85f);
    bo_ui_box(money, BO_C_DARK);
    snprintf(value, sizeof(value), "$%.0f", G.shownDollars);
    bo_font_draw(value, money.x + money.w / 2, money.y + money.h * 0.12f, money.h * 0.8f, bo_rgb(BO_C_MONEY),
                 BO_ALIGN_CENTER);

    y = G.height * 0.76f;
    snprintf(value, sizeof(value), "%d/%d", run->ante, run->win_ante);
    draw_label_value("Ante", value, bo_rect(pad, y, colW, h), BO_C_ORANGE);
    snprintf(value, sizeof(value), "%d", run->round);
    draw_label_value("Round", value, bo_rect(pad * 2 + colW, y, colW, h), BO_C_ORANGE);

    BORect info = bo_rect(pad, G.height * 0.875f, inner, G.height * 0.065f);
    if (bo_ui_button(BO_ID_RUN_INFO, info, BO_C_RED, "Run Info", 1)) G.showInfo = 1;
    snprintf(text, sizeof(text), "Seed %s", run->random.seed);
    bo_font_draw(text, panelW / 2, G.height * 0.95f, G.height * 0.032f, bo_rgb(BO_C_GREY), BO_ALIGN_CENTER);
}

// --- The jokers, the consumables, and the text of a card -----------------

static void draw_items(void) {
    BORun *run = G.run;
    BORect jokers = joker_area(), consumables = consumable_area();
    sync_items(G.jokerViews, &G.jokerViewCount, run->jokers, run->joker_count, jokers);
    sync_items(G.consumableViews, &G.consumableViewCount, run->consumables, run->consumable_count, consumables);
    char text[24];
    snprintf(text, sizeof(text), "%d/%d", run->joker_count, run->joker_slots);
    bo_font_draw(text, jokers.x, jokers.y + jokers.h + 2, G.height * 0.032f, bo_rgb(BO_C_WHITE), BO_ALIGN_LEFT);
    snprintf(text, sizeof(text), "%d/%d", run->consumable_count, run->consumable_slots);
    bo_font_draw(text, consumables.x + consumables.w, consumables.y + consumables.h + 2, G.height * 0.032f,
                 bo_rgb(BO_C_WHITE), BO_ALIGN_RIGHT);
    for (int i = 0; i < run->consumable_count; i++) {
        BOItemView *v = &G.consumableViews[i];
        draw_item_view(&run->consumables[i], v);
        bo_ui_area(BO_ID_CONSUMABLE + i, bo_rect(v->x, v->y, G.cardW, G.cardH));
    }
    for (int i = 0; i < run->joker_count; i++) {
        if (G.dragging && i == G.dragIndex) continue;
        BOItemView *v = &G.jokerViews[i];
        draw_item_view(&run->jokers[i], v);
        bo_ui_area(BO_ID_JOKER + i, bo_rect(v->x, v->y, G.cardW, G.cardH));
    }
    // The held joker is on top of the others.
    if (G.dragging && G.dragIndex >= 0 && G.dragIndex < G.jokerViewCount) {
        draw_item_view(&run->jokers[G.dragIndex], &G.jokerViews[G.dragIndex]);
    }
}

static const char *rarity_name(int rarity) {
    static const char *names[] = {"", "Common", "Uncommon", "Rare", "Legendary"};
    return rarity >= 1 && rarity <= 4 ? names[rarity] : "";
}

static unsigned int rarity_colour(int rarity) {
    static const unsigned int colours[] = {0x708b91, 0x009dff, 0x4bc292, 0xfe5f55, 0xb26cbb};
    return rarity >= 0 && rarity <= 4 ? colours[rarity] : 0x708b91;
}

static const char *set_name(int set) {
    switch (set) {
    case BO_SET_TAROT: return "Tarot";
    case BO_SET_PLANET: return "Planet";
    case BO_SET_SPECTRAL: return "Spectral";
    case BO_SET_VOUCHER: return "Voucher";
    case BO_SET_BOOSTER: return "Booster";
    default: return "";
    }
}

static unsigned int set_colour(int set) {
    switch (set) {
    case BO_SET_TAROT: return 0xa782d1;
    case BO_SET_PLANET: return 0x13afce;
    case BO_SET_SPECTRAL: return 0x4584fa;
    case BO_SET_VOUCHER: return 0xfd682b;
    case BO_SET_BOOSTER: return 0x646eb7;
    default: return 0x708b91;
    }
}

// A button of the text box of a card.
typedef struct {
    const char *label;
    unsigned int colour;
    int enabled;
} BOAction;

// The box with the name, the badge, and the text of a thing on the screen
// at (thingX, thingY) with the size thingW x thingH, under or over it, with
// up to two buttons. badge can be NULL. Returns the number of the button
// that is tapped, from 1, or 0.
static int draw_text_box(const BODescription *d, const char *badge, unsigned int badgeColour, float thingX,
                         float thingY, float thingW, float thingH, const BOAction *actions, int actionCount,
                         int beside) {
    float line = G.height * 0.042f, pad = 10;
    float w = G.width * 0.3f;
    for (int i = 0; i < d->line_count; i++) {
        float lw = bo_rich_width(d->lines[i], line) + pad * 4;
        if (lw > w) w = lw;
    }
    float nameH = G.height * 0.06f, badgeH = badge ? G.height * 0.045f : 0;
    float h = pad * 2 + nameH + d->line_count * line * 1.15f + badgeH + pad * 2;
    if (actionCount > 0) h += G.height * 0.08f;
    float x = thingX + thingW / 2 - w / 2;
    float y = thingY + thingH + 8;
    if (y + h > G.height - 4) y = thingY - h - 8;
    if (beside) {
        // Beside the thing, so that the hand under it stays in view.
        x = thingX + thingW + 8;
        if (x + w > G.width - 4) x = thingX - w - 8;
        y = thingY - h * 0.25f;
    }
    if (x < 4) x = 4;
    if (x + w > G.width - 4) x = G.width - 4 - w;
    if (y + h > G.height - 4) y = G.height - 4 - h;
    if (y < 4) y = 4;
    BORect box = bo_rect(x, y, w, h);
    bo_gfx_round_rect(box.x + 3, box.y + 4, box.w, box.h, 10, bo_rgba(0x000000, 0.35f));
    bo_ui_box(box, BO_C_L_BLACK);
    bo_font_draw(d->name, x + w / 2, y + pad, nameH * 0.85f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    BORect body = bo_rect(x + pad, y + pad + nameH, w - pad * 2, d->line_count * line * 1.15f + pad);
    bo_ui_box(body, BO_C_WHITE);
    for (int i = 0; i < d->line_count; i++) {
        bo_rich_draw(d->lines[i], x + w / 2, body.y + pad * 0.5f + i * line * 1.15f, line, bo_rgb(BO_C_TEXT_DARK),
                     BO_ALIGN_CENTER);
    }
    float by = body.y + body.h + pad * 0.6f;
    if (badge) {
        BORect badgeBox = bo_rect(x + w * 0.25f, by, w * 0.5f, badgeH);
        bo_ui_box(badgeBox, badgeColour);
        bo_font_draw(badge, x + w / 2, by + badgeH * 0.12f, badgeH * 0.8f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    }
    bo_ui_area(BO_ID_BACKGROUND + 1000, box);
    int tapped = 0;
    if (actionCount > 0) {
        float bh = G.height * 0.07f, gap = pad;
        float bw = actionCount == 1 ? w * 0.6f : (w - pad * 2 - gap) / 2;
        float bx = actionCount == 1 ? x + w * 0.2f : x + pad;
        for (int i = 0; i < actionCount; i++) {
            BORect button = bo_rect(bx + i * (bw + gap), by + badgeH + pad * 0.6f, bw, bh);
            if (bo_ui_button(BO_ID_ACTION + i, button, actions[i].colour, actions[i].label, actions[i].enabled)) {
                tapped = i + 1;
            }
        }
    }
    return tapped;
}

// The text box of a card.
static int draw_card_text(const BOItem *item, float cardX, float cardY, const BOAction *actions, int actionCount,
                          int beside) {
    BODescription d;
    bo_describe_item(G.run, item, &d);
    const BOCenterInfo *c = bo_item_center(item);
    // The badge: the rarity of a joker, or the set of another card.
    const char *badge = c->set == BO_SET_JOKER ? rarity_name(c->rarity) : set_name(c->set);
    unsigned int badgeColour = c->set == BO_SET_JOKER ? rarity_colour(c->rarity) : set_colour(c->set);
    return draw_text_box(&d, badge, badgeColour, cardX, cardY, G.cardW, G.cardH, actions, actionCount, beside);
}

// --- The round -------------------------------------------------------------

static uint64_t selected_mask(int *count) {
    uint64_t mask = 0;
    int n = 0;
    for (int i = 0; i < G.handCount; i++) {
        if (!G.hand[i].selected) continue;
        n++;
        for (int j = 0; j < G.run->hand_count; j++) {
            if (G.run->hand[j].uid == G.hand[i].card.uid) mask |= (uint64_t)1 << j;
        }
    }
    if (count) *count = n;
    return mask;
}

static void update_preview(void) {
    BOCard cards[BO_MAX_PLAY];
    int count = 0;
    for (int i = 0; i < G.handCount && count < BO_MAX_PLAY; i++) {
        if (G.hand[i].selected) cards[count++] = G.hand[i].card;
    }
    if (count == 0) {
        G.shownHand = NULL;
        return;
    }
    BOHandRules rules;
    memset(&rules, 0, sizeof(rules));
    rules.four_fingers = bo_run_find_joker(G.run, BO_J_FOUR_FINGERS) > 0;
    rules.shortcut = bo_run_find_joker(G.run, BO_J_SHORTCUT) > 0;
    rules.smeared = bo_run_find_joker(G.run, BO_J_SMEARED) > 0;
    BOHandResult result = bo_hand_evaluate(cards, count, &rules);
    G.shownHand = bo_hand_name(result.type, result.royal);
    G.shownChips = bo_hand_level_chips(result.type, G.run->hands[result.type].level);
    G.shownMult = bo_hand_level_mult(result.type, G.run->hands[result.type].level);
    // CardArea:parse_highlighted: a boss can refuse the hand.
    if (bo_boss_debuff_hand(G.run, count, &result, result.type, 1)) {
        G.shownHand = "Not Allowed!";
        G.shownChips = 0;
        G.shownMult = 0;
    }
    // get_poker_hand_info: a card face down hides the hand.
    for (int i = 0; i < count; i++) {
        if (cards[i].face_down) G.shownHand = "????";
    }
}

static void play_selected(void) {
    int count = 0;
    uint64_t mask = selected_mask(&count);
    // Keep the views of the played cards, in the order of the hand.
    BOCardView played[BO_MAX_PLAY];
    int n = 0;
    for (int i = 0; i < G.handCount; i++) {
        if (G.hand[i].selected && n < BO_MAX_PLAY) {
            played[n] = G.hand[i];
            played[n].selected = 0;
            n++;
        }
    }
    if (!bo_run_play(G.run, mask, &G.score)) return;
    memcpy(G.play, played, sizeof(BOCardView) * n);
    G.playCount = n;
    G.shownHand = bo_hand_name(G.score.type, G.score.royal);
    G.shownChips = G.score.base_chips;
    G.shownMult = G.score.base_mult;
    // Balatro lifts the scoring cards of the played hand.
    for (int i = 0; i < n; i++) G.play[i].selected = (G.score.scoring >> i) & 1;
    BOCardView kept[BO_MAX_HAND];
    int keptCount = 0;
    for (int i = 0; i < G.handCount; i++) {
        if (!G.hand[i].selected) kept[keptCount++] = G.hand[i];
    }
    memcpy(G.hand, kept, sizeof(BOCardView) * keptCount);
    G.handCount = keptCount;
    G.nextEvent = 0;
    G.flow = BO_FLOW_SCORE;
    G.wait = 0.7;
    G.focus = BO_FOCUS_NONE;
    layout();
    fprintf(stderr, "play: %s, total %.0f, round %.0f of %.0f\n", G.shownHand, G.score.total, G.run->chips,
            G.run->blind_chips);
}

static void discard_selected(void) {
    int count = 0;
    uint64_t mask = selected_mask(&count);
    BOCardView gone[BO_MAX_PLAY];
    int n = 0;
    for (int i = 0; i < G.handCount; i++) {
        if (G.hand[i].selected && n < BO_MAX_PLAY) gone[n++] = G.hand[i];
    }
    if (!bo_run_discard(G.run, mask)) return;
    memcpy(G.play, gone, sizeof(BOCardView) * n);
    G.playCount = n;
    for (int i = 0; i < n; i++) {
        G.play[i].tx = G.width + G.cardW;
        G.play[i].selected = 0;
    }
    G.flow = BO_FLOW_CLEAR;
    G.wait = 0.35;
    G.shownHand = NULL;
    sync_hand();
}

// The place on the screen of the card of a score event.
static void event_place(const BOScoreEvent *event, float *x, float *y, BOMotion **motion) {
    static BOMotion none;
    *motion = &none;
    switch (event->area) {
    case BO_AREA_PLAY:
        *x = G.play[event->index].tx + G.cardW / 2;
        *y = G.play[event->index].ty - G.height * 0.07f;
        *motion = &G.play[event->index].m;
        break;
    case BO_AREA_HAND:
        *x = G.hand[event->index].tx + G.cardW / 2;
        *y = G.hand[event->index].ty - G.height * 0.07f;
        *motion = &G.hand[event->index].m;
        break;
    case BO_AREA_JOKER:
        if (event->index < G.jokerViewCount) {
            *x = G.jokerViews[event->index].x + G.cardW / 2;
            *y = G.jokerViews[event->index].y + G.cardH + G.height * 0.01f;
            *motion = &G.jokerViews[event->index].m;
        }
        break;
    case BO_AREA_CONSUMABLE:
        if (event->index < G.consumableViewCount) {
            *x = G.consumableViews[event->index].x + G.cardW / 2;
            *y = G.consumableViews[event->index].y + G.cardH + G.height * 0.01f;
            *motion = &G.consumableViews[event->index].m;
        }
        break;
    }
}

static void show_event(const BOScoreEvent *event) {
    float x = G.width / 2, y = G.height / 2;
    BOMotion *motion;
    event_place(event, &x, &y, &motion);
    // card_eval_status_text: juice_up(0.6, 0.1).
    bo_motion_juice(motion, 0.6f, 0.1f, G.time);
    char text[24];
    unsigned int color = BO_C_WHITE;
    switch (event->kind) {
    case BO_EVENT_CHIPS:
        snprintf(text, sizeof(text), "+%.0f", event->value);
        color = BO_C_BLUE;
        break;
    case BO_EVENT_MULT:
        snprintf(text, sizeof(text), "+%.14g Mult", event->value);
        color = BO_C_RED;
        break;
    case BO_EVENT_XMULT:
        snprintf(text, sizeof(text), "X%.14g Mult", event->value);
        color = BO_C_RED;
        break;
    case BO_EVENT_DOLLARS:
        snprintf(text, sizeof(text), "$%.0f", event->value);
        color = BO_C_MONEY;
        break;
    case BO_EVENT_AGAIN:
        snprintf(text, sizeof(text), "Again!");
        color = BO_C_ORANGE;
        break;
    case BO_EVENT_LEVEL_UP:
        snprintf(text, sizeof(text), "Level up!");
        color = BO_C_ORANGE;
        break;
    case BO_EVENT_DEBUFFED:
        snprintf(text, sizeof(text), "Debuffed");
        color = BO_C_RED;
        break;
    case BO_EVENT_BALANCED:
        // The Plasma Deck: the text shows over the played cards.
        snprintf(text, sizeof(text), "Balanced");
        color = 0xcc73d9;
        x = main_area().x + main_area().w / 2;
        y = G.height * 0.22f;
        break;
    default:
        snprintf(text, sizeof(text), "!");
        color = BO_C_ORANGE;
        break;
    }
    popup(text, x, y, color);
    // juice_up() with no arguments: 0.4, and a turn of 0.24 to a side.
    float side = (G.nextEvent & 1) ? 0.24f : -0.24f;
    if (event->chips != G.shownChips) bo_motion_juice(&G.chipsBox, 0.4f, side, G.time);
    if (event->mult != G.shownMult) bo_motion_juice(&G.multBox, 0.4f, -side, G.time);
    G.jiggle += 0.7f;
    G.shownChips = event->chips;
    G.shownMult = event->mult;
}

static void update_flow(double dt) {
    G.wait -= dt;
    if (G.wait > 0) return;
    if (G.flow == BO_FLOW_SCORE) {
        if (G.nextEvent < G.score.event_count) {
            const BOScoreEvent *event = &G.score.events[G.nextEvent++];
            show_event(event);
            G.wait = event->kind == BO_EVENT_MESSAGE ? 0.3 : 0.38;
        } else {
            for (int i = 0; i < G.playCount; i++) {
                G.play[i].tx = G.width + G.cardW;
                G.play[i].selected = 0;
            }
            G.flow = BO_FLOW_CLEAR;
            G.wait = 0.6;
        }
    } else if (G.flow == BO_FLOW_CLEAR) {
        G.playCount = 0;
        G.flow = BO_FLOW_IDLE;
        G.shownHand = NULL;
        if (G.run->state == BO_STATE_SELECTING) {
            sync_hand();
        } else {
            G.handCount = 0;
        }
    }
}

static void draw_hand_controls(void) {
    BORun *run = G.run;
    int count = 0;
    selected_mask(&count);
    int idle = G.flow == BO_FLOW_IDLE && run->state == BO_STATE_SELECTING;
    BORect area = main_area();
    float bw = area.w * 0.22f, bh = G.height * 0.09f;
    float y = G.height - bh - G.height * 0.04f;
    float centre = area.x + area.w / 2;
    if (bo_ui_button(BO_ID_PLAY, bo_rect(area.x + area.w * 0.04f, y, bw, bh), BO_C_BLUE, "Play Hand",
                     idle && count > 0 && run->hands_left > 0)) {
        play_selected();
    }
    if (bo_ui_button(BO_ID_DISCARD, bo_rect(area.x + area.w * 0.96f - bw, y, bw, bh), BO_C_RED, "Discard",
                     idle && count > 0 && run->discards_left > 0)) {
        discard_selected();
    }
    float sw = area.w * 0.17f, sh = bh * 0.62f;
    BORect sortBox = bo_rect(centre - sw - G.width * 0.012f, y - bh * 0.08f, sw * 2 + G.width * 0.024f, bh * 1.16f);
    bo_ui_box(sortBox, BO_C_DARK);
    bo_font_draw("Sort Hand", centre, sortBox.y + sortBox.h * 0.04f, sortBox.h * 0.26f, bo_rgb(BO_C_WHITE),
                 BO_ALIGN_CENTER);
    float sy = sortBox.y + sortBox.h * 0.34f;
    if (bo_ui_button(BO_ID_SORT_RANK, bo_rect(centre - sw - G.width * 0.004f, sy, sw, sh), BO_C_ORANGE, "Rank",
                     idle)) {
        bo_run_sort_hand(run, 0);
        sync_hand();
    }
    if (bo_ui_button(BO_ID_SORT_SUIT, bo_rect(centre + G.width * 0.004f, sy, sw, sh), BO_C_ORANGE, "Suit", idle)) {
        bo_run_sort_hand(run, 1);
        sync_hand();
    }
}

// The cards of the hand. With interactive, a tap selects a card, as for a
// play or for a Tarot card.
static void draw_hand_cards(int interactive) {
    for (int i = 0; i < G.handCount; i++) {
        BOCardView *c = &G.hand[i];
        draw_view(c);
        if (interactive) bo_ui_area(BO_ID_HAND_CARD + i, bo_rect(c->tx, c->ty, G.cardW, G.cardH));
    }
}

static void handle_hand_taps(void) {
    for (int i = 0; i < G.handCount; i++) {
        if (!bo_ui_clicked(BO_ID_HAND_CARD + i)) continue;
        int count = 0;
        selected_mask(&count);
        // A Tarot card keeps its focus, so that the cards for it can be
        // selected.
        if (G.focus != BO_FOCUS_CONSUMABLE && G.focus != BO_FOCUS_PACK_CARD) G.focus = BO_FOCUS_NONE;
        if (!G.hand[i].selected && count >= BO_MAX_PLAY) break;
        if (G.hand[i].card.forced_selection) break;
        G.hand[i].selected = !G.hand[i].selected;
        update_preview();
        layout();
        break;
    }
}

static void clear_selection(void) {
    for (int i = 0; i < G.handCount; i++) G.hand[i].selected = G.hand[i].card.forced_selection;
    update_preview();
    layout();
}

// The text of the boss of the round, over the empty play area.
static void draw_boss_text(void) {
    BORun *run = G.run;
    if (!bo_blind_info(run->blind)->boss || run->blind_disabled || G.playCount > 0) return;
    BODescription d;
    if (!bo_describe_blind(run, run->blind, &d)) return;
    BORect area = main_area();
    float lh = G.height * 0.04f, y = G.height * 0.33f;
    bo_font_draw(d.name, area.x + area.w / 2, y, lh * 1.2f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    for (int l = 0; l < d.line_count; l++) {
        bo_rich_draw(d.lines[l], area.x + area.w / 2, y + lh * 1.5f + l * lh * 1.1f, lh, bo_rgba(BO_C_WHITE, 0.8f),
                     BO_ALIGN_CENTER);
    }
}

static void draw_round(void) {
    if (G.run->state == BO_STATE_SELECTING && G.flow == BO_FLOW_IDLE) draw_boss_text();
    draw_hand_cards(G.flow == BO_FLOW_IDLE && G.run->state == BO_STATE_SELECTING);
    for (int i = 0; i < G.playCount; i++) draw_view(&G.play[i]);
    draw_hand_controls();
    handle_hand_taps();
}

// --- The blind select, the cash out, and the end ----------------------------

static void draw_blind_select(void) {
    BORun *run = G.run;
    BORect area = main_area();
    float colW = area.w * 0.31f, gap = (area.w - colW * 3) / 2;
    char value[32], text[48];
    float baseTop = G.height * 0.30f;
    for (int slot = 0; slot < 3; slot++) {
        BOBlindId blind = bo_run_slot_blind(run, (BOBlindSlot)slot);
        const BOBlindInfo *info = bo_blind_info(blind);
        int current = slot == (int)run->slot;
        int done = slot < (int)run->slot;
        float x = area.x + slot * (colW + gap);
        float top = current ? baseTop - G.height * 0.035f : baseTop;
        BORect box = bo_rect(x, top, colW, G.height - top + 20);
        bo_ui_box(box, BO_C_BLACK);
        BORect inner = bo_rect(x + 8, top + 8, colW - 16, box.h - 16);
        bo_ui_box(inner, BO_C_DARK);
        float bh = G.height * 0.075f;
        BORect button = bo_rect(inner.x + 8, inner.y + 8, inner.w - 16, bh);
        if (current) {
            if (bo_ui_button(BO_ID_SELECT_BLIND, button, BO_C_ORANGE, "Select", 1)) {
                bo_run_select_blind(run);
                G.shownRound = 0;
                sync_hand();
                return;
            }
        } else {
            bo_ui_button(-1, button, BO_C_GREY, done ? ((run->skipped_slots >> slot) & 1 ? "Skipped" : "Defeated") : "Upcoming", 0);
        }
        float y = button.y + bh + 12;
        BORect name = bo_rect(inner.x + 8, y, inner.w - 16, bh * 0.8f);
        bo_ui_box(name, info->colour);
        bo_font_draw(info->name, name.x + name.w / 2, name.y + name.h * 0.12f, name.h * 0.75f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_CENTER);
        y += name.h + 10;
        float chip = colW * 0.42f;
        draw_blind_chip(blind, x + (colW - chip) / 2, y, chip);
        y += chip + 8;
        bo_font_draw("Score at least", x + colW / 2, y, G.height * 0.035f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
        format_number(bo_run_slot_chips(run, (BOBlindSlot)slot), value, sizeof(value));
        bo_font_draw(value, x + colW / 2, y + G.height * 0.045f, G.height * 0.06f, bo_rgb(BO_C_RED), BO_ALIGN_CENTER);
        int dollars = info->dollars;
        if (run->stake >= 2 && slot == BO_SLOT_SMALL) dollars = 0;
        snprintf(text, sizeof(text), "Reward: %.*s", dollars, "$$$$$$$$$$");
        bo_font_draw(dollars ? text : "No reward", x + colW / 2, y + G.height * 0.12f, G.height * 0.035f,
                     bo_rgb(BO_C_MONEY), BO_ALIGN_CENTER);
        if (slot != BO_SLOT_BOSS && !done) {
            // The tag of the blind, and the Skip Blind button.
            float size = tag_size();
            float ty = y + G.height * 0.215f;
            float tx = x + 12;
            bo_font_draw("or", x + colW / 2, ty - G.height * 0.045f, G.height * 0.032f, bo_rgb(BO_C_WHITE),
                         BO_ALIGN_CENTER);
            draw_tag(run->blind_tags[slot], tx, ty, size);
            G.blindTagX[slot] = tx;
            G.blindTagY[slot] = ty;
            bo_ui_area(BO_ID_BLIND_TAG + slot, bo_rect(tx, ty, size, size));
            BORect skip = bo_rect(tx + size + 8, ty + size * 0.1f, colW - size - 36, size * 0.8f);
            if (bo_ui_button(current ? BO_ID_SKIP_BLIND : -1, skip, BO_C_RED, "Skip Blind",
                             current && bo_run_can_skip_blind(run))) {
                bo_run_skip_blind(run);
                G.focus = BO_FOCUS_NONE;
                if (run->state == BO_STATE_PACK) sync_hand();
                return;
            }
        }
        if (info->boss) {
            BODescription d;
            bo_describe_blind(run, blind, &d);
            float ly = y + G.height * 0.18f, lh = G.height * 0.032f;
            for (int l = 0; l < d.line_count; l++) {
                bo_rich_draw(d.lines[l], x + colW / 2, ly + l * lh * 1.1f, lh, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
            }
        }
        if (!current) bo_gfx_rect(box.x, box.y, box.w, box.h, bo_rgba(0x000000, 0.35f));
        // Director's Cut and Retcon: a new boss for $10.
        if (slot == BO_SLOT_BOSS && (run->used_vouchers[BO_V_DIRECTORS_CUT] || run->used_vouchers[BO_V_RETCON])) {
            BORect reroll = bo_rect(x + 8, G.height - G.height * 0.1f, colW - 16, G.height * 0.075f);
            if (bo_ui_button(BO_ID_REROLL_BOSS, reroll, BO_C_RED, "Reroll Boss $10", bo_voucher_can_reroll_boss(run))) {
                bo_voucher_reroll_boss(run);
            }
        }
    }
}

static void draw_cash_out(void) {
    BORun *run = G.run;
    BORect area = main_area();
    BORect box = bo_rect(area.x + area.w * 0.08f, G.height * 0.27f, area.w * 0.84f, G.height * 0.66f);
    bo_ui_box(box, BO_C_BLACK);
    BORect inner = bo_rect(box.x + 8, box.y + 8, box.w - 16, box.h - 16);
    bo_ui_box(inner, BO_C_DARK);
    char text[64], value[16];
    snprintf(text, sizeof(text), "Cash Out: $%d", run->eval.total);
    if (bo_ui_button(BO_ID_CASH_OUT, bo_rect(inner.x + 12, inner.y + 12, inner.w - 24, G.height * 0.1f), BO_C_ORANGE,
                     text, 1)) {
        bo_run_cash_out(run);
        G.shownRound = 0;
        G.focus = BO_FOCUS_NONE;
        return;
    }
    float y = inner.y + G.height * 0.15f, rowH = G.height * 0.065f, size = G.height * 0.042f;
    for (int r = 0; r < run->eval.row_count; r++) {
        const BOEvalRow *row = &run->eval.rows[r];
        if (row->kind == BO_EVAL_BLIND) {
            const BOBlindInfo *info = bo_blind_info(run->blind);
            draw_blind_chip(run->blind, inner.x + 16, y, rowH);
            bo_font_draw(info->name, inner.x + 24 + rowH, y + rowH * 0.2f, size, bo_rgb(BO_C_WHITE), BO_ALIGN_LEFT);
            snprintf(value, sizeof(value), "%.*s", row->dollars, "$$$$$$$$$$");
            bo_font_draw(value, inner.x + inner.w - 16, y + rowH * 0.2f, size, bo_rgb(BO_C_MONEY), BO_ALIGN_RIGHT);
            y += rowH + 8;
            bo_gfx_rect(inner.x + 16, y, inner.w - 32, 2, bo_rgb(BO_C_L_BLACK));
            y += 12;
            continue;
        }
        const char *label = "";
        unsigned int color = BO_C_WHITE;
        char count[16] = "", each[40];
        if (row->kind == BO_EVAL_HANDS) {
            snprintf(count, sizeof(count), "%d", row->count);
            snprintf(each, sizeof(each), "Remaining Hands ($%d each)", run->money_per_hand);
            label = each;
            color = BO_C_BLUE;
        } else if (row->kind == BO_EVAL_DISCARDS) {
            snprintf(count, sizeof(count), "%d", row->count);
            snprintf(each, sizeof(each), "Remaining Discards ($%d each)", run->money_per_discard);
            label = each;
            color = BO_C_RED;
        } else if (row->kind == BO_EVAL_INTEREST) {
            snprintf(count, sizeof(count), "%d", row->count);
            label = "Interest ($1 per $5)";
            color = BO_C_MONEY;
        } else if (row->kind == BO_EVAL_TAG) {
            label = "Investment Tag";
        } else if (row->kind == BO_EVAL_JOKER) {
            label = row->index < run->joker_count ? bo_center(run->jokers[row->index].center)->name : "Joker";
        }
        float x = inner.x + 16;
        if (count[0]) {
            bo_font_draw(count, x, y, size, bo_rgb(color), BO_ALIGN_LEFT);
            x += bo_font_width(count, size) + 10;
        }
        bo_font_draw(label, x, y, size, bo_rgb(BO_C_WHITE), BO_ALIGN_LEFT);
        snprintf(value, sizeof(value), "$%d", row->dollars);
        bo_font_draw(value, inner.x + inner.w - 16, y, size, bo_rgb(BO_C_MONEY), BO_ALIGN_RIGHT);
        y += rowH;
    }
}

static void draw_end(int won) {
    BORun *run = G.run;
    BORect area = main_area();
    BORect box = bo_rect(area.x + area.w * 0.12f, G.height * 0.26f, area.w * 0.76f, G.height * 0.62f);
    bo_ui_box(box, BO_C_BLACK);
    BORect inner = bo_rect(box.x + 8, box.y + 8, box.w - 16, box.h - 16);
    bo_ui_box(inner, BO_C_DARK);
    bo_font_draw(won ? "YOU WIN!" : "GAME OVER", inner.x + inner.w / 2, inner.y + 20, G.height * 0.1f,
                 bo_rgb(won ? BO_C_ORANGE : BO_C_RED), BO_ALIGN_CENTER);
    char text[64];
    float y = inner.y + G.height * 0.17f, size = G.height * 0.05f;
    snprintf(text, sizeof(text), "Ante %d, round %d", run->ante, run->round);
    bo_font_draw(text, inner.x + inner.w / 2, y, size, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    if (!won) {
        snprintf(text, sizeof(text), "Defeated by %s", bo_blind_info(run->blind)->name);
        bo_font_draw(text, inner.x + inner.w / 2, y + size * 1.4f, size, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    }
    snprintf(text, sizeof(text), "Seed %s", run->random.seed);
    bo_font_draw(text, inner.x + inner.w / 2, y + size * 2.8f, size, bo_rgb(BO_C_GREY), BO_ALIGN_CENTER);
    float bh = G.height * 0.1f, bw = inner.w * 0.42f;
    float by = inner.y + inner.h - bh - 16;
    if (won) {
        if (bo_ui_button(BO_ID_ENDLESS, bo_rect(inner.x + inner.w / 2 + 8, by, bw, bh), BO_C_ORANGE, "Endless Mode",
                         1)) {
            G.endless = 1;
            return;
        }
        if (bo_ui_button(BO_ID_NEW_RUN, bo_rect(inner.x + inner.w / 2 - 8 - bw, by, bw, bh), BO_C_BLUE, "New Run",
                         1)) {
            G.setup = 1;
        }
    } else if (bo_ui_button(BO_ID_NEW_RUN, bo_rect(inner.x + (inner.w - bw) / 2, by, bw, bh), BO_C_BLUE, "New Run",
                            1)) {
        G.setup = 1;
    }
}

// --- The run setup ---------------------------------------------------------

// The setup of a new run, as the New Run screen of Balatro: the deck, its
// text, and the Play button.
static void draw_setup(void) {
    BORect box = bo_rect(G.width * 0.08f, G.height * 0.05f, G.width * 0.84f, G.height * 0.9f);
    bo_gfx_round_rect(box.x + 3, box.y + 5, box.w, box.h, 12, bo_rgba(0x000000, 0.35f));
    bo_ui_box(box, BO_C_BLACK);
    BORect inner = bo_rect(box.x + 8, box.y + 8, box.w - 16, box.h - 16);
    bo_ui_box(inner, BO_C_DARK);
    bo_font_draw("New Run", inner.x + inner.w / 2, inner.y + G.height * 0.02f, G.height * 0.08f,
                 bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    float colW = inner.w * 0.2f;
    if (bo_ui_button(BO_ID_COLLECTION, bo_rect(inner.x + inner.w - colW - 14, inner.y + 14, colW, G.height * 0.075f),
                     BO_C_ORANGE, "Collection", 1)) {
        G.collection = 1;
        G.colFocus = -1;
    }
    // The deck, between the two arrows.
    float cw = G.cardW * 1.3f, ch = G.cardH * 1.3f;
    float cx = inner.x + inner.w * 0.24f - cw / 2, cy = inner.y + G.height * 0.17f;
    const BOCenterInfo *back = bo_center(G.setupBack);
    int locked = !bo_profile_unlocked(G.setupBack);
    draw_shadow(cx, cy, cw, ch);
    // A locked deck shows the locked back at (4, 0).
    draw_cell(&G.centers, 7, 5, locked ? 4 : back->pos_x, locked ? 0 : back->pos_y, cx, cy, cw, ch,
              bo_rgb(BO_C_WHITE));
    float aw = G.width * 0.06f, ah = ch * 0.4f;
    if (bo_ui_button(BO_ID_SETUP_PREV, bo_rect(cx - aw - 14, cy + (ch - ah) / 2, aw, ah), BO_C_RED, "<", 1)) {
        G.setupBack = G.setupBack == BO_B_RED ? BO_B_ERRATIC : G.setupBack - 1;
    }
    if (bo_ui_button(BO_ID_SETUP_NEXT, bo_rect(cx + cw + 14, cy + (ch - ah) / 2, aw, ah), BO_C_RED, ">", 1)) {
        G.setupBack = G.setupBack == BO_B_ERRATIC ? BO_B_RED : G.setupBack + 1;
    }
    char text[16];
    snprintf(text, sizeof(text), "%d/15", G.setupBack - BO_B_RED + 1);
    bo_font_draw(text, cx + cw / 2, cy + ch + 10, G.height * 0.04f, bo_rgb(BO_C_GREY), BO_ALIGN_CENTER);
    // The name and the text of the deck.
    BOItem item;
    memset(&item, 0, sizeof(item));
    item.center = G.setupBack;
    BODescription d;
    if (locked) {
        bo_describe_locked_back(G.setupBack, &d);
    } else {
        bo_describe_item(NULL, &item, &d);
    }
    float tx = inner.x + inner.w * 0.47f, tw = inner.w * 0.49f;
    float line = G.height * 0.045f, pad = 10;
    BORect nameBox = bo_rect(tx, cy, tw, G.height * 0.08f);
    bo_ui_box(nameBox, BO_C_L_BLACK);
    bo_font_draw(d.name, tx + tw / 2, cy + G.height * 0.01f, G.height * 0.065f, bo_rgb(BO_C_WHITE),
                 BO_ALIGN_CENTER);
    BORect body = bo_rect(tx, cy + nameBox.h + pad, tw, d.line_count * line * 1.2f + pad * 2);
    bo_ui_box(body, BO_C_WHITE);
    for (int i = 0; i < d.line_count; i++) {
        bo_rich_draw(d.lines[i], tx + tw / 2, body.y + pad + i * line * 1.2f, line, bo_rgb(BO_C_TEXT_DARK),
                     BO_ALIGN_CENTER);
    }
    // The stake: its chip between two arrows, its name, and its text.
    static const char *stakeKeys[8] = {"stake_white", "stake_red", "stake_green", "stake_black",
                                        "stake_blue", "stake_purple", "stake_orange", "stake_gold"};
    static const int chipPos[8][2] = {{0, 0}, {1, 0}, {2, 0}, {4, 0}, {3, 0}, {0, 1}, {1, 1}, {2, 1}};
    // The stakes over the best win of the deck plus one are locked.
    int maxStake = bo_profile_max_stake(G.profile, G.setupBack);
    if (G.setupStake > maxStake) G.setupStake = maxStake;
    int stake = G.setupStake;
    float sy = cy + ch + G.height * 0.075f, chip = G.height * 0.1f;
    float scx = cx + cw / 2 - chip / 2;
    draw_cell(&G.stakes, 5, 2, chipPos[stake - 1][0], chipPos[stake - 1][1], scx, sy, chip, chip,
              bo_rgb(BO_C_WHITE));
    float sah = chip * 0.8f;
    if (bo_ui_button(BO_ID_STAKE_PREV, bo_rect(cx - aw - 14, sy + (chip - sah) / 2, aw, sah), BO_C_RED, "<", 1)) {
        G.setupStake = stake == 1 ? maxStake : stake - 1;
    }
    if (bo_ui_button(BO_ID_STAKE_NEXT, bo_rect(cx + cw + 14, sy + (chip - sah) / 2, aw, sah), BO_C_RED, ">", 1)) {
        G.setupStake = stake >= maxStake ? 1 : stake + 1;
    }
    char path[64];
    snprintf(path, sizeof(path), "descriptions.Stake.%s", stakeKeys[stake - 1]);
    BOLuaValue *entry = G.text ? bo_luatab_path(G.text, path) : NULL;
    const char *stakeName = entry ? bo_luatab_string(entry, "name") : NULL;
    BORect stakeBox = bo_rect(tx, sy - G.height * 0.02f, tw, G.height * 0.06f);
    bo_ui_box(stakeBox, BO_C_L_BLACK);
    bo_font_draw(stakeName ? stakeName : "Stake", tx + tw / 2, stakeBox.y + G.height * 0.005f, G.height * 0.05f,
                 bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    BOLuaValue *lines = entry ? bo_luatab_get(entry, "text") : NULL;
    int lineCount = lines ? bo_luatab_length(lines) : 0;
    float sline = G.height * 0.036f;
    BORect stakeBody = bo_rect(tx, stakeBox.y + stakeBox.h + 6, tw, lineCount * sline * 1.2f + pad);
    if (lineCount) bo_ui_box(stakeBody, BO_C_WHITE);
    for (int i = 0; i < lineCount; i++) {
        BOLuaValue *l = bo_luatab_item(lines, i);
        if (!l || l->type != BO_LUA_STRING) continue;
        bo_rich_draw(l->string, tx + tw / 2, stakeBody.y + pad * 0.5f + i * sline * 1.2f, sline,
                     bo_rgb(BO_C_TEXT_DARK), BO_ALIGN_CENTER);
    }

    float bw = inner.w * 0.34f, bh = G.height * 0.11f;
    // A run that is not over can go on.
    if (G.run->state != BO_STATE_GAME_OVER &&
        bo_ui_button(BO_ID_SETUP_BACK, bo_rect(inner.x + 14, inner.y + inner.h - bh - 14, inner.w * 0.18f, bh),
                     BO_C_ORANGE, "Back", 1)) {
        G.setup = 0;
    }
    // Unlock All of the profile menu: the second tap unlocks.
    if (G.profile && !G.profile->all_unlocked) {
        float uw = inner.w * 0.2f;
        const char *label = G.unlockConfirm > 0 ? "Sure?" : "Unlock All";
        if (bo_ui_button(BO_ID_UNLOCK_ALL, bo_rect(inner.x + inner.w - uw - 14, inner.y + inner.h - bh - 14, uw, bh),
                         G.unlockConfirm > 0 ? BO_C_RED : BO_C_GREY, label, 1)) {
            if (G.unlockConfirm > 0) {
                bo_profile_unlock_all(G.profile);
                G.unlockConfirm = 0;
            } else {
                G.unlockConfirm = 3;
            }
        }
    }
    if (bo_ui_button(BO_ID_SETUP_PLAY, bo_rect(inner.x + (inner.w - bw) / 2, inner.y + inner.h - bh - 14, bw, bh),
                     BO_C_BLUE, "Play", !locked)) {
        G.setup = 0;
        new_run();
    }
}

// --- The collection ---------------------------------------------------------

typedef struct {
    const char *label;
    int set;          // A BOSet, or -1 for the blinds.
    int columns, rows;
} BOCollectionTab;

static const BOCollectionTab kCollectionTabs[] = {
    {"Jokers", BO_SET_JOKER, 5, 3},     {"Decks", BO_SET_BACK, 5, 3},       {"Vouchers", BO_SET_VOUCHER, 5, 3},
    {"Tarots", BO_SET_TAROT, 5, 3},     {"Planets", BO_SET_PLANET, 5, 3},   {"Spectrals", BO_SET_SPECTRAL, 5, 3},
    {"Packs", BO_SET_BOOSTER, 5, 3},    {"Tags", BO_SET_TAG, 6, 4},         {"Blinds", -1, 6, 5},
};
#define BO_COLLECTION_TABS ((int)(sizeof(kCollectionTabs) / sizeof(kCollectionTabs[0])))

// The cards of a tab, in the order of the collection of Balatro.
static int collection_items(int tab, int *out, int max) {
    int count = 0;
    if (kCollectionTabs[tab].set < 0) {
        for (int b = 0; b < BO_BLIND_COUNT && count < max; b++) out[count++] = b;
        for (int i = 1; i < count; i++) {
            for (int j = i; j > 0 && bo_blind_info((BOBlindId)out[j - 1])->order > bo_blind_info((BOBlindId)out[j])->order;
                 j--) {
                int t = out[j];
                out[j] = out[j - 1];
                out[j - 1] = t;
            }
        }
        return count;
    }
    for (int i = 0; i < BO_CENTER_COUNT && count < max; i++) {
        const BOCenterInfo *c = bo_center((BOCenterId)i);
        if (c->set != kCollectionTabs[tab].set || (c->omit && c->set != BO_SET_TAG)) continue;
        out[count++] = i;
    }
    for (int i = 1; i < count; i++) {
        for (int j = i; j > 0 && bo_center((BOCenterId)out[j - 1])->order > bo_center((BOCenterId)out[j])->order; j--) {
            int t = out[j];
            out[j] = out[j - 1];
            out[j - 1] = t;
        }
    }
    return count;
}

static int collection_blind_discovered(int blind) {
    return !G.profile || G.profile->blind_discovered[blind];
}

// One card of the collection: its art when it is discovered, else the art
// of a locked or an undiscovered card, as Card:set_sprites.
static void draw_collection_card(int tab, int id, float x, float y, float w, float h) {
    BOColor white = bo_rgb(BO_C_WHITE);
    int set = kCollectionTabs[tab].set;
    if (set < 0) {
        if (collection_blind_discovered(id)) {
            draw_blind_chip((BOBlindId)id, x, y, w);
        } else {
            float frame = G.chips.width / 21.0f;
            bo_gfx_sprite(&G.chips, 0, frame * 30, frame, frame, x, y, w, w, 0, white);
        }
        return;
    }
    if (set == BO_SET_TAG) {
        if (bo_profile_discovered((BOCenterId)id)) {
            draw_tag((BOCenterId)id, x, y, w);
        } else {
            draw_cell(&G.tags, 6, 5, 3, 4, x, y, w, w, white);
        }
        return;
    }
    int locked = !bo_profile_unlocked((BOCenterId)id);
    int found = bo_profile_discovered((BOCenterId)id);
    draw_shadow(x, y, w, h);
    if (set == BO_SET_BACK) {
        const BOCenterInfo *c = bo_center((BOCenterId)id);
        draw_cell(&G.centers, 7, 5, locked ? 4 : c->pos_x, locked ? 0 : c->pos_y, x, y, w, h, white);
    } else if (locked && set == BO_SET_JOKER) {
        draw_cell(&G.jokers, 10, 16, 8, 9, x, y, w, h, white);
    } else if (locked && set == BO_SET_VOUCHER) {
        draw_cell(&G.vouchers, 9, 4, 8, 3, x, y, w, h, white);
    } else if (!found && set == BO_SET_JOKER) {
        draw_cell(&G.jokers, 10, 16, 9, 9, x, y, w, h, white);
    } else if (!found && set == BO_SET_VOUCHER) {
        draw_cell(&G.vouchers, 9, 4, 8, 2, x, y, w, h, white);
    } else if (!found && set == BO_SET_BOOSTER) {
        draw_cell(&G.boosters, 4, 9, 0, 5, x, y, w, h, white);
    } else if (!found && (set == BO_SET_TAROT || set == BO_SET_PLANET || set == BO_SET_SPECTRAL)) {
        int col = set == BO_SET_TAROT ? 6 : set == BO_SET_PLANET ? 7 : 5;
        draw_cell(&G.tarots, 10, 6, col, 2, x, y, w, h, white);
    } else {
        BOItem item;
        memset(&item, 0, sizeof(item));
        item.center = (BOCenterId)id;
        bo_item_set_ability(&item, item.center, 0);
        draw_item(&item, x, y, w, h);
    }
}

// The text of the focused card of the collection.
static void collection_text(int tab, int id, BODescription *d) {
    int set = kCollectionTabs[tab].set;
    if (set < 0) {
        if (collection_blind_discovered(id)) {
            bo_describe_blind(NULL, id, d);
        } else {
            memset(d, 0, sizeof(*d));
            snprintf(d->name, sizeof(d->name), "%s", "Undiscovered");
            // b_undiscovered.debuff_text, in two lines.
            snprintf(d->lines[0], sizeof(d->lines[0]), "%s", "Defeat this blind");
            snprintf(d->lines[1], sizeof(d->lines[1]), "%s", "to discover");
            d->line_count = 2;
        }
        return;
    }
    if (!bo_profile_unlocked((BOCenterId)id)) {
        if (set == BO_SET_BACK) bo_describe_locked_back(id, d);
        else bo_describe_locked(id, d);
    } else if (!bo_profile_discovered((BOCenterId)id)) {
        bo_describe_undiscovered(id, d);
    } else if (set == BO_SET_TAG) {
        bo_describe_tag(NULL, id, BO_HAND_HIGH_CARD, d);
    } else {
        BOItem item;
        memset(&item, 0, sizeof(item));
        item.center = (BOCenterId)id;
        bo_item_set_ability(&item, item.center, 0);
        bo_describe_item(NULL, &item, d);
    }
}

// G.FUNCS.your_collection: the tabs, a page of cards, and the text of the
// card that the player taps.
static void draw_collection(void) {
    BORect box = bo_rect(6, 6, G.width - 12, G.height - 12);
    bo_ui_box(box, BO_C_BLACK);
    BORect inner = bo_rect(box.x + 6, box.y + 6, box.w - 12, box.h - 12);
    bo_ui_box(inner, BO_C_DARK);
    // The tabs.
    float tabH = G.height * 0.075f, gap = 4;
    float tabW = (inner.w - 16 - gap * (BO_COLLECTION_TABS - 1)) / BO_COLLECTION_TABS;
    for (int t = 0; t < BO_COLLECTION_TABS; t++) {
        BORect r = bo_rect(inner.x + 8 + t * (tabW + gap), inner.y + 8, tabW, tabH);
        if (bo_ui_button(BO_ID_COL_TAB + t, r, t == G.colTab ? BO_C_RED : BO_C_GREY, kCollectionTabs[t].label, 1)) {
            G.colTab = t;
            G.colPage = 0;
            G.colFocus = -1;
        }
    }
    int items[BO_CENTER_COUNT];
    int count = collection_items(G.colTab, items, BO_CENTER_COUNT);
    const BOCollectionTab *tab = &kCollectionTabs[G.colTab];
    int perPage = tab->columns * tab->rows;
    int pages = (count + perPage - 1) / perPage;
    if (G.colPage >= pages) G.colPage = pages - 1;
    if (G.colPage < 0) G.colPage = 0;
    // The grid.
    float top = inner.y + tabH + 20, bottom = inner.y + inner.h - G.height * 0.11f;
    float cellH = (bottom - top) / tab->rows, cellW = (inner.w * 0.62f) / tab->columns;
    int square = tab->set == BO_SET_TAG || tab->set < 0;
    float h = cellH * 0.9f, w = square ? h : h * G.cardW / G.cardH;
    if (w > cellW * 0.92f) {
        w = cellW * 0.92f;
        h = square ? w : w * G.cardH / G.cardW;
    }
    float gridX = inner.x + 12;
    for (int i = 0; i < perPage; i++) {
        int index = G.colPage * perPage + i;
        if (index >= count) break;
        int col = i % tab->columns, row = i / tab->columns;
        float x = gridX + col * cellW + (cellW - w) / 2, y = top + row * cellH + (cellH - h) / 2;
        float lift = G.colFocus == index ? h * 0.06f : 0;
        draw_collection_card(G.colTab, items[index], x, y - lift, w, h);
        bo_ui_area(BO_ID_COL_CARD + i, bo_rect(x, y, w, h));
        if (bo_ui_clicked(BO_ID_COL_CARD + i)) G.colFocus = index;
    }
    // The page arrows.
    float aw = G.width * 0.07f, ah = G.height * 0.075f, ay = inner.y + inner.h - ah - 10;
    float midX = gridX + cellW * tab->columns / 2;
    if (pages > 1 && bo_ui_button(BO_ID_COL_PREV, bo_rect(midX - aw * 2, ay, aw, ah), BO_C_RED, "<", 1)) {
        G.colPage = G.colPage == 0 ? pages - 1 : G.colPage - 1;
        G.colFocus = -1;
    }
    if (pages > 1 && bo_ui_button(BO_ID_COL_NEXT, bo_rect(midX + aw, ay, aw, ah), BO_C_RED, ">", 1)) {
        G.colPage = G.colPage == pages - 1 ? 0 : G.colPage + 1;
        G.colFocus = -1;
    }
    char text[32];
    snprintf(text, sizeof(text), "%d/%d", G.colPage + 1, pages > 0 ? pages : 1);
    bo_font_draw(text, midX, ay + ah * 0.15f, ah * 0.7f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    if (bo_ui_button(BO_ID_COL_CLOSE, bo_rect(inner.x + inner.w - inner.w * 0.18f - 10, ay, inner.w * 0.18f, ah),
                     BO_C_ORANGE, "Back", 1)) {
        G.collection = 0;
    }
    // The text of the focused card, on the right.
    if (G.colFocus >= 0 && G.colFocus < count) {
        BODescription d;
        collection_text(G.colTab, items[G.colFocus], &d);
        float tx = inner.x + inner.w * 0.66f, tw = inner.w * 0.32f, line = G.height * 0.04f, pad = 8;
        BORect nameBox = bo_rect(tx, top, tw, G.height * 0.07f);
        bo_ui_box(nameBox, BO_C_L_BLACK);
        bo_font_draw(d.name, tx + tw / 2, top + G.height * 0.008f, G.height * 0.055f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_CENTER);
        if (d.line_count > 0) {
            BORect body = bo_rect(tx, top + nameBox.h + 6, tw, d.line_count * line * 1.2f + pad * 2);
            bo_ui_box(body, BO_C_WHITE);
            for (int i = 0; i < d.line_count; i++) {
                bo_rich_draw(d.lines[i], tx + tw / 2, body.y + pad + i * line * 1.2f, line, bo_rgb(BO_C_TEXT_DARK),
                             BO_ALIGN_CENTER);
            }
        }
    }
    // The count of discovered cards, as the collection of Balatro shows.
    if (G.profile) {
        int total, of;
        bo_profile_discover_tallies(G.profile, &total, &of, NULL, NULL);
        snprintf(text, sizeof(text), "%d/%d", total, of);
        bo_font_draw(text, inner.x + 14, ay + ah * 0.2f, ah * 0.55f, bo_rgb(BO_C_GREY), BO_ALIGN_LEFT);
    }
}

// --- The shop --------------------------------------------------------------

static void voucher_place(float *x, float *y);
static void pack_place(int index, float *x, float *y);

static BORect shop_box(void) {
    BORect area = main_area();
    return bo_rect(area.x, G.height * 0.255f, area.w + G.width * 0.02f, G.height * 0.73f);
}

static void shop_card_place(int index, int count, float *x, float *y) {
    BORect box = shop_box();
    BORect cards = bo_rect(box.x + box.w * 0.34f, box.y, box.w * 0.62f, G.cardH);
    *x = slot_x(cards, index, count);
    *y = box.y + G.height * 0.07f;
}

static void draw_price(int cost, float x, float y) {
    char text[16];
    snprintf(text, sizeof(text), "$%d", cost);
    float w = G.cardW * 0.5f, h = G.height * 0.05f;
    bo_ui_box(bo_rect(x + (G.cardW - w) / 2, y - h - 2, w, h), BO_C_DARK);
    bo_font_draw(text, x + G.cardW / 2, y - h + h * 0.08f, h * 0.8f, bo_rgb(BO_C_MONEY), BO_ALIGN_CENTER);
}

static void draw_shop(void) {
    BORun *run = G.run;
    BORect box = shop_box();
    bo_ui_box(box, BO_C_BLACK);
    BORect inner = bo_rect(box.x + 8, box.y + 8, box.w - 16, box.h - 16);
    bo_ui_box(inner, BO_C_DARK);

    float bw = box.w * 0.26f, bh = G.height * 0.12f;
    float bx = inner.x + 12, by = inner.y + 16;
    if (bo_ui_button(BO_ID_NEXT_ROUND, bo_rect(bx, by, bw, bh), BO_C_RED, "Next Round", 1)) {
        bo_run_end_shop(run);
        G.focus = BO_FOCUS_NONE;
        return;
    }
    char text[24];
    snprintf(text, sizeof(text), "Reroll $%d", run->reroll_cost);
    int canReroll = run->reroll_cost <= bo_run_spendable(run) || run->reroll_cost <= 0;
    if (bo_ui_button(BO_ID_REROLL, bo_rect(bx, by + bh + 12, bw, bh), BO_C_GREEN, text, canReroll)) {
        bo_run_reroll(run);
        G.focus = BO_FOCUS_NONE;
    }

    // The cards for sale.
    for (int i = 0; i < run->shop_count; i++) {
        float x, y;
        shop_card_place(i, run->shop_count, &x, &y);
        int focused = G.focus == BO_FOCUS_SHOP && G.focusIndex == i;
        if (focused) y -= G.cardH * 0.08f;
        BOShopCard *card = &run->shop[i];
        if (card->is_playing_card) {
            draw_playing_card(&card->card, x, y, G.cardW, G.cardH);
        } else {
            draw_item(&card->item, x, y, G.cardW, G.cardH);
        }
        draw_price(card->item.cost, x, y);
        bo_ui_area(BO_ID_SHOP + i, bo_rect(x, y, G.cardW, G.cardH));
    }

    // The voucher and the packs. They open in 0.4 and 0.5.
    if (run->has_shop_voucher) {
        float x, y2;
        voucher_place(&x, &y2);
        draw_item(&run->shop_voucher, x, y2, G.cardW, G.cardH);
        draw_price(run->shop_voucher.cost, x, y2);
        bo_ui_area(BO_ID_VOUCHER, bo_rect(x, y2, G.cardW, G.cardH));
    }
    for (int i = 0; i < run->tag_voucher_count; i++) {
        float x, y2;
        voucher_place(&x, &y2);
        x += (run->has_shop_voucher ? i + 1 : i) * (G.cardW + 8);
        draw_item(&run->tag_vouchers[i], x, y2, G.cardW, G.cardH);
        draw_price(run->tag_vouchers[i].cost, x, y2);
        bo_ui_area(BO_ID_TAG_VOUCHER + i, bo_rect(x, y2, G.cardW, G.cardH));
    }
    for (int i = 0; i < run->shop_pack_count; i++) {
        float x, y2;
        pack_place(i, &x, &y2);
        draw_item(&run->shop_packs[i], x, y2, G.cardW * 1.1f, G.cardH * 1.1f);
        draw_price(run->shop_packs[i].cost, x + G.cardW * 0.05f, y2);
        bo_ui_area(BO_ID_PACK + i, bo_rect(x, y2, G.cardW * 1.1f, G.cardH * 1.1f));
    }
}

// --- The focus of a card ---------------------------------------------------

// The place of the voucher and of the packs in the shop.
static void voucher_place(float *x, float *y) {
    BORect box = shop_box();
    *x = box.x + 8 + (box.w - 16) * 0.08f;
    *y = box.y + box.h - 16 - G.cardH * 1.1f;
}

static void pack_place(int index, float *x, float *y) {
    BORect box = shop_box();
    *x = box.x + 8 + (box.w - 16) * 0.42f + index * (G.cardW * 1.25f);
    *y = box.y + box.h - 16 - G.cardH * 1.1f;
}

// The place of a card of the open pack.
static void pack_card_place(int index, int count, float *x, float *y) {
    BORect area = main_area();
    BORect row = bo_rect(area.x + area.w * 0.05f, 0, area.w * 0.9f, G.cardH);
    *x = slot_x(row, index, count);
    *y = G.height * 0.295f;
}

// A popup over the side panel after a card was used: the new level of a
// hand, or the money.
static void after_use(const BOItem *used, int dollarsBefore) {
    const BOCenterInfo *c = bo_item_center(used);
    char text[48];
    float x = panel_width() / 2, y = G.height * 0.36f;
    const char *hand = bo_center_text(c, BO_F_HAND_TYPE);
    if (hand) {
        int type = bo_rules_hand_by_name(hand);
        if (type >= 0) {
            snprintf(text, sizeof(text), "%s lvl.%d", hand, G.run->hands[type].level);
            popup(text, x, y, BO_C_ORANGE);
        }
    } else if (used->center == BO_C_BLACK_HOLE) {
        popup("All hands +1", x, y, BO_C_ORANGE);
    }
    int gained = G.run->dollars - dollarsBefore;
    if (gained != 0) {
        snprintf(text, sizeof(text), gained > 0 ? "+$%d" : "-$%d", gained > 0 ? gained : -gained);
        popup(text, x, G.height * 0.66f, BO_C_MONEY);
    }
    sync_hand();
    clear_selection();
    G.focus = BO_FOCUS_NONE;
}

static void draw_focus(void) {
    BORun *run = G.run;
    float x = 0, y = 0;
    const BOItem *item = NULL;
    BOAction actions[2];
    int actionCount = 0;
    char sell[24];
    int idle = G.flow == BO_FLOW_IDLE;
    int count = 0;
    uint64_t mask = selected_mask(&count);
    switch (G.focus) {
    case BO_FOCUS_JOKER:
        if (G.focusIndex >= run->joker_count) break;
        item = &run->jokers[G.focusIndex];
        x = G.jokerViews[G.focusIndex].x;
        y = G.jokerViews[G.focusIndex].y;
        snprintf(sell, sizeof(sell), "Sell $%d", item->sell_cost);
        actions[actionCount++] = (BOAction){sell, BO_C_GREEN, !item->eternal && idle};
        break;
    case BO_FOCUS_CONSUMABLE:
        if (G.focusIndex >= run->consumable_count) break;
        item = &run->consumables[G.focusIndex];
        x = G.consumableViews[G.focusIndex].x;
        y = G.consumableViews[G.focusIndex].y;
        actions[actionCount++] =
            (BOAction){"Use", BO_C_RED, idle && bo_run_can_use_consumable(run, G.focusIndex, mask)};
        snprintf(sell, sizeof(sell), "Sell $%d", item->sell_cost);
        actions[actionCount++] = (BOAction){sell, BO_C_GREEN, idle};
        break;
    case BO_FOCUS_SHOP:
        if (G.focusIndex >= run->shop_count) break;
        item = &run->shop[G.focusIndex].item;
        shop_card_place(G.focusIndex, run->shop_count, &x, &y);
        actions[actionCount++] = (BOAction){"Buy", BO_C_ORANGE, item->cost <= bo_run_spendable(run) || item->cost <= 0};
        break;
    case BO_FOCUS_VOUCHER:
        if (!run->has_shop_voucher) break;
        item = &run->shop_voucher;
        voucher_place(&x, &y);
        actions[actionCount++] = (BOAction){"Redeem", BO_C_ORANGE, idle && bo_voucher_can_redeem(run)};
        break;
    case BO_FOCUS_PACK:
        if (G.focusIndex >= run->shop_pack_count) break;
        item = &run->shop_packs[G.focusIndex];
        pack_place(G.focusIndex, &x, &y);
        actions[actionCount++] = (BOAction){"Open", BO_C_GREEN, item->cost <= bo_run_spendable(run) || item->cost <= 0};
        break;
    case BO_FOCUS_BLIND_TAG:
    case BO_FOCUS_HELD_TAG: {
        if (G.focus == BO_FOCUS_HELD_TAG && G.focusIndex >= run->tag_count) break;
        if (G.focus == BO_FOCUS_BLIND_TAG && run->state != BO_STATE_BLIND_SELECT) break;
        BOCenterId tag = G.focus == BO_FOCUS_HELD_TAG ? run->tags[G.focusIndex].center : run->blind_tags[G.focusIndex];
        int orbital = G.focus == BO_FOCUS_HELD_TAG ? run->tags[G.focusIndex].orbital_hand
                                                  : run->orbital_choices[G.focusIndex];
        float tx = G.blindTagX[G.focusIndex & 1], ty = G.blindTagY[G.focusIndex & 1];
        if (G.focus == BO_FOCUS_HELD_TAG) held_tag_place(G.focusIndex, &tx, &ty);
        BODescription d;
        bo_describe_tag(run, tag, orbital, &d);
        draw_text_box(&d, NULL, 0, tx, ty, tag_size(), tag_size(), NULL, 0, 0);
        return;
    }
    case BO_FOCUS_TAG_VOUCHER:
        if (G.focusIndex >= run->tag_voucher_count) break;
        item = &run->tag_vouchers[G.focusIndex];
        voucher_place(&x, &y);
        x += (run->has_shop_voucher ? G.focusIndex + 1 : G.focusIndex) * (G.cardW + 8);
        actions[actionCount++] =
            (BOAction){"Redeem", BO_C_ORANGE, idle && bo_voucher_can_redeem_tag(run, G.focusIndex)};
        break;
    case BO_FOCUS_PACK_CARD:
        if (run->state != BO_STATE_PACK || G.focusIndex >= run->pack_count) break;
        item = &run->pack_cards[G.focusIndex].item;
        pack_card_place(G.focusIndex, run->pack_count, &x, &y);
        actions[actionCount++] = (BOAction){bo_item_center(item)->consumeable ? "Use" : "Select",
                                            BO_C_GREEN, idle && bo_pack_can_choose(run, G.focusIndex, mask)};
        break;
    default: break;
    }
    if (!item) {
        G.focus = BO_FOCUS_NONE;
        return;
    }
    // A playing card has no text box; only its button shows.
    int playing = (G.focus == BO_FOCUS_SHOP && run->shop[G.focusIndex].is_playing_card) ||
                  (G.focus == BO_FOCUS_PACK_CARD && run->pack_cards[G.focusIndex].is_playing_card);
    int tapped = 0;
    if (playing) {
        BORect button = bo_rect(x - G.cardW * 0.1f, y + G.cardH + 8, G.cardW * 1.2f, G.height * 0.07f);
        tapped = bo_ui_button(BO_ID_ACTION, button, actions[0].colour, actions[0].label, actions[0].enabled);
    } else {
        tapped = draw_card_text(item, x, y, actions, actionCount, G.focus == BO_FOCUS_PACK_CARD && G.handCount > 0);
    }
    if (!tapped) return;
    int done = 0;
    int dollars = run->dollars;
    BOItem used = *item;
    switch (G.focus) {
    case BO_FOCUS_JOKER: done = bo_run_sell_joker(run, G.focusIndex); break;
    case BO_FOCUS_CONSUMABLE:
        if (tapped == 1) {
            done = bo_run_use_consumable(run, G.focusIndex, mask);
            if (done) after_use(&used, dollars);
        } else {
            done = bo_run_sell_consumable(run, G.focusIndex);
        }
        break;
    case BO_FOCUS_SHOP: done = bo_run_buy(run, G.focusIndex); break;
    case BO_FOCUS_VOUCHER: done = bo_voucher_redeem(run); break;
    case BO_FOCUS_TAG_VOUCHER: done = bo_voucher_redeem_tag(run, G.focusIndex); break;
    case BO_FOCUS_PACK:
        done = bo_run_open_pack(run, G.focusIndex);
        if (done) sync_hand();
        break;
    case BO_FOCUS_PACK_CARD:
        done = bo_pack_choose(run, G.focusIndex, mask);
        if (done) after_use(&used, dollars);
        break;
    default: break;
    }
    if (done) G.focus = BO_FOCUS_NONE;
    fprintf(stderr, "action %d: %s\n", tapped, done ? "done" : "refused");
}

static void toggle_focus(BOFocusArea area, int index) {
    int same = G.focus == area && G.focusIndex == index;
    G.focus = same ? BO_FOCUS_NONE : area;
    G.focusIndex = index;
}

static void handle_focus_taps(void) {
    BORun *run = G.run;
    for (int i = 0; i < run->joker_count; i++) {
        if (bo_ui_clicked(BO_ID_JOKER + i)) toggle_focus(BO_FOCUS_JOKER, i);
    }
    for (int i = 0; i < run->consumable_count; i++) {
        if (bo_ui_clicked(BO_ID_CONSUMABLE + i)) toggle_focus(BO_FOCUS_CONSUMABLE, i);
    }
    for (int i = 0; i < run->shop_count; i++) {
        if (bo_ui_clicked(BO_ID_SHOP + i)) toggle_focus(BO_FOCUS_SHOP, i);
    }
    if (bo_ui_clicked(BO_ID_VOUCHER)) toggle_focus(BO_FOCUS_VOUCHER, 0);
    for (int i = 0; i < run->shop_pack_count; i++) {
        if (bo_ui_clicked(BO_ID_PACK + i)) toggle_focus(BO_FOCUS_PACK, i);
    }
    for (int i = 0; i < run->pack_count; i++) {
        if (bo_ui_clicked(BO_ID_PACK_CARD + i)) toggle_focus(BO_FOCUS_PACK_CARD, i);
    }
    for (int i = 0; i < 2; i++) {
        if (bo_ui_clicked(BO_ID_BLIND_TAG + i)) toggle_focus(BO_FOCUS_BLIND_TAG, i);
    }
    for (int i = 0; i < run->tag_count; i++) {
        if (bo_ui_clicked(BO_ID_HELD_TAG + i)) toggle_focus(BO_FOCUS_HELD_TAG, i);
    }
    for (int i = 0; i < run->tag_voucher_count; i++) {
        if (bo_ui_clicked(BO_ID_TAG_VOUCHER + i)) toggle_focus(BO_FOCUS_TAG_VOUCHER, i);
    }
    if (bo_ui_clicked(BO_ID_BACKGROUND)) G.focus = BO_FOCUS_NONE;
}

// --- The open pack ---------------------------------------------------------

static void draw_pack(void) {
    BORun *run = G.run;
    const BOCenterInfo *pack = bo_center(run->pack);
    BORect area = main_area();
    BODescription d;
    BOItem shown;
    memset(&shown, 0, sizeof(shown));
    shown.center = run->pack;
    bo_describe_item(run, &shown, &d);
    BORect box = bo_rect(area.x, G.height * 0.23f, area.w, G.cardH + G.height * 0.15f);
    bo_ui_box(box, BO_C_BLACK);
    float titleH = G.height * 0.05f;
    bo_font_draw(d.name, box.x + box.w / 2, box.y + 6, titleH, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    for (int i = 0; i < run->pack_count; i++) {
        float x, y;
        pack_card_place(i, run->pack_count, &x, &y);
        if (G.focus == BO_FOCUS_PACK_CARD && G.focusIndex == i) y -= G.cardH * 0.08f;
        const BOShopCard *card = &run->pack_cards[i];
        if (card->is_playing_card) {
            draw_playing_card(&card->card, x, y, G.cardW, G.cardH);
        } else {
            draw_item(&card->item, x, y, G.cardW, G.cardH);
        }
        if (G.flow == BO_FLOW_IDLE) bo_ui_area(BO_ID_PACK_CARD + i, bo_rect(x, y, G.cardW, G.cardH));
    }
    char text[32];
    snprintf(text, sizeof(text), "Choose %d", run->pack_choices);
    bo_font_draw(text, box.x + 16, box.y + box.h - titleH - 6, titleH * 0.8f, bo_rgb(BO_C_WHITE), BO_ALIGN_LEFT);
    float bw = area.w * 0.2f, bh = G.height * 0.07f;
    if (bo_ui_button(BO_ID_SKIP, bo_rect(box.x + box.w - bw - 12, box.y + box.h - bh - 8, bw, bh), BO_C_GREY, "Skip",
                     bo_pack_can_skip(run) && G.flow == BO_FLOW_IDLE)) {
        bo_pack_skip(run);
        G.focus = BO_FOCUS_NONE;
        sync_hand();
        return;
    }
    if (G.handCount > 0) {
        draw_hand_cards(G.flow == BO_FLOW_IDLE);
        handle_hand_taps();
    }
    (void)pack;
}

// --- Run Info --------------------------------------------------------------

static void draw_run_info(void) {
    BORun *run = G.run;
    bo_gfx_rect(0, 0, G.width, G.height, bo_rgba(0x000000, 0.55f));
    BORect box = bo_rect(G.width * 0.12f, G.height * 0.04f, G.width * 0.76f, G.height * 0.92f);
    bo_ui_box(box, BO_C_BLACK);
    float rowH = G.height * 0.062f, size = rowH * 0.62f;
    bo_font_draw("Poker Hands", box.x + box.w / 2, box.y + 10, rowH * 0.8f, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
    float y = box.y + rowH * 1.3f;
    char text[32];
    for (int h = 0; h < BO_HAND_COUNT; h++) {
        if (!run->hands[h].visible) continue;
        BORect row = bo_rect(box.x + 12, y, box.w - 24, rowH - 6);
        bo_ui_box(row, BO_C_DARK);
        int level = run->hands[h].level;
        snprintf(text, sizeof(text), "lvl.%d", level);
        bo_font_draw(text, row.x + 10, row.y + row.h * 0.18f, size, bo_rgb(level > 1 ? BO_C_ORANGE : BO_C_WHITE),
                     BO_ALIGN_LEFT);
        bo_font_draw(bo_hand_name((BOHandType)h, 0), row.x + row.w * 0.16f, row.y + row.h * 0.18f, size,
                     bo_rgb(BO_C_WHITE), BO_ALIGN_LEFT);
        float cw = row.w * 0.13f;
        BORect chips = bo_rect(row.x + row.w * 0.56f, row.y + 3, cw, row.h - 6);
        BORect mult = bo_rect(chips.x + cw + row.w * 0.03f, row.y + 3, cw, row.h - 6);
        bo_ui_box(chips, BO_C_BLUE);
        bo_ui_box(mult, BO_C_RED);
        snprintf(text, sizeof(text), "%d", bo_hand_level_chips((BOHandType)h, level));
        bo_font_draw(text, chips.x + chips.w / 2, chips.y + chips.h * 0.12f, size, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
        bo_font_draw("X", chips.x + cw + row.w * 0.015f, chips.y + chips.h * 0.12f, size, bo_rgb(BO_C_RED),
                     BO_ALIGN_CENTER);
        snprintf(text, sizeof(text), "%d", bo_hand_level_mult((BOHandType)h, level));
        bo_font_draw(text, mult.x + mult.w / 2, mult.y + mult.h * 0.12f, size, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
        snprintf(text, sizeof(text), "# %d", run->hands[h].played);
        bo_font_draw(text, row.x + row.w - 10, row.y + row.h * 0.18f, size, bo_rgb(BO_C_ORANGE), BO_ALIGN_RIGHT);
        y += rowH;
    }
    // Any tap closes the screen. The New Run button is over that area.
    bo_ui_area(BO_ID_RUN_INFO + 1000, bo_rect(0, 0, G.width, G.height));
    if (bo_ui_clicked(BO_ID_RUN_INFO + 1000)) G.showInfo = 0;
    float bw = box.w * 0.22f, bh = rowH * 0.9f;
    if (bo_ui_button(BO_ID_INFO_NEW_RUN, bo_rect(box.x + box.w - bw - 12, box.y + 8, bw, bh), BO_C_RED, "New Run",
                     1)) {
        G.showInfo = 0;
        G.setup = 1;
    }
}

// --- The frame -------------------------------------------------------------

// The alert of a card that the profile unlocked, at the top of the screen.
static void draw_notice(double dt) {
    if (!G.profile) return;
    if (G.notice == BO_CENTER_NONE) {
        G.notice = bo_profile_take_notice(G.profile);
        G.noticeAge = 0;
        if (G.notice == BO_CENTER_NONE) return;
    }
    G.noticeAge += (float)dt;
    if (G.noticeAge > 3) {
        G.notice = BO_CENTER_NONE;
        return;
    }
    BOItem item;
    memset(&item, 0, sizeof(item));
    item.center = G.notice;
    BODescription d;
    bo_describe_item(NULL, &item, &d);
    char text[96];
    snprintf(text, sizeof(text), "Unlocked: %s", d.name);
    float size = G.height * 0.05f, w = bo_font_width(text, size) + 40, h = size * 1.6f;
    float slide = G.noticeAge < 0.3f ? G.noticeAge / 0.3f : G.noticeAge > 2.7f ? (3 - G.noticeAge) / 0.3f : 1;
    float y = -h + (h + 10) * slide;
    BORect box = bo_rect((G.width - w) / 2, y, w, h);
    bo_ui_box(box, BO_C_ORANGE);
    bo_font_draw(text, G.width / 2, y + h * 0.18f, size, bo_rgb(BO_C_WHITE), BO_ALIGN_CENTER);
}

static void draw_popups(double dt) {
    for (int i = 0; i < 24; i++) {
        BOPopup *p = &G.popups[i];
        if (p->age <= 0 || p->age > 1.2f) continue;
        p->age += (float)dt;
        float alpha = p->age < 0.9f ? 1 : (1.2f - p->age) / 0.3f;
        float size = G.height * 0.06f;
        bo_font_draw(p->text, p->x + 2, p->y - p->age * 20 + 2, size, bo_rgba(0x000000, alpha * 0.4f),
                     BO_ALIGN_CENTER);
        bo_font_draw(p->text, p->x, p->y - p->age * 20, size, bo_rgba(p->color, alpha), BO_ALIGN_CENTER);
    }
}

// Counts a shown number up to its value in about half a second.
static void count_to(double *shown, double value, double dt) {
    if (fabs(value - *shown) < 1) {
        *shown = value;
    } else {
        *shown += (value - *shown) * (1 - exp(-dt * 8));
    }
}

static BOColor scale_colour(BOColor c, float f) {
    BOColor out = {c.r * f, c.g * f, c.b * f, c.a};
    return out;
}

static BOColor mix_colour(BOColor a, BOColor b, float part_a) {
    BOColor out = {a.r * part_a + b.r * (1 - part_a), a.g * part_a + b.g * (1 - part_a),
                   a.b * part_a + b.b * (1 - part_a), a.a * part_a + b.a * (1 - part_a)};
    return out;
}

static void approach_colour(BOColor *c, BOColor target, float k) {
    c->r += (target.r - c->r) * k;
    c->g += (target.g - c->g) * k;
    c->b += (target.b - c->b) * k;
    c->a += (target.a - c->a) * k;
}

// ease_background_colour_blind: the colours of the background for the state
// of the run, which the background moves to in about 0.6 seconds.
static void update_background(double dt) {
    BORun *run = G.run;
    BOColor black = bo_rgb(0x374244), darkBlack = scale_colour(bo_rgb(0x374244), 0.8f);
    BOColor newColour = bo_rgb(0x50846e), special = {0, 0, 0, 0}, tertiary = {0, 0, 0, 0};
    int hasSpecial = 0, hasTertiary = 0;
    float contrast = 1, spin = 0;
    const BOCenterInfo *pack = run->state == BO_STATE_PACK ? bo_center(run->pack) : NULL;
    const char *kind = pack && pack->kind ? pack->kind : "";
    const BOBlindInfo *blind = bo_blind_info(run->blind);
    if (pack && strcmp(kind, "Arcana") == 0) {
        newColour = bo_rgb(0x8867a5), special = darkBlack, hasSpecial = 1, contrast = 1.5f;
    } else if (pack && strcmp(kind, "Spectral") == 0) {
        newColour = bo_rgb(0x4584fa), special = darkBlack, hasSpecial = 1, contrast = 2;
    } else if (pack && strcmp(kind, "Standard") == 0) {
        newColour = darkBlack, special = bo_rgb(0xfe5f55), hasSpecial = 1, contrast = 3;
    } else if (pack && strcmp(kind, "Buffoon") == 0) {
        newColour = bo_rgb(0xff9a00), special = black, hasSpecial = 1, contrast = 2;
    } else if (pack) {
        newColour = black, contrast = 3;
    } else if (run->won && G.endless == 0) {
        newColour = bo_rgb(0x4f6367);
    } else if (run->state == BO_STATE_SELECTING && blind->boss) {
        if (blind->showdown) {
            newColour = bo_rgb(0x009dff), special = bo_rgb(0xfe5f55), tertiary = scale_colour(black, 0.6f);
            hasSpecial = hasTertiary = 1;
            contrast = 3;
            spin = 0.5f;
        } else {
            BOColor boss = bo_rgb(blind->colour);
            BOColor mixed = mix_colour(boss, black, 0.3f);
            newColour = (BOColor){mixed.r * 0.9f + 0.1f, mixed.g * 0.9f + 0.1f, mixed.b * 0.9f + 0.1f, 1};
            special = boss, hasSpecial = 1, contrast = 2;
            spin = 0.25f;
        }
    }
    BOColor targetL, targetC, targetD;
    if (hasSpecial && hasTertiary) {
        targetL = newColour, targetC = special, targetD = tertiary;
    } else {
        targetL = scale_colour(newColour, 1.3f);
        targetD = scale_colour(newColour, hasSpecial ? 0.4f : 0.7f);
        targetC = hasSpecial ? special : scale_colour(newColour, 0.9f);
    }
    targetL.a = targetC.a = targetD.a = 1;
    if (G.backContrast == 0) {
        G.backL = targetL, G.backC = targetC, G.backD = targetD, G.backContrast = contrast;
    }
    float k = (float)(1 - exp(-dt * 6));
    approach_colour(&G.backL, targetL, k);
    approach_colour(&G.backC, targetC, k);
    approach_colour(&G.backD, targetD, k);
    G.backContrast += (contrast - G.backContrast) * k;
    G.backSpin += (spin - G.backSpin) * (float)(1 - exp(-dt * 2));
    G.backSpinTime += (float)dt * G.backSpin;
}

// The jokers fan a little and sway, as align_cards puts them. A focused
// card lifts, and a held joker follows the finger.
static void move_item_row(BOItemView *views, int count, BORect area, BOFocusArea focus, double dt) {
    float u = unit();
    for (int i = 0; i < count; i++) {
        BOItemView *v = &views[i];
        float tx = slot_x(area, i, count), ty = area.y, tr = 0;
        if (G.focus == focus && G.focusIndex == i) ty -= G.cardH * 0.12f;
        if (focus == BO_FOCUS_JOKER) {
            tr = bo_motion_fan_angle(i, count, 0.1f) + 0.02f * sinf(2 * (float)G.time + tx / u);
            if (G.dragging && i == G.dragIndex) {
                tx = G.dragX - G.cardW / 2;
                ty = area.y - G.cardH * 0.1f;
            }
        }
        bo_motion_step(&v->m, tx / u, ty / u, tr, 1, dt, G.time);
        v->x = v->m.x * u;
        v->y = v->m.y * u;
    }
}

static void move_items(double dt) {
    move_item_row(G.jokerViews, G.jokerViewCount, joker_area(), BO_FOCUS_JOKER, dt);
    move_item_row(G.consumableViews, G.consumableViewCount, consumable_area(), BO_FOCUS_CONSUMABLE, dt);
}

// update_canvas_juice with the screen shake at 50, as it starts. The
// background does not move.
static void shake_room(void) {
    const float amount = 1.5f;
    float t = (float)G.time, u = unit();
    float r = (0.001f * sinf(0.3f * t) + 0.002f * G.jiggle * sinf(39.913f * t)) * amount;
    float dx = amount * (0.015f * sinf(0.913f * t) + 0.01f * G.jiggle * amount * sinf(19.913f * t));
    float dy = amount * (0.015f * sinf(0.952f * t) + 0.01f * G.jiggle * amount * sinf(21.913f * t));
    bo_gfx_room(r, dx * u, dy * u);
}

void bo_game_frame(double dt) {
    bo_debug_poll(dt);
    G.time += dt;
    G.frames++;
    if (G.time - G.fpsTime >= 5) {
        fprintf(stderr, "fps %.1f, draw calls %d\n", G.frames / (G.time - G.fpsTime), bo_gfx_draw_calls());
        fflush(stderr);
        G.frames = 0;
        G.fpsTime = G.time;
    }
    update_flow(dt);
    autosave();
    for (int i = 0; i < G.handCount; i++) move_view(&G.hand[i], dt);
    for (int i = 0; i < G.playCount; i++) move_view(&G.play[i], dt);
    move_items(dt);
    bo_motion_step(&G.chipsBox, 0, 0, 0, 1, dt, G.time);
    bo_motion_step(&G.multBox, 0, 0, 0, 1, dt, G.time);
    G.jiggle *= (float)fmax(0, 1 - 5 * dt);
    // The tallies of the jokers, as Card:update keeps them.
    for (int i = 0; i < G.run->joker_count; i++) bo_joker_update(G.run, &G.run->jokers[i]);
    if (G.flow != BO_FLOW_SCORE) count_to(&G.shownRound, G.run->chips, dt);
    count_to(&G.shownDollars, G.run->dollars, dt);

    // The hand shows only while a blind is on or a pack shows it.
    if (G.flow == BO_FLOW_IDLE && G.run->state != BO_STATE_SELECTING && G.run->state != BO_STATE_PACK) {
        G.handCount = 0;
    }
    bo_ui_begin();
    update_background(dt);
    bo_gfx_set_background((float)G.time, G.backSpinTime, G.backC, G.backL, G.backD, G.backContrast, G.backSpin);
    bo_gfx_begin(G.width, G.height, bo_rgb(0x3a5c4f));
    shake_room();
    if (G.unlockConfirm > 0) G.unlockConfirm -= (float)dt;
    if (G.collection) {
        draw_collection();
        draw_notice(dt);
        bo_gfx_flush();
        bo_ui_end();
        return;
    }
    if (G.setup) {
        draw_setup();
        draw_notice(dt);
        bo_gfx_flush();
        bo_ui_end();
        return;
    }
    // A tap on the table takes the focus away.
    bo_ui_area(BO_ID_BACKGROUND, bo_rect(0, 0, G.width, G.height));
    draw_panel();
    draw_deck();
    BORunState state = G.run->state;
    if (state == BO_STATE_BLIND_SELECT) {
        draw_blind_select();
    } else if (state == BO_STATE_SELECTING || G.flow != BO_FLOW_IDLE) {
        draw_round();
    } else if (state == BO_STATE_ROUND_EVAL) {
        if (G.run->won && !G.endless) {
            draw_end(1);
        } else {
            draw_cash_out();
        }
    } else if (state == BO_STATE_SHOP) {
        draw_shop();
    } else if (state == BO_STATE_GAME_OVER) {
        draw_end(0);
    } else if (state == BO_STATE_PACK) {
        draw_pack();
    }
    draw_items();
    draw_held_tags();
    handle_focus_taps();
    if (G.focus != BO_FOCUS_NONE) draw_focus();
    draw_popups(dt);
    draw_notice(dt);
    if (G.showInfo) draw_run_info();
    bo_gfx_flush();
    bo_ui_end();
}

void bo_game_debug(const char *line) {
    double value;
    char word[64];
    if (sscanf(line, "target %lf", &value) == 1) {
        G.run->blind_chips = value;
        fprintf(stderr, "debug: target %.0f\n", value);
    } else if (sscanf(line, "ante %lf", &value) == 1) {
        G.run->ante = (int)value;
        fprintf(stderr, "debug: ante %.0f\n", value);
    } else if (sscanf(line, "dollars %lf", &value) == 1) {
        G.run->dollars = (int)value;
        fprintf(stderr, "debug: dollars %.0f\n", value);
    } else if (sscanf(line, "joker %63s", word) == 1) {
        BOCenterId center = bo_center_by_key(word);
        if (center != BO_CENTER_NONE) {
            BOItem item = bo_run_create_item(G.run, 0, 0, 0, 0, center, NULL, 0);
            if (bo_center(center)->consumeable) {
                bo_run_add_consumable(G.run, &item);
            } else {
                bo_run_add_joker(G.run, &item);
            }
        }
        fprintf(stderr, "debug: joker %s\n", word);
    } else if (sscanf(line, "edition %63s", word) == 1) {
        // The edition of the last joker.
        static const char *names[] = {"none", "foil", "holo", "polychrome", "negative"};
        for (int e = 0; e < 5; e++) {
            if (strcmp(word, names[e]) == 0 && G.run->joker_count > 0) {
                G.run->jokers[G.run->joker_count - 1].edition = (uint8_t)e;
            }
        }
        fprintf(stderr, "debug: edition %s\n", word);
    } else if (sscanf(line, "boss %63s", word) == 1) {
        // The boss comes next.
        for (int b = 0; b < BO_BLIND_COUNT; b++) {
            if (strcmp(bo_blind_info((BOBlindId)b)->key, word) == 0 && G.run->state == BO_STATE_BLIND_SELECT) {
                G.run->boss = (BOBlindId)b;
                G.run->slot = BO_SLOT_BOSS;
            }
        }
        fprintf(stderr, "debug: boss %s\n", word);
    } else if (sscanf(line, "voucher %63s", word) == 1) {
        BOCenterId center = bo_center_by_key(word);
        if (center != BO_CENTER_NONE && bo_center(center)->set == BO_SET_VOUCHER) {
            G.run->used_vouchers[center] = 1;
            bo_voucher_apply(G.run, center);
        }
        fprintf(stderr, "debug: voucher %s\n", word);
    } else if (sscanf(line, "sticker %63s", word) == 1) {
        // A sticker on the last joker.
        if (G.run->joker_count > 0) {
            BOItem *joker = &G.run->jokers[G.run->joker_count - 1];
            if (strcmp(word, "eternal") == 0) joker->eternal = 1;
            if (strcmp(word, "perishable") == 0) joker->perishable = 1, joker->perish_tally = 5;
            if (strcmp(word, "rental") == 0) joker->rental = 1;
        }
        fprintf(stderr, "debug: sticker %s\n", word);
    } else if (sscanf(line, "stake %lf", &value) == 1) {
        // The stake of the next run.
        if (value >= 1 && value <= 8) G.setupStake = (int)value;
        fprintf(stderr, "debug: stake %.0f\n", value);
    } else if (sscanf(line, "deck %63s", word) == 1) {
        // A new run with this deck.
        BOCenterId center = bo_center_by_key(word);
        if (center != BO_CENTER_NONE && bo_center(center)->set == BO_SET_BACK) {
            G.setupBack = center;
            G.setup = 0;
            new_run();
        }
        fprintf(stderr, "debug: deck %s\n", word);
    } else if (sscanf(line, "pack %63s", word) == 1) {
        BOCenterId center = bo_center_by_key(word);
        if (center != BO_CENTER_NONE && G.run->state == BO_STATE_SHOP) {
            bo_pack_open(G.run, center, 0);
            sync_hand();
        }
        fprintf(stderr, "debug: pack %s\n", word);
    } else {
        fprintf(stderr, "debug: unknown command: %s", line);
    }
}

// A touch on a joker can be a tap or the start of a drag.
static int joker_at(float x, float y) {
    BORect area = joker_area();
    for (int i = G.jokerViewCount - 1; i >= 0; i--) {
        if (x >= G.jokerViews[i].x && x < G.jokerViews[i].x + G.cardW && y >= area.y && y < area.y + G.cardH) return i;
    }
    return -1;
}

void bo_game_touch(int phase, float x, float y) {
    if (phase == BO_TOUCH_BEGAN) {
        G.dragIndex = G.flow == BO_FLOW_IDLE ? joker_at(x, y) : -1;
        G.dragging = 0;
        G.dragStartX = x;
        G.dragStartY = y;
        G.dragX = x;
        if (G.dragIndex < 0) bo_ui_tap(x, y);
    } else if (phase == BO_TOUCH_MOVED) {
        if (G.dragIndex >= 0 && (G.dragging || fabsf(x - G.dragStartX) > 14)) {
            G.dragging = 1;
            G.dragX = x;
        }
    } else if (phase == BO_TOUCH_ENDED) {
        if (G.dragIndex >= 0) {
            if (G.dragging) {
                // The new place is the slot nearest to the finger.
                BORect area = joker_area();
                int count = G.run->joker_count, best = G.dragIndex;
                float bestDistance = 1e9f;
                for (int i = 0; i < count; i++) {
                    float d = fabsf(slot_x(area, i, count) + G.cardW / 2 - x);
                    if (d < bestDistance) {
                        bestDistance = d;
                        best = i;
                    }
                }
                bo_run_move_joker(G.run, G.dragIndex, best);
                G.focus = BO_FOCUS_NONE;
            } else {
                bo_ui_tap(G.dragStartX, G.dragStartY);
            }
        }
        G.dragIndex = -1;
        G.dragging = 0;
    }
}
