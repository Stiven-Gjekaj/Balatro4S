// bo_gfx: a sprite batch for OpenGL ES 2.0.
// The coordinates are pixels of the screen, with (0, 0) at the top left.
// Each sprite is a rectangle of a texture, with a tint and a rotation.

#ifndef BO_GFX_H
#define BO_GFX_H

#include <stddef.h>

typedef struct {
    unsigned int id;  // The name of the GL texture.
    int width;        // The size of the texture in texels.
    int height;
} BOTexture;

typedef struct {
    float r, g, b, a;
} BOColor;

// Makes the shader and the buffers. Call it with a current GL context.
// Returns 0 when the shader does not compile.
int bo_gfx_init(void);

// Starts a frame with the size of the drawable in pixels, and clears it.
void bo_gfx_begin(int width, int height, BOColor clear);

// Draws the texels (sx, sy, sw, sh) of texture into the rectangle
// (x, y, w, h). The rectangle turns by angle radians around its centre.
void bo_gfx_sprite(const BOTexture *texture, float sx, float sy, float sw, float sh,
                   float x, float y, float w, float h, float angle, BOColor tint);

// Fills a rectangle with one colour.
void bo_gfx_rect(float x, float y, float w, float h, BOColor color);

// Fills a rectangle with round corners of radius pixels.
void bo_gfx_round_rect(float x, float y, float w, float h, float radius, BOColor color);

// Sends the sprites that wait in the batch to the GPU.
void bo_gfx_flush(void);

// The number of draw calls in the last frame, for the speed log.
int bo_gfx_draw_calls(void);

// Makes a texture from RGBA pixels with premultiplied alpha.
BOTexture bo_gfx_texture(const unsigned char *rgba, int width, int height);

// The shaders of the editions and the shines, as in Balatro.
typedef enum {
    BO_FX_FOIL,
    BO_FX_HOLO,
    BO_FX_POLYCHROME,
    BO_FX_NEGATIVE,
    BO_FX_NEGATIVE_SHINE,
    BO_FX_VOUCHER,
    BO_FX_BOOSTER,
    BO_FX_BACKGROUND,
    BO_FX_COUNT,
} BOShaderEffect;

// Compiles the fragment shader of an effect from the LOVE source of
// Balatro.love. name is the name of its vec2 uniform, as "foil". Returns 0
// when it does not compile; the effect then draws nothing.
int bo_gfx_load_effect(BOShaderEffect effect, const char *name, const char *source);

// Draws one sprite with an effect: the cell (col, row) of an atlas of
// columns x rows, into (x, y, w, h). value is the vec2 of the effect, and
// time is the time of the game in seconds. The A5 cannot run these shaders
// on each card in each frame, so each card with an effect goes into a cache
// texture, and bo_gfx_begin draws one card of the cache again each frame.
void bo_gfx_effect_sprite(BOShaderEffect effect, const BOTexture *texture, int columns, int rows, int col, int row,
                          float x, float y, float w, float h, float value1, float value2, float time);

// The moving background of Balatro (BO_FX_BACKGROUND), at a low
// resolution, over the whole screen. Call it before bo_gfx_begin, which
// draws it first.
void bo_gfx_set_background(float time, float spin_time, BOColor colour1, BOColor colour2, BOColor colour3,
                           float contrast, float spin_amount);

BOColor bo_rgb(unsigned int hex);
BOColor bo_rgba(unsigned int hex, float alpha);

#endif
