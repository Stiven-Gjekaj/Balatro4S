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
#include "bo_zip.h"

#define BO_HAND_SIZE 8

// The width of one card in the 2x atlases of Balatro.
static const float kAtlasCardW = 142;

typedef struct {
    BOCard card;
    float x, y;          // The place on the screen now.
    float tx, ty;        // The place that the card moves to.
    int selected;
} BOHandCard;

static struct {
    char error[256];
    int width, height;
    float cardW, cardH;  // The size of a card on the screen.
    BOTexture deck;      // 8BitDeck: the ranks and the suits.
    BOTexture centers;   // Enhancers: the card fronts and the backs.
    BOCard pile[52];
    int pileCount;
    BOHandCard hand[BO_HAND_SIZE];
    int handCount;
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

static void layout_hand(void) {
    float areaX = G.width * 0.27f, areaW = G.width * 0.70f;
    float step = G.handCount > 1 ? (areaW - G.cardW) / (G.handCount - 1) : 0;
    if (step > G.cardW * 1.05f) step = G.cardW * 1.05f;
    float total = step * (G.handCount - 1) + G.cardW;
    float startX = areaX + (areaW - total) / 2;
    float baseY = G.height - G.cardH - G.height * 0.06f;
    for (int i = 0; i < G.handCount; i++) {
        G.hand[i].tx = startX + step * i;
        G.hand[i].ty = baseY - (G.hand[i].selected ? G.cardH * 0.2f : 0);
    }
}

static void deal(void) {
    while (G.handCount < BO_HAND_SIZE && G.pileCount > 0) {
        BOHandCard *slot = &G.hand[G.handCount++];
        memset(slot, 0, sizeof(*slot));
        slot->card = G.pile[--G.pileCount];
        slot->x = G.width + G.cardW;
        slot->y = G.height - G.cardH * 1.2f;
    }
    layout_hand();
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
    if (font) bo_font_load(font, fontSize, 48);
    free(font);
    bo_zip_close(zip);
    if (!G.deck.id || !G.centers.id || !font) {
        fail("Balatro.love does not have the textures or the font that Balatro4S needs.");
        return 0;
    }
    fprintf(stderr, "card %.0fx%.0f, deck texture %dx%d\n", G.cardW, G.cardH, G.deck.width, G.deck.height);

    srand((unsigned)time(NULL));
    for (int suit = 0; suit < BO_SUIT_COUNT; suit++) {
        for (int rank = 2; rank <= BO_ACE; rank++) {
            BOCard card = bo_card_make(rank, (BOSuit)suit);
            card.uid = (uint32_t)G.pileCount;
            G.pile[G.pileCount++] = card;
        }
    }
    shuffle(G.pile, G.pileCount);
    deal();
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

static void draw_card(const BOCard *card, float x, float y) {
    float cw = G.centers.width / 7.0f, ch = G.centers.height / 5.0f;
    // The front of a plain card is at (1, 0) in Enhancers.png.
    bo_gfx_sprite(&G.centers, cw * 1, 0, cw, ch, x, y, G.cardW, G.cardH, 0, bo_rgb(0xffffff));
    float dw = G.deck.width / 13.0f, dh = G.deck.height / 4.0f;
    bo_gfx_sprite(&G.deck, dw * (card->rank - 2), dh * deck_row(card->suit), dw, dh, x, y, G.cardW, G.cardH, 0,
                  bo_rgb(0xffffff));
}

static int selected_cards(BOCard *out) {
    int count = 0;
    for (int i = 0; i < G.handCount; i++) {
        if (G.hand[i].selected) out[count++] = G.hand[i].card;
    }
    return count;
}

static void draw_panel(void) {
    float panelW = G.width * 0.25f;
    bo_gfx_rect(0, 0, panelW, G.height, bo_rgb(0x374244));
    BOCard played[BO_MAX_PLAY];
    int count = selected_cards(played);
    float pad = G.width * 0.015f;
    float boxY = G.height * 0.42f, boxH = G.height * 0.2f;
    bo_gfx_rect(pad, boxY, panelW - pad * 2, boxH, bo_rgb(0x1e2b2d));
    if (count == 0) return;

    BOHandResult result = bo_hand_evaluate(played, count, NULL);
    const BOHandBase *base = bo_hand_base(result.type);
    bo_font_draw(bo_hand_name(result.type, result.royal), panelW / 2, boxY + pad, G.height * 0.05f,
                 bo_rgb(0xffffff), BO_ALIGN_CENTER);
    float half = (panelW - pad * 3) / 2;
    float rowY = boxY + boxH * 0.45f, rowH = boxH * 0.42f;
    bo_gfx_rect(pad, rowY, half - pad / 2, rowH, bo_rgb(0x009dff));
    bo_gfx_rect(pad * 2 + half, rowY, half - pad / 2, rowH, bo_rgb(0xfe5f55));
    char text[32];
    snprintf(text, sizeof(text), "%d", base->chips);
    bo_font_draw(text, pad + half - pad, rowY + rowH * 0.15f, rowH * 0.75f, bo_rgb(0xffffff), BO_ALIGN_RIGHT);
    snprintf(text, sizeof(text), "%d", base->mult);
    bo_font_draw(text, pad * 2 + half + pad / 2, rowY + rowH * 0.15f, rowH * 0.75f, bo_rgb(0xffffff),
                 BO_ALIGN_LEFT);
    bo_font_draw("X", panelW / 2, rowY + rowH * 0.2f, rowH * 0.6f, bo_rgb(0xfe5f55), BO_ALIGN_CENTER);
}

static void approach(float *value, float target, double dt) {
    *value += (target - *value) * (float)(1 - exp(-dt * 14));
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
    for (int i = 0; i < G.handCount; i++) {
        approach(&G.hand[i].x, G.hand[i].tx, dt);
        approach(&G.hand[i].y, G.hand[i].ty, dt);
    }

    bo_gfx_begin(G.width, G.height, bo_rgb(0x3a5c4f));
    draw_panel();
    for (int i = 0; i < G.handCount; i++) draw_card(&G.hand[i].card, G.hand[i].x, G.hand[i].y);
    bo_gfx_flush();
}

void bo_game_touch(int phase, float x, float y) {
    if (phase != BO_TOUCH_BEGAN) return;
    // The card on top is the last one drawn, so test from the end.
    for (int i = G.handCount - 1; i >= 0; i--) {
        BOHandCard *c = &G.hand[i];
        if (x >= c->tx && x < c->tx + G.cardW && y >= c->ty && y < c->ty + G.cardH) {
            BOCard played[BO_MAX_PLAY];
            if (!c->selected && selected_cards(played) >= BO_MAX_PLAY) return;
            c->selected = !c->selected;
            layout_hand();
            return;
        }
    }
}
