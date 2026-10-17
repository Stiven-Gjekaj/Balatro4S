#include "bo_game.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bo_card.h"
#include "bo_debug.h"
#include "bo_font.h"
#include "bo_gfx.h"
#include "bo_hand.h"
#include "bo_image.h"
#include "bo_score.h"
#include "bo_zip.h"

#define BO_HAND_SIZE 8
#define BO_DECK_SIZE 52

// The width of one card in the 2x atlases of Balatro.
static const float kAtlasCardW = 142;

// The colours of Balatro, from globals.lua.
#define BO_C_BLUE 0x009dff
#define BO_C_RED 0xfe5f55
#define BO_C_ORANGE 0xfda200
#define BO_C_BLACK 0x374244
#define BO_C_L_BLACK 0x4f6367
#define BO_C_DARK 0x1e2b2d
#define BO_C_MONEY 0xf3b958

typedef struct {
    BOCard card;
    float x, y;    // The place on the screen now.
    float tx, ty;  // The place that the card moves to.
    float pulse;   // A short grow when the card scores, from 1 down to 0.
    int selected;
} BOCardView;

typedef struct {
    char text[16];
    float x, y;
    float age;
    unsigned int color;
} BOPopup;

typedef enum {
    BO_PHASE_SELECT,  // The player selects cards.
    BO_PHASE_SCORE,   // The played cards score, one event at a time.
    BO_PHASE_DRAW,    // The used cards leave, and new cards come.
} BOPhase;

typedef struct {
    float x, y, w, h;
    unsigned int color;
    const char *label;
} BOButton;

static struct {
    char error[256];
    int width, height;
    float cardW, cardH;
    BOTexture deck;
    BOTexture centers;

    BOCard pile[BO_DECK_SIZE];
    int pileCount;
    BOCardView hand[BO_HAND_SIZE];
    int handCount;
    BOCardView play[BO_MAX_PLAY];
    int playCount;

    BOScoreContext context;
    BOScore score;
    int nextEvent;
    double shownChips, shownMult;
    const char *shownHand;
    double roundScore;
    int hands, discards;

    BOPhase phase;
    double wait;
    BOPopup popups[16];
    BOButton playButton, discardButton;

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

static void shuffle(BOCard *cards, int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        BOCard t = cards[i];
        cards[i] = cards[j];
        cards[j] = t;
    }
}

// The areas of the screen, in pixels of a 960 x 640 screen.
static float panel_width(void) {
    return G.width * 0.25f;
}

static float hand_top(void) {
    return G.height * 0.60f;
}

static void layout_row(BOCardView *cards, int count, float areaX, float areaW, float y, float lift) {
    float step = count > 1 ? (areaW - G.cardW) / (count - 1) : 0;
    if (step > G.cardW * 1.05f) step = G.cardW * 1.05f;
    float total = step * (count - 1) + G.cardW;
    float startX = areaX + (areaW - total) / 2;
    for (int i = 0; i < count; i++) {
        cards[i].tx = startX + step * i;
        cards[i].ty = y - (cards[i].selected ? lift : 0);
    }
}

static float deck_x(void) {
    return G.width - G.cardW - G.width * 0.02f;
}

static void layout(void) {
    float left = panel_width() + G.width * 0.02f;
    float right = deck_x() - G.width * 0.02f;
    layout_row(G.hand, G.handCount, left, right - left, hand_top(), G.cardH * 0.2f);
    layout_row(G.play, G.playCount, left, right - left, G.height * 0.30f, G.cardH * 0.12f);
}

