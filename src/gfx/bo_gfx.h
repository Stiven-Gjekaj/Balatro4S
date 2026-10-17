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

// Sends the sprites that wait in the batch to the GPU.
void bo_gfx_flush(void);

// The number of draw calls in the last frame, for the speed log.
int bo_gfx_draw_calls(void);

// Makes a texture from RGBA pixels with premultiplied alpha.
BOTexture bo_gfx_texture(const unsigned char *rgba, int width, int height);

BOColor bo_rgb(unsigned int hex);
BOColor bo_rgba(unsigned int hex, float alpha);

#endif
