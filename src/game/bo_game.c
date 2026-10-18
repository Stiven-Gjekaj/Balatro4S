#include "bo_game.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bo_blind.h"
#include "bo_card.h"
#include "bo_debug.h"
#include "bo_font.h"
#include "bo_gfx.h"
#include "bo_hand.h"
#include "bo_image.h"
#include "bo_run.h"
#include "bo_score.h"
#include "bo_ui.h"
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
    BO_ID_HAND_CARD = 100,
};

typedef struct {
    BOCard card;
    float x, y;
    float tx, ty;
    float pulse;  // A short grow when the card scores, from 1 down to 0.
    int selected;
} BOCardView;

typedef struct {
    char text[16];
    float x, y;
    float age;
    unsigned int color;
} BOPopup;

typedef enum {
    BO_FLOW_IDLE,     // The screen follows the state of the run.
    BO_FLOW_SCORE,    // The played cards score, one event at a time.
    BO_FLOW_CLEAR,    // The played cards leave.
} BOFlow;

static struct {
    char error[256];
    int width, height;
    float cardW, cardH;
    BOTexture deck;
    BOTexture centers;
    BOTexture chips;  // BlindChips.png: 21 frames for each blind.

    BORun *run;
    BOCardView hand[BO_MAX_HAND];
    int handCount;
    BOCardView play[BO_MAX_PLAY];
    int playCount;

    BOScore score;
    int nextEvent;
    BOFlow flow;
    double wait;
    double shownChips, shownMult;
    const char *shownHand;
    double shownRound;  // The round score on the screen. It counts up.
    double shownDollars;
    BOPopup popups[16];
    int endless;  // The player goes on after the win.

    double time;
    int frames;
    double fpsTime;
} G;

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

// The areas of the screen.
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

static void layout_row(BOCardView *cards, int count, BORect area, float y, float lift) {
    float step = count > 1 ? (area.w - G.cardW) / (count - 1) : 0;
    if (step > G.cardW * 1.05f) step = G.cardW * 1.05f;
    float total = step * (count - 1) + G.cardW;
    float startX = area.x + (area.w - total) / 2;
    for (int i = 0; i < count; i++) {
        cards[i].tx = startX + step * i;
        cards[i].ty = y - (cards[i].selected ? lift : 0);
    }
}

static void layout(void) {
    layout_row(G.hand, G.handCount, main_area(), hand_top(), G.cardH * 0.2f);
    if (G.flow != BO_FLOW_CLEAR) layout_row(G.play, G.playCount, main_area(), G.height * 0.28f, G.cardH * 0.12f);
}

// Makes the cards on the screen follow the hand of the run. A card that
// stays keeps its place, and a new card comes from the deck.
static void sync_hand(void) {
    BOCardView views[BO_MAX_HAND];
    int count = 0;
    for (int i = 0; i < G.run->hand_count; i++) {
        BOCardView view;
        memset(&view, 0, sizeof(view));
        view.card = G.run->hand[i];
        view.x = deck_x();
        view.y = hand_top();
        for (int j = 0; j < G.handCount; j++) {
            if (G.hand[j].card.uid == view.card.uid) {
                view = G.hand[j];
                view.card = G.run->hand[i];
                break;
            }
        }
        views[count++] = view;
    }
    memcpy(G.hand, views, sizeof(BOCardView) * count);
    G.handCount = count;
    layout();
}