static void draw_cards(void) {
    while (G.handCount < BO_HAND_SIZE && G.pileCount > 0) {
        BOCardView *slot = &G.hand[G.handCount++];
        memset(slot, 0, sizeof(*slot));
        slot->card = G.pile[--G.pileCount];
        slot->x = deck_x();
        slot->y = hand_top();
    }
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

    size_t fontSize = 0;
    unsigned char *font = bo_zip_read(zip, "resources/fonts/m6x11plus.ttf", &fontSize);
    int fontLoaded = font && bo_font_load(font, fontSize, 48);
    free(font);
    bo_zip_close(zip);
    if (!G.deck.id || !G.centers.id || !fontLoaded) {
        fail("Balatro.love does not have the textures or the font that Balatro4S needs.");
        return 0;
    }

    srand((unsigned)time(NULL));
    for (int suit = 0; suit < BO_SUIT_COUNT; suit++) {
        for (int rank = 2; rank <= BO_ACE; rank++) {
            BOCard card = bo_card_make(rank, (BOSuit)suit);
            card.uid = (uint32_t)G.pileCount;
            G.pile[G.pileCount++] = card;
        }
    }
    shuffle(G.pile, G.pileCount);
    bo_score_context_init(&G.context);
    G.hands = 4;
    G.discards = 3;

    float buttonW = G.width * 0.15f, buttonH = G.height * 0.09f;
    float centre = (panel_width() + deck_x()) / 2;
    float buttonY = G.height - buttonH - G.height * 0.03f;
    G.playButton = (BOButton){centre - buttonW - G.width * 0.01f, buttonY, buttonW, buttonH, BO_C_BLUE, "Play Hand"};
    G.discardButton = (BOButton){centre + G.width * 0.01f, buttonY, buttonW, buttonH, BO_C_RED, "Discard"};
    draw_cards();
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
    // A soft shadow under the card.
    bo_gfx_round_rect(x + G.cardW * 0.04f, y + G.cardH * 0.04f, w, h, G.cardW * 0.06f, bo_rgba(0x000000, 0.25f));
    // The front of a plain card is at (1, 0) in Enhancers.png.
    bo_gfx_sprite(&G.centers, cw * 1, 0, cw, ch, x, y, w, h, 0, bo_rgb(0xffffff));
    float dw = G.deck.width / 13.0f, dh = G.deck.height / 4.0f;
    bo_gfx_sprite(&G.deck, dw * (view->card.rank - 2), dh * deck_row(view->card.suit), dw, dh, x, y, w, h, 0,
                  bo_rgb(0xffffff));
}

static void draw_deck(void) {
    float cw = G.centers.width / 7.0f, ch = G.centers.height / 5.0f;
    float x = deck_x(), y = hand_top();
    // The Red Deck back is at (0, 0) in Enhancers.png. A few cards under it
    // show the height of the pile.
    int layers = G.pileCount > 0 ? 1 + G.pileCount / 13 : 0;
    for (int i = layers - 1; i >= 0; i--) {
        bo_gfx_sprite(&G.centers, 0, 0, cw, ch, x - i * 2, y - i * 2, G.cardW, G.cardH, 0, bo_rgb(0xffffff));
    }
    char text[16];
    snprintf(text, sizeof(text), "%d/%d", G.pileCount, BO_DECK_SIZE);
    bo_font_draw(text, x + G.cardW / 2, y + G.cardH + G.height * 0.01f, G.height * 0.04f, bo_rgb(0xffffff),
                 BO_ALIGN_CENTER);
}

static int selected_cards(BOCard *out) {
    int count = 0;
    for (int i = 0; i < G.handCount; i++) {
        if (G.hand[i].selected) out[count++] = G.hand[i].card;
    }
    return count;
}

static void format_number(double value, char *text, size_t size) {
    snprintf(text, size, "%.0f", value);
}

static void draw_counter(const char *label, int value, float x, float y, float w, unsigned int color) {
    float h = G.height * 0.10f;
    bo_gfx_round_rect(x, y, w, h, G.width * 0.008f, bo_rgb(BO_C_DARK));
    bo_font_draw(label, x + w / 2, y + h * 0.08f, h * 0.32f, bo_rgb(0xffffff), BO_ALIGN_CENTER);
    char text[16];
    snprintf(text, sizeof(text), "%d", value);
    bo_gfx_round_rect(x + w * 0.1f, y + h * 0.45f, w * 0.8f, h * 0.48f, G.width * 0.006f, bo_rgb(BO_C_L_BLACK));
    bo_font_draw(text, x + w / 2, y + h * 0.48f, h * 0.45f, bo_rgb(color), BO_ALIGN_CENTER);
}

