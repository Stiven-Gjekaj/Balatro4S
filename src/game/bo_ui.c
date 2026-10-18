#include "bo_ui.h"

#include <string.h>

#include "bo_font.h"

#define BO_UI_MAX_AREAS 128

static struct {
    int ids[BO_UI_MAX_AREAS];
    BORect rects[BO_UI_MAX_AREAS];
    int count;
    // The taps that wait. Each frame takes one, so that two quick taps both
    // reach their widgets.
    int taps;
    float tapX[8], tapY[8];
    int hitId;      // The id that the tap hit at the end of the last frame.
    int clickedId;  // The id that the widgets read in this frame.
} U = {.hitId = -1, .clickedId = -1};

BORect bo_rect(float x, float y, float w, float h) {
    BORect rect = {x, y, w, h};
    return rect;
}

int bo_rect_contains(BORect r, float x, float y) {
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

void bo_ui_begin(void) {
    U.clickedId = U.hitId;
    U.hitId = -1;
    U.count = 0;
}

void bo_ui_end(void) {
    if (U.taps == 0) return;
    float x = U.tapX[0], y = U.tapY[0];
    U.taps--;
    memmove(U.tapX, U.tapX + 1, sizeof(float) * U.taps);
    memmove(U.tapY, U.tapY + 1, sizeof(float) * U.taps);
    for (int i = U.count - 1; i >= 0; i--) {
        if (bo_rect_contains(U.rects[i], x, y)) {
            U.hitId = U.ids[i];
            return;
        }
    }
}

void bo_ui_tap(float x, float y) {
    if (U.taps == 8) return;
    U.tapX[U.taps] = x;
    U.tapY[U.taps] = y;
    U.taps++;
}

void bo_ui_area(int id, BORect rect) {
    if (U.count == BO_UI_MAX_AREAS) return;
    U.ids[U.count] = id;
    U.rects[U.count] = rect;
    U.count++;
}

int bo_ui_clicked(int id) {
    if (U.clickedId != id) return 0;
    U.clickedId = -1;
    return 1;
}

int bo_ui_button(int id, BORect r, unsigned int color, const char *label, int enabled) {
    float radius = r.h * 0.18f;
    bo_gfx_round_rect(r.x, r.y + r.h * 0.08f, r.w, r.h, radius, bo_rgba(0x000000, 0.3f));
    bo_gfx_round_rect(r.x, r.y, r.w, r.h, radius, enabled ? bo_rgb(color) : bo_rgb(0x5f7377));
    float size = r.h * 0.55f;
    float width = bo_font_width(label, size);
    if (width > r.w * 0.9f) size *= r.w * 0.9f / width;
    bo_font_draw(label, r.x + r.w / 2, r.y + (r.h - size) / 2, size, bo_rgb(0xffffff), BO_ALIGN_CENTER);
    if (!enabled) return 0;
    bo_ui_area(id, r);
    return bo_ui_clicked(id);
}

void bo_ui_box(BORect r, unsigned int color) {
    bo_gfx_round_rect(r.x, r.y, r.w, r.h, r.h < 40 ? r.h * 0.25f : 10, bo_rgb(color));
}