static void popup(const char *text, float x, float y, unsigned int color) {
    for (int i = 0; i < 16; i++) {
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

static void new_run(void) {
    // Balatro makes the seed from the place and the time of the cursor.
    // Here the time of the clock gives the number.
    char seed[9];
    bo_random_string((double)time(NULL) * 0.33411983 + fmod(G.time, 1000) * 0.412311010, seed);
    bo_run_start(G.run, seed);
    G.handCount = 0;
    G.playCount = 0;
    G.flow = BO_FLOW_IDLE;
    G.shownHand = NULL;
    G.shownRound = 0;
    G.shownDollars = G.run->dollars;
    G.endless = 0;
    fprintf(stderr, "new run: seed %s\n", seed);
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

    size_t fontSize = 0;
    unsigned char *font = bo_zip_read(zip, "resources/fonts/m6x11plus.ttf", &fontSize);
    int fontLoaded = font && bo_font_load(font, fontSize, 48);
    free(font);
    bo_zip_close(zip);
    if (!G.deck.id || !G.centers.id || !G.chips.id || !fontLoaded) {
        fail("Balatro.love does not have the textures or the font that Balatro4S needs.");
        return 0;
    }
    G.run = calloc(1, sizeof(BORun));
    new_run();
    return 1;
}

// The row of each suit in 8BitDeck.png.
static int deck_row(int suit) {
    switch (suit) {
    case BO_HEARTS: return 0;
    case BO_CLUBS: return 1;
    case BO_DIAMONDS: return 2;
    default: return 3;
    }
}

static void draw_card(const BOCardView *view) {
    float grow = 1 + view->pulse * 0.12f;
    float w = G.cardW * grow, h = G.cardH * grow;
    float x = view->x - (w - G.cardW) / 2, y = view->y - (h - G.cardH) / 2;
    float cw = G.centers.width / 7.0f, ch = G.centers.height / 5.0f;
    bo_gfx_round_rect(x + G.cardW * 0.04f, y + G.cardH * 0.04f, w, h, G.cardW * 0.06f, bo_rgba(0x000000, 0.25f));
    // The front of a plain card is at (1, 0) in Enhancers.png.
    bo_gfx_sprite(&G.centers, cw * 1, 0, cw, ch, x, y, w, h, 0, bo_rgb(BO_C_WHITE));
    float dw = G.deck.width / 13.0f, dh = G.deck.height / 4.0f;
    bo_gfx_sprite(&G.deck, dw * (view->card.rank - 2), dh * deck_row(view->card.suit), dw, dh, x, y, w, h, 0,
                  bo_rgb(BO_C_WHITE));
}

static void draw_deck(void) {
    float cw = G.centers.width / 7.0f, ch = G.centers.height / 5.0f;
    float x = deck_x(), y = hand_top();
    int count = G.run->deck_count;
    // The Red Deck back is at (0, 0) in Enhancers.png. A few cards under it
    // show the height of the pile.
    int layers = count > 0 ? 1 + count / 13 : 0;
    for (int i = layers - 1; i >= 0; i--) {
        bo_gfx_sprite(&G.centers, 0, 0, cw, ch, x - i * 2, y - i * 2, G.cardW, G.cardH, 0, bo_rgb(BO_C_WHITE));
    }
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

    // The blind, or the title of the blind select.
    BORect blindBox = bo_rect(pad, pad, inner, G.height * 0.2f);
    if (run->state != BO_STATE_BLIND_SELECT || G.flow != BO_FLOW_IDLE) {
        const BOBlindInfo *info = bo_blind_info(run->blind);
        bo_ui_box(blindBox, BO_C_DARK);
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
        bo_ui_box(blindBox, BO_C_DARK);
        const char *title = run->state == BO_STATE_BLIND_SELECT ? "Choose your" : "";
        const char *title2 = run->state == BO_STATE_BLIND_SELECT ? "next Blind" : "";
        bo_font_draw(title, panelW / 2, blindBox.y + blindBox.h * 0.22f, blindBox.h * 0.22f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_CENTER);
        bo_font_draw(title2, panelW / 2, blindBox.y + blindBox.h * 0.5f, blindBox.h * 0.22f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_CENTER);
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
    bo_ui_box(bo_rect(pad * 2, rowY, half, rowH), BO_C_BLUE);
    bo_ui_box(bo_rect(pad * 3 + half, rowY, half, rowH), BO_C_RED);
    if (G.shownHand) {
        format_number(G.shownChips, value, sizeof(value));
        bo_font_draw(value, pad * 2 + half - pad * 0.6f, rowY + rowH * 0.12f, rowH * 0.8f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_RIGHT);
        format_number(G.shownMult, value, sizeof(value));
        bo_font_draw(value, pad * 3 + half + pad * 0.6f, rowY + rowH * 0.12f, rowH * 0.8f, bo_rgb(BO_C_WHITE),
                     BO_ALIGN_LEFT);
    }
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

    // The seed of the run, small at the bottom.
    snprintf(text, sizeof(text), "Seed %s", run->random.seed);
    bo_font_draw(text, panelW / 2, G.height * 0.9f, G.height * 0.035f, bo_rgb(BO_C_GREY), BO_ALIGN_CENTER);
}

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
    BOHandResult result = bo_hand_evaluate(cards, count, &G.run->context.rules);
    G.shownHand = bo_hand_name(result.type, result.royal);
    G.shownChips = bo_hand_level_chips(result.type, G.run->context.levels[result.type]);
    G.shownMult = bo_hand_level_mult(result.type, G.run->context.levels[result.type]);
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
    // The hand keeps only the cards that stay. New cards come after the score.
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

static void update_flow(double dt) {
    G.wait -= dt;
    if (G.wait > 0) return;
    if (G.flow == BO_FLOW_SCORE) {
        if (G.nextEvent < G.score.event_count) {
            const BOScoreEvent *event = &G.score.events[G.nextEvent++];
            BOCardView *card = &G.play[event->index];
            card->pulse = 1;
            char text[16];
            snprintf(text, sizeof(text), "+%.0f", event->value);
            unsigned int color = event->kind == BO_EVENT_CHIPS ? BO_C_BLUE : BO_C_RED;
            popup(text, card->tx + G.cardW / 2, card->ty - G.height * 0.07f, color);
            G.shownChips = event->chips;
            G.shownMult = event->mult;
            G.wait = 0.4;
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
            // The round is over. The cards that are left leave.
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
    // The two sort buttons, in a box with the label "Sort Hand".
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

static void draw_round(void) {
    for (int i = 0; i < G.handCount; i++) {
        BOCardView *c = &G.hand[i];
        draw_card(c);
        if (G.flow == BO_FLOW_IDLE && G.run->state == BO_STATE_SELECTING) {
            bo_ui_area(BO_ID_HAND_CARD + i, bo_rect(c->tx, c->ty, G.cardW, G.cardH));
        }
    }
    for (int i = 0; i < G.playCount; i++) draw_card(&G.play[i]);
    draw_hand_controls();
    for (int i = 0; i < G.handCount; i++) {
        if (!bo_ui_clicked(BO_ID_HAND_CARD + i)) continue;
        int count = 0;
        selected_mask(&count);
        if (!G.hand[i].selected && count >= BO_MAX_PLAY) break;
        G.hand[i].selected = !G.hand[i].selected;
        update_preview();
        layout();
        break;
    }
}

static void draw_blind_select(void) {
    BORun *run = G.run;
    BORect area = main_area();
    float colW = area.w * 0.31f, gap = (area.w - colW * 3) / 2;
    char value[32], text[48];
    for (int slot = 0; slot < 3; slot++) {
        BOBlindId blind = bo_run_slot_blind(run, (BOBlindSlot)slot);
        const BOBlindInfo *info = bo_blind_info(blind);
        int current = slot == (int)run->slot;
        int done = slot < (int)run->slot;
        float x = area.x + slot * (colW + gap);
        float top = current ? G.height * 0.18f : G.height * 0.26f;
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
            bo_ui_button(-1, button, BO_C_GREY, done ? "Defeated" : "Upcoming", 0);
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
        if (!current) bo_gfx_rect(box.x, box.y, box.w, box.h, bo_rgba(0x000000, 0.35f));
    }
}

static void draw_cash_out(void) {
    BORun *run = G.run;
    BORect area = main_area();
    BORect box = bo_rect(area.x + area.w * 0.1f, G.height * 0.16f, area.w * 0.8f, G.height * 0.62f);
    bo_ui_box(box, BO_C_BLACK);
    BORect inner = bo_rect(box.x + 8, box.y + 8, box.w - 16, box.h - 16);
    bo_ui_box(inner, BO_C_DARK);
    char text[64], value[16];
    snprintf(text, sizeof(text), "Cash Out: $%d", run->eval.total);
    if (bo_ui_button(BO_ID_CASH_OUT, bo_rect(inner.x + 12, inner.y + 12, inner.w - 24, G.height * 0.1f), BO_C_ORANGE,
                     text, 1)) {
        bo_run_cash_out(run);
        G.shownRound = 0;
        return;
    }
    float y = inner.y + G.height * 0.15f, rowH = G.height * 0.07f, size = G.height * 0.045f;
    for (int r = 0; r < run->eval.row_count; r++) {
        const BOEvalRow *row = &run->eval.rows[r];
        if (row->kind == BO_EVAL_BLIND) {
            const BOBlindInfo *info = bo_blind_info(run->blind);
            draw_blind_chip(run->blind, inner.x + 16, y, rowH);
            bo_font_draw(info->name, inner.x + 24 + rowH, y + rowH * 0.2f, size, bo_rgb(BO_C_WHITE), BO_ALIGN_LEFT);
            snprintf(value, sizeof(value), "%.*s", row->dollars, "$$$$$$$$$$");
            bo_font_draw(value, inner.x + inner.w - 16, y + rowH * 0.2f, size, bo_rgb(BO_C_MONEY), BO_ALIGN_RIGHT);
            y += rowH + 10;
            bo_gfx_rect(inner.x + 16, y, inner.w - 32, 2, bo_rgb(BO_C_L_BLACK));
            y += 14;
            continue;
        }
        const char *label = "";
        unsigned int color = BO_C_WHITE;
        char count[16] = "";
        if (row->kind == BO_EVAL_HANDS) {
            snprintf(count, sizeof(count), "%d", row->count);
            label = "Remaining Hands ($1 each)";
            color = BO_C_BLUE;
        } else if (row->kind == BO_EVAL_DISCARDS) {
            snprintf(count, sizeof(count), "%d", row->count);
            label = "Remaining Discards ($1 each)";
            color = BO_C_RED;
        } else if (row->kind == BO_EVAL_INTEREST) {
            snprintf(count, sizeof(count), "%d", row->count);
            label = "Interest ($1 per $5)";
            color = BO_C_MONEY;
        } else if (row->kind == BO_EVAL_JOKER) {
            label = bo_center(run->jokers[row->index].center)->name;
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
    BORect box = bo_rect(area.x + area.w * 0.12f, G.height * 0.14f, area.w * 0.76f, G.height * 0.66f);
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
            new_run();
        }
    } else if (bo_ui_button(BO_ID_NEW_RUN, bo_rect(inner.x + (inner.w - bw) / 2, by, bw, bh), BO_C_BLUE, "New Run",
                            1)) {
        new_run();
    }
}

static void draw_popups(double dt) {
    for (int i = 0; i < 16; i++) {
        BOPopup *p = &G.popups[i];
        if (p->age <= 0 || p->age > 1.2f) continue;
        p->age += (float)dt;
        float alpha = p->age < 0.9f ? 1 : (1.2f - p->age) / 0.3f;
        float size = G.height * 0.07f;
        bo_font_draw(p->text, p->x + 2, p->y - p->age * 20 + 2, size, bo_rgba(0x000000, alpha * 0.4f),
                     BO_ALIGN_CENTER);
        bo_font_draw(p->text, p->x, p->y - p->age * 20, size, bo_rgba(p->color, alpha), BO_ALIGN_CENTER);
    }
}

static void approach(float *value, float target, double dt) {
    *value += (target - *value) * (float)(1 - exp(-dt * 12));
}

// Counts a shown number up to its value in about half a second.
static void count_to(double *shown, double value, double dt) {
    double step = (value - *shown) * (1 - exp(-dt * 8));
    if (fabs(value - *shown) < 1) {
        *shown = value;
    } else {
        *shown += step;
    }
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
    BOCardView *areas[2] = {G.hand, G.play};
    int counts[2] = {G.handCount, G.playCount};
    for (int a = 0; a < 2; a++) {
        for (int i = 0; i < counts[a]; i++) {
            approach(&areas[a][i].x, areas[a][i].tx, dt);
            approach(&areas[a][i].y, areas[a][i].ty, dt);
            areas[a][i].pulse = fmaxf(0, areas[a][i].pulse - (float)dt * 4);
        }
    }
    // The round score counts up after the hand scores, as in Balatro.
    if (G.flow != BO_FLOW_SCORE) count_to(&G.shownRound, G.run->chips, dt);
    count_to(&G.shownDollars, G.run->dollars, dt);

    bo_ui_begin();
    bo_gfx_begin(G.width, G.height, bo_rgb(0x3a5c4f));
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
    } else if (state == BO_STATE_GAME_OVER) {
        draw_end(0);
    }
    draw_popups(dt);
    bo_gfx_flush();
    bo_ui_end();
}

void bo_game_debug(const char *line) {
    double value;
    if (sscanf(line, "target %lf", &value) == 1) {
        G.run->blind_chips = value;
        fprintf(stderr, "debug: target %.0f\n", value);
    } else if (sscanf(line, "ante %lf", &value) == 1) {
        G.run->ante = (int)value;
        fprintf(stderr, "debug: ante %.0f\n", value);
    } else if (sscanf(line, "dollars %lf", &value) == 1) {
        G.run->dollars = (int)value;
        fprintf(stderr, "debug: dollars %.0f\n", value);
    } else {
        fprintf(stderr, "debug: unknown command: %s", line);
    }
}

void bo_game_touch(int phase, float x, float y) {
    if (phase == BO_TOUCH_BEGAN) bo_ui_tap(x, y);
}