static void draw_panel(void) {
    float panelW = panel_width(), pad = G.width * 0.012f;
    bo_gfx_rect(0, 0, panelW, G.height, bo_rgb(BO_C_BLACK));
    float inner = panelW - pad * 2;

    // The round score.
    float y = G.height * 0.22f, h = G.height * 0.10f;
    bo_gfx_round_rect(pad, y, inner, h, pad, bo_rgb(BO_C_DARK));
    bo_font_draw("Round", pad * 2, y + h * 0.12f, h * 0.32f, bo_rgb(0xffffff), BO_ALIGN_LEFT);
    bo_font_draw("score", pad * 2, y + h * 0.48f, h * 0.32f, bo_rgb(0xffffff), BO_ALIGN_LEFT);
    char text[32];
    format_number(G.roundScore, text, sizeof(text));
    bo_gfx_round_rect(pad + inner * 0.38f, y + h * 0.12f, inner * 0.58f, h * 0.76f, pad * 0.6f, bo_rgb(BO_C_L_BLACK));
    bo_font_draw(text, pad + inner * 0.67f, y + h * 0.2f, h * 0.6f, bo_rgb(0xffffff), BO_ALIGN_CENTER);

    // The hand and its chips and mult.
    y = G.height * 0.34f;
    h = G.height * 0.19f;
    bo_gfx_round_rect(pad, y, inner, h, pad, bo_rgb(BO_C_DARK));
    if (G.shownHand) {
        bo_font_draw(G.shownHand, panelW / 2, y + h * 0.06f, h * 0.26f, bo_rgb(0xffffff), BO_ALIGN_CENTER);
    }
    float half = (inner - pad * 3) / 2;
    float rowY = y + h * 0.42f, rowH = h * 0.48f;
    bo_gfx_round_rect(pad * 2, rowY, half, rowH, pad * 0.6f, bo_rgb(BO_C_BLUE));
    bo_gfx_round_rect(pad * 3 + half, rowY, half, rowH, pad * 0.6f, bo_rgb(BO_C_RED));
    if (G.shownHand) {
        format_number(G.shownChips, text, sizeof(text));
        bo_font_draw(text, pad * 2 + half - pad * 0.6f, rowY + rowH * 0.12f, rowH * 0.8f, bo_rgb(0xffffff),
                     BO_ALIGN_RIGHT);
        format_number(G.shownMult, text, sizeof(text));
        bo_font_draw(text, pad * 3 + half + pad * 0.6f, rowY + rowH * 0.12f, rowH * 0.8f, bo_rgb(0xffffff),
                     BO_ALIGN_LEFT);
    }
    bo_font_draw("X", panelW / 2, rowY + rowH * 0.2f, rowH * 0.6f, bo_rgb(BO_C_RED), BO_ALIGN_CENTER);

    // The hands and the discards that are left.
    y = G.height * 0.56f;
    draw_counter("Hands", G.hands, pad, y, (inner - pad) / 2, BO_C_BLUE);
    draw_counter("Discards", G.discards, pad * 2 + (inner - pad) / 2, y, (inner - pad) / 2, BO_C_RED);
}

static int button_enabled(const BOButton *button) {
    BOCard played[BO_MAX_PLAY];
    if (G.phase != BO_PHASE_SELECT || selected_cards(played) == 0) return 0;
    if (button == &G.discardButton) return G.discards > 0;
    return G.hands > 0;
}

