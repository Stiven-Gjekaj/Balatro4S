// bo_ui: an immediate mode UI. Each frame, the screen draws its widgets
// again and gives each one an id. A tap goes to the top widget under it.
// The widget reads the tap in the next frame.

#ifndef BO_UI_H
#define BO_UI_H

#include "bo_gfx.h"

typedef struct {
    float x, y, w, h;
} BORect;

// Starts a frame. The tap of the last frame becomes the clicked id.
void bo_ui_begin(void);
// Finds the top widget under the tap that waits, if one waits.
void bo_ui_end(void);

// Keeps a tap for the end of this frame.
void bo_ui_tap(float x, float y);

// Registers a rectangle that a tap can hit.
void bo_ui_area(int id, BORect rect);

// 1 when the widget with this id got the tap of the last frame.
int bo_ui_clicked(int id);

// Draws a button and registers it. Returns 1 when it got a tap. A button
// that is not enabled is grey and gets no tap.
int bo_ui_button(int id, BORect rect, unsigned int color, const char *label, int enabled);

// Draws a dark box with round corners, as the boxes of the side panel.
void bo_ui_box(BORect rect, unsigned int color);

BORect bo_rect(float x, float y, float w, float h);
int bo_rect_contains(BORect rect, float x, float y);

#endif
