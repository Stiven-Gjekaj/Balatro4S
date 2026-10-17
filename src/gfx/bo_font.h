// bo_font: draws text with a TrueType font, such as m6x11plus of Balatro.
// It renders the printable ASCII characters into one texture.

#ifndef BO_FONT_H
#define BO_FONT_H

#include <stddef.h>
#include "bo_gfx.h"

enum {
    BO_ALIGN_LEFT = 0,
    BO_ALIGN_CENTER = 1,
    BO_ALIGN_RIGHT = 2,
};

// Renders the font at size pixels. Returns 0 when the bytes are not a font.
int bo_font_load(const unsigned char *ttf, size_t size, float pixels);

// The width of text in pixels when its line is height pixels high.
float bo_font_width(const char *text, float height);

// Draws text with its top at y. The alignment is about x.
// Returns the width in pixels.
float bo_font_draw(const char *text, float x, float y, float height, BOColor color, int align);

#endif