static void draw_button(const BOButton *button) {
    BOColor color = button_enabled(button) ? bo_rgb(button->color) : bo_rgb(0x5f7377);
    float r = button->h * 0.18f;
    bo_gfx_round_rect(button->x, button->y + button->h * 0.08f, button->w, button->h, r, bo_rgba(0x000000, 0.3f));
    bo_gfx_round_rect(button->x, button->y, button->w, button->h, r, color);
    bo_font_draw(button->label, button->x + button->w / 2, button->y + button->h * 0.22f, button->h * 0.55f,
                 bo_rgb(0xffffff), BO_ALIGN_CENTER);
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

static int hit(const BOButton *b, float x, float y) {
    return x >= b->x && x < b->x + b->w && y >= b->y && y < b->y + b->h;
}

// Moves the selected cards out of the hand into dest, in their order.
static int take_selected(BOCardView *dest) {
    int count = 0, kept = 0;
    for (int i = 0; i < G.handCount; i++) {
        if (G.hand[i].selected) {
            dest[count] = G.hand[i];
            dest[count].selected = 0;
            count++;
        } else {
            G.hand[kept++] = G.hand[i];
        }
    }
    G.handCount = kept;
    return count;
}

static void play_hand(void) {
    G.hands--;
    G.playCount = take_selected(G.play);
    BOCard played[BO_MAX_PLAY], held[BO_HAND_SIZE];
    for (int i = 0; i < G.playCount; i++) played[i] = G.play[i].card;
    for (int i = 0; i < G.handCount; i++) held[i] = G.hand[i].card;
    bo_score_play(&G.context, played, G.playCount, held, G.handCount, &G.score);
    G.shownHand = bo_hand_name(G.score.type, G.score.royal);
    G.shownChips = G.score.base_chips;
    G.shownMult = G.score.base_mult;
    // Balatro lifts the scoring cards of the played hand.
    for (int i = 0; i < G.playCount; i++) G.play[i].selected = (G.score.scoring >> i) & 1;
    layout();
    G.nextEvent = 0;
    G.phase = BO_PHASE_SCORE;
    G.wait = 0.8;
    fprintf(stderr, "play: %s, %d events, total %.0f\n", G.shownHand, G.score.event_count, G.score.total);
}

static void discard_hand(void) {
    G.discards--;
    G.playCount = take_selected(G.play);
    for (int i = 0; i < G.playCount; i++) {
        G.play[i].tx = G.width + G.cardW;
        G.play[i].ty = G.play[i].y;
    }
    G.phase = BO_PHASE_DRAW;
    G.wait = 0.4;
}

static void update_flow(double dt) {
    G.wait -= dt;
    if (G.wait > 0) return;
    if (G.phase == BO_PHASE_SCORE) {
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
            G.wait = 0.45;
        } else {
            G.roundScore += G.score.total;
            G.shownHand = NULL;
            for (int i = 0; i < G.playCount; i++) {
                G.play[i].tx = G.width + G.cardW;
                G.play[i].selected = 0;
            }
            G.phase = BO_PHASE_DRAW;
            G.wait = 0.5;
        }
    } else if (G.phase == BO_PHASE_DRAW) {
        G.playCount = 0;
        draw_cards();
        G.phase = BO_PHASE_SELECT;
    }
}

static void approach(float *value, float target, double dt) {
    *value += (target - *value) * (float)(1 - exp(-dt * 12));
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

    bo_gfx_begin(G.width, G.height, bo_rgb(0x3a5c4f));
    draw_panel();
    draw_deck();
    for (int i = 0; i < G.handCount; i++) draw_card(&G.hand[i]);
    for (int i = 0; i < G.playCount; i++) draw_card(&G.play[i]);
    draw_button(&G.playButton);
    draw_button(&G.discardButton);
    draw_popups(dt);
    bo_gfx_flush();
}

void bo_game_touch(int phase, float x, float y) {
    if (phase != BO_TOUCH_BEGAN || G.phase != BO_PHASE_SELECT) return;
    if (hit(&G.playButton, x, y)) {
        if (button_enabled(&G.playButton)) play_hand();
        return;
    }
    if (hit(&G.discardButton, x, y)) {
        if (button_enabled(&G.discardButton)) discard_hand();
        return;
    }
    // The card on top is the last one drawn, so test from the end.
    for (int i = G.handCount - 1; i >= 0; i--) {
        BOCardView *c = &G.hand[i];
        if (x >= c->tx && x < c->tx + G.cardW && y >= c->ty && y < c->ty + G.cardH) {
            BOCard played[BO_MAX_PLAY];
            if (!c->selected && selected_cards(played) >= BO_MAX_PLAY) return;
            c->selected = !c->selected;
            int count = selected_cards(played);
            if (count) {
                BOHandResult result = bo_hand_evaluate(played, count, &G.context.rules);
                G.shownHand = bo_hand_name(result.type, result.royal);
                G.shownChips = bo_hand_level_chips(result.type, G.context.levels[result.type]);
                G.shownMult = bo_hand_level_mult(result.type, G.context.levels[result.type]);
            } else {
                G.shownHand = NULL;
            }
            layout();
            return;
        }
    }
}
