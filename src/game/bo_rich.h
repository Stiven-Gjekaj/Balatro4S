// bo_rich: draws a line of text with the markup of Balatro, as
// "{C:mult}+4{} Mult" or "{X:mult,C:white} X2 {} Mult".
// C sets the colour of the text, X sets a box behind it, and s sets the
// size. The other controls do nothing here.

#ifndef BO_RICH_H
#define BO_RICH_H

#include "bo_gfx.h"

// The colour of a name of loc_colour, as "mult" or "attention".
BOColor bo_rich_colour(const char *name, BOColor fallback);

// The width of a line in pixels.
float bo_rich_width(const char *line, float height);

// Draws a line. align is a BO_ALIGN_* of bo_font.h. Returns the width.
float bo_rich_draw(const char *line, float x, float y, float height, BOColor fallback, int align);

#endif
