#include "bo_gfx.h"

#include <OpenGLES/ES2/gl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// One vertex: the position in pixels, the texture coordinate, and the
// tint with premultiplied alpha as four bytes.
typedef struct {
    float x, y;
    float u, v;
    unsigned char r, g, b, a;
} BOVertex;

#define BO_MAX_SPRITES 2048

static BOVertex gVertices[BO_MAX_SPRITES * 4];
static int gSprites;
static GLuint gProgram;
static GLuint gIndexBuffer;
static GLuint gVertexBuffer;
static GLint gScreenUniform;
static GLuint gCurrentTexture;
static BOTexture gCircle;
static int gDrawCalls;
static int gLastDrawCalls;
static float gScreenW, gScreenH;
static GLint gMainFramebuffer;
static GLuint gVertexShaderObject;

static const char *kVertexShader =
    "uniform vec2 screen;\n"
    "attribute vec2 position;\n"
    "attribute vec2 texcoord;\n"
    "attribute vec4 color;\n"
    "varying vec2 v_texcoord;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "    v_texcoord = texcoord;\n"
    "    v_color = color;\n"
    "    gl_Position = vec4(position.x / screen.x * 2.0 - 1.0, 1.0 - position.y / screen.y * 2.0, 0.0, 1.0);\n"
    "}\n";

static const char *kFragmentShader =
    "uniform sampler2D texture;\n"
    "varying mediump vec2 v_texcoord;\n"
    "varying lowp vec4 v_color;\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(texture, v_texcoord) * v_color;\n"
    "}\n";

static GLuint compile(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "bo_gfx: shader: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

int bo_gfx_init(void) {
    GLuint vertex = compile(GL_VERTEX_SHADER, kVertexShader);
    gVertexShaderObject = vertex;
    GLuint fragment = compile(GL_FRAGMENT_SHADER, kFragmentShader);
    if (!vertex || !fragment) return 0;
    gProgram = glCreateProgram();
    glAttachShader(gProgram, vertex);
    glAttachShader(gProgram, fragment);
    glBindAttribLocation(gProgram, 0, "position");
    glBindAttribLocation(gProgram, 1, "texcoord");
    glBindAttribLocation(gProgram, 2, "color");
    glLinkProgram(gProgram);
    GLint ok = 0;
    glGetProgramiv(gProgram, GL_LINK_STATUS, &ok);
    if (!ok) return 0;
    glUseProgram(gProgram);
    gScreenUniform = glGetUniformLocation(gProgram, "screen");
    glUniform1i(glGetUniformLocation(gProgram, "texture"), 0);

    // Each sprite is two triangles over four vertices.
    static GLushort indices[BO_MAX_SPRITES * 6];
    for (int i = 0; i < BO_MAX_SPRITES; i++) {
        GLushort base = (GLushort)(i * 4);
        GLushort quad[6] = {base, (GLushort)(base + 1), (GLushort)(base + 2),
                            (GLushort)(base + 2), (GLushort)(base + 1), (GLushort)(base + 3)};
        memcpy(&indices[i * 6], quad, sizeof(quad));
    }
    glGenBuffers(1, &gIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glGenBuffers(1, &gVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BOVertex), (void *)offsetof(BOVertex, x));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BOVertex), (void *)offsetof(BOVertex, u));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BOVertex), (void *)offsetof(BOVertex, r));

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // A white disc with a smooth edge, for the round corners. The plain
    // rectangles use its solid centre, so that both share one texture.
    enum { size = 64 };
    static unsigned char disc[size * size * 4];
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = x + 0.5f - size / 2.0f, dy = y + 0.5f - size / 2.0f;
            float edge = size / 2.0f - sqrtf(dx * dx + dy * dy);
            unsigned char a = edge >= 1 ? 255 : edge <= 0 ? 0 : (unsigned char)(edge * 255);
            memset(&disc[(y * size + x) * 4], a, 4);
        }
    }
    gCircle = bo_gfx_texture(disc, size, size);
    return 1;
}

static void refresh_effects(void);
static void render_background(void);
static void push(GLuint texture, float u0, float v0, float u1, float v1,
                 float x, float y, float w, float h, float angle, BOColor tint);
static int gBackReady;
static GLuint gBackTexture;

void bo_gfx_begin(int width, int height, BOColor clear) {
    gLastDrawCalls = gDrawCalls;
    gDrawCalls = 0;
    gScreenW = (float)width;
    gScreenH = (float)height;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &gMainFramebuffer);
    // The passes into other framebuffers come before the clear. A change of
    // framebuffer after the clear makes the GPU store and load the screen.
    refresh_effects();
    render_background();
    glViewport(0, 0, width, height);
    glUniform2f(gScreenUniform, (float)width, (float)height);
    glClearColor(clear.r, clear.g, clear.b, clear.a);
    glClear(GL_COLOR_BUFFER_BIT);
    if (gBackReady) {
        // The small texture over the whole screen. Its rows are upside down.
        BOColor white = {1, 1, 1, 1};
        push(gBackTexture, 0, 1, 1, 0, 0, 0, gScreenW, gScreenH, 0, white);
        gBackReady = 0;
    }
}

void bo_gfx_flush(void) {
    if (gSprites == 0) return;
    // A new buffer each time lets the GPU keep the old one while it draws.
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(BOVertex) * 4 * gSprites), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(sizeof(BOVertex) * 4 * gSprites), gVertices);
    glDrawElements(GL_TRIANGLES, gSprites * 6, GL_UNSIGNED_SHORT, 0);
    gSprites = 0;
    gDrawCalls++;
}

static unsigned char byte(float value) {
    if (value <= 0) return 0;
    if (value >= 1) return 255;
    return (unsigned char)(value * 255.0f + 0.5f);
}

static void push(GLuint texture, float u0, float v0, float u1, float v1,
                 float x, float y, float w, float h, float angle, BOColor tint) {
    if (texture != gCurrentTexture) {
        bo_gfx_flush();
        glBindTexture(GL_TEXTURE_2D, texture);
        gCurrentTexture = texture;
    }
    if (gSprites == BO_MAX_SPRITES) bo_gfx_flush();

    float cx = x + w * 0.5f, cy = y + h * 0.5f;
    float hx = w * 0.5f, hy = h * 0.5f;
    float c = 1, s = 0;
    if (angle != 0) {
        c = cosf(angle);
        s = sinf(angle);
    }
    const float corners[4][4] = {
        {-hx, -hy, u0, v0}, {hx, -hy, u1, v0}, {-hx, hy, u0, v1}, {hx, hy, u1, v1}};
    unsigned char r = byte(tint.r * tint.a), g = byte(tint.g * tint.a), b = byte(tint.b * tint.a), a = byte(tint.a);
    BOVertex *out = &gVertices[gSprites * 4];
    for (int i = 0; i < 4; i++) {
        out[i].x = cx + corners[i][0] * c - corners[i][1] * s;
        out[i].y = cy + corners[i][0] * s + corners[i][1] * c;
        out[i].u = corners[i][2];
        out[i].v = corners[i][3];
        out[i].r = r;
        out[i].g = g;
        out[i].b = b;
        out[i].a = a;
    }
    gSprites++;
}

void bo_gfx_sprite(const BOTexture *texture, float sx, float sy, float sw, float sh,
                   float x, float y, float w, float h, float angle, BOColor tint) {
    if (!texture || !texture->id) return;
    float tw = (float)texture->width, th = (float)texture->height;
    push(texture->id, sx / tw, sy / th, (sx + sw) / tw, (sy + sh) / th, x, y, w, h, angle, tint);
}

void bo_gfx_rect(float x, float y, float w, float h, BOColor color) {
    push(gCircle.id, 0.5f, 0.5f, 0.5f, 0.5f, x, y, w, h, 0, color);
}

void bo_gfx_round_rect(float x, float y, float w, float h, float radius, BOColor color) {
    if (radius * 2 > w) radius = w / 2;
    if (radius * 2 > h) radius = h / 2;
    float r = radius, half = gCircle.width / 2.0f;
    // The four corners are the four quarters of the disc.
    bo_gfx_sprite(&gCircle, 0, 0, half, half, x, y, r, r, 0, color);
    bo_gfx_sprite(&gCircle, half, 0, half, half, x + w - r, y, r, r, 0, color);
    bo_gfx_sprite(&gCircle, 0, half, half, half, x, y + h - r, r, r, 0, color);
    bo_gfx_sprite(&gCircle, half, half, half, half, x + w - r, y + h - r, r, r, 0, color);
    bo_gfx_rect(x + r, y, w - r * 2, h, color);
    bo_gfx_rect(x, y + r, r, h - r * 2, color);
    bo_gfx_rect(x + w - r, y + r, r, h - r * 2, color);
}

int bo_gfx_draw_calls(void) {
    return gLastDrawCalls;
}

BOTexture bo_gfx_texture(const unsigned char *rgba, int width, int height) {
    BOTexture texture = {0, width, height};
    bo_gfx_flush();
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    gCurrentTexture = texture.id;
    // Most atlases are not a power of two, so OpenGL ES 2.0 allows only
    // clamp to edge and no mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    return texture;
}

BOColor bo_rgb(unsigned int hex) {
    return bo_rgba(hex, 1);
}

BOColor bo_rgba(unsigned int hex, float alpha) {
    BOColor color = {((hex >> 16) & 0xff) / 255.0f, ((hex >> 8) & 0xff) / 255.0f, (hex & 0xff) / 255.0f, alpha};
    return color;
}

// --- The effects ------------------------------------------------------------

typedef struct {
    GLuint program;
    GLint screen, value, time, dissolve, textureDetails, imageDetails, shadow, burn1, burn2;
    // The background.
    GLint spinTime, colour1, colour2, colour3, contrast, spinAmount, screenSize;
} BOEffectProgram;

static BOEffectProgram gEffects[BO_FX_COUNT];

// The LOVE shader language over GLSL ES 1.0. Texel gives colours that are
// not premultiplied, as LOVE does; main premultiplies the result again.
static const char *kEffectHeader =
    "precision highp float;\n"
    "#define extern uniform\n"
    "#define number float\n"
    "#define Image sampler2D\n"
    "varying mediump vec2 v_texcoord;\n"
    "varying lowp vec4 v_color;\n"
    "uniform sampler2D bo_texture;\n"
    "uniform vec4 love_ScreenSize;\n"
    "vec4 bo_texel(sampler2D s, vec2 uv) {\n"
    "    vec4 c = texture2D(s, uv);\n"
    "    return c.a > 0.0 ? vec4(c.rgb / c.a, c.a) : vec4(0.0);\n"
    "}\n"
    "#define Texel bo_texel\n";

static const char *kEffectMain =
    "\nvoid main() {\n"
    "    vec4 c = effect(v_color, bo_texture, v_texcoord, gl_FragCoord.xy);\n"
    "    gl_FragColor = vec4(c.rgb * c.a, c.a) * v_color;\n"
    "}\n";

int bo_gfx_load_effect(BOShaderEffect effect, const char *name, const char *source) {
    if (effect < 0 || effect >= BO_FX_COUNT || !source || !gVertexShaderObject) return 0;
    size_t length = strlen(kEffectHeader) + strlen(source) + strlen(kEffectMain) + 1;
    char *text = malloc(length);
    if (!text) return 0;
    snprintf(text, length, "%s%s%s", kEffectHeader, source, kEffectMain);
    GLuint fragment = compile(GL_FRAGMENT_SHADER, text);
    free(text);
    if (!fragment) return 0;
    GLuint program = glCreateProgram();
    glAttachShader(program, gVertexShaderObject);
    glAttachShader(program, fragment);
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "texcoord");
    glBindAttribLocation(program, 2, "color");
    glLinkProgram(program);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "bo_gfx: effect %s: %s\n", name, log);
        return 0;
    }
    BOEffectProgram *e = &gEffects[effect];
    e->program = program;
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "bo_texture"), 0);
    e->screen = glGetUniformLocation(program, "screen");
    e->value = glGetUniformLocation(program, name);
    e->time = glGetUniformLocation(program, "time");
    e->dissolve = glGetUniformLocation(program, "dissolve");
    e->textureDetails = glGetUniformLocation(program, "texture_details");
    e->imageDetails = glGetUniformLocation(program, "image_details");
    e->shadow = glGetUniformLocation(program, "shadow");
    e->burn1 = glGetUniformLocation(program, "burn_colour_1");
    e->burn2 = glGetUniformLocation(program, "burn_colour_2");
    e->spinTime = glGetUniformLocation(program, "spin_time");
    e->colour1 = glGetUniformLocation(program, "colour_1");
    e->colour2 = glGetUniformLocation(program, "colour_2");
    e->colour3 = glGetUniformLocation(program, "colour_3");
    e->contrast = glGetUniformLocation(program, "contrast");
    e->spinAmount = glGetUniformLocation(program, "spin_amount");
    e->screenSize = glGetUniformLocation(program, "love_ScreenSize");
    glUseProgram(gProgram);
    return 1;
}

// The cache: one texture of 1024 x 1024. The top half has small slots for
// the shines over a card, at half size. The bottom half has big slots for
// the Negative cards, at full size, because they replace the card.
enum {
    CACHE_SIZE = 1024,
    SMALL_W = 64, SMALL_H = 80, SMALL_COLUMNS = CACHE_SIZE / SMALL_W, SMALL_SLOTS = SMALL_COLUMNS * (512 / SMALL_H),
    BIG_W = 128, BIG_H = 160, BIG_COLUMNS = CACHE_SIZE / BIG_W, BIG_SLOTS = BIG_COLUMNS * (512 / BIG_H),
    MAX_ENTRIES = SMALL_SLOTS + BIG_SLOTS,
};

typedef struct {
    int used;
    GLuint texture;
    int textureW, textureH, columns, rows, col, row;
    BOShaderEffect effect;
    int big, slot;
    float renderW, renderH;
    float value1, value2, time;
    unsigned lastUsed;     // The frame of the last draw.
    double renderedAt;     // The time of the last render.
} BOCacheEntry;

static GLuint gCacheFramebuffer, gCacheTexture;
static BOCacheEntry gEntries[MAX_ENTRIES];
static unsigned gFrame;
static float gLastTime;

static int cache_ready(void) {
    if (gCacheFramebuffer) return 1;
    unsigned char *empty = calloc(CACHE_SIZE * CACHE_SIZE, 4);
    if (!empty) return 0;
    BOTexture t = bo_gfx_texture(empty, CACHE_SIZE, CACHE_SIZE);
    free(empty);
    gCacheTexture = t.id;
    glGenFramebuffers(1, &gCacheFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gCacheFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gCacheTexture, 0);
    int ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)gMainFramebuffer);
    if (!ok) fprintf(stderr, "bo_gfx: the effect cache has no framebuffer\n");
    return ok;
}

static void slot_origin(const BOCacheEntry *e, float *x, float *y) {
    if (e->big) {
        *x = (float)(e->slot % BIG_COLUMNS) * BIG_W;
        *y = 512.0f + (float)(e->slot / BIG_COLUMNS) * BIG_H;
    } else {
        *x = (float)(e->slot % SMALL_COLUMNS) * SMALL_W;
        *y = (float)(e->slot / SMALL_COLUMNS) * SMALL_H;
    }
}

// Runs the shader of an entry into its slot.
static void render_entry(BOCacheEntry *e) {
    BOEffectProgram *p = &gEffects[e->effect];
    bo_gfx_flush();
    glBindFramebuffer(GL_FRAMEBUFFER, gCacheFramebuffer);
    glViewport(0, 0, CACHE_SIZE, CACHE_SIZE);
    float sx, sy;
    slot_origin(e, &sx, &sy);
    // Clear the slot. GL counts the rows from the bottom of the texture.
    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint)sx, (GLint)(CACHE_SIZE - sy - (e->big ? BIG_H : SMALL_H)), e->big ? BIG_W : SMALL_W,
              e->big ? BIG_H : SMALL_H);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glUseProgram(p->program);
    glUniform2f(p->screen, CACHE_SIZE, CACHE_SIZE);
    glUniform2f(p->value, e->value1, e->value2);
    glUniform1f(p->time, e->time);
    glUniform1f(p->dissolve, 0);
    float cw = e->textureW / (float)e->columns, ch = e->textureH / (float)e->rows;
    // Sprite:get_pos_pixel and get_image_dims: the cell and the size of the
    // atlas, so that the shader knows where the card is.
    glUniform4f(p->textureDetails, (float)e->col, (float)e->row, cw, ch);
    glUniform2f(p->imageDetails, (float)e->textureW, (float)e->textureH);
    glUniform1i(p->shadow, 0);
    glUniform4f(p->burn1, 0, 0, 0, 0);
    glUniform4f(p->burn2, 0, 0, 0, 0);
    BOTexture source = {e->texture, e->textureW, e->textureH};
    BOColor white = {1, 1, 1, 1};
    bo_gfx_sprite(&source, cw * e->col, ch * e->row, cw, ch, sx + 1, sy + 1, e->renderW, e->renderH, 0, white);
    bo_gfx_flush();
    glEnable(GL_BLEND);
    glUseProgram(gProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)gMainFramebuffer);
    glViewport(0, 0, (GLsizei)gScreenW, (GLsizei)gScreenH);
    glUniform2f(gScreenUniform, gScreenW, gScreenH);
    e->renderedAt = e->time;
}

// Each frame, the two cards that waited longest render again.
static void refresh_effects(void) {
    gFrame++;
    for (int n = 0; n < 2; n++) {
        BOCacheEntry *oldest = NULL;
        for (int i = 0; i < MAX_ENTRIES; i++) {
            BOCacheEntry *e = &gEntries[i];
            if (!e->used || e->lastUsed + 1 < gFrame) continue;
            if (!oldest || e->renderedAt < oldest->renderedAt) oldest = e;
        }
        if (!oldest || gLastTime - oldest->renderedAt < 1.0 / 30) return;
        oldest->time = gLastTime;
        render_entry(oldest);
    }
}

static BOCacheEntry *find_entry(BOShaderEffect effect, const BOTexture *texture, int columns, int rows, int col,
                                int row) {
    for (int i = 0; i < MAX_ENTRIES; i++) {
        BOCacheEntry *e = &gEntries[i];
        if (e->used && e->effect == effect && e->texture == texture->id && e->columns == columns &&
            e->rows == rows && e->col == col && e->row == row) {
            return e;
        }
    }
    return NULL;
}

// A free slot, or the slot that waited longest since its last draw.
static BOCacheEntry *new_entry(int big) {
    int slots = big ? BIG_SLOTS : SMALL_SLOTS;
    uint8_t taken[SMALL_SLOTS];
    memset(taken, 0, sizeof(taken));
    BOCacheEntry *free_entry = NULL, *oldest = NULL;
    for (int i = 0; i < MAX_ENTRIES; i++) {
        BOCacheEntry *e = &gEntries[i];
        if (!e->used) {
            if (!free_entry) free_entry = e;
            continue;
        }
        if (e->big != big) continue;
        taken[e->slot] = 1;
        if (!oldest || e->lastUsed < oldest->lastUsed) oldest = e;
    }
    for (int s = 0; s < slots; s++) {
        if (!taken[s] && free_entry) {
            memset(free_entry, 0, sizeof(*free_entry));
            free_entry->used = 1;
            free_entry->big = big;
            free_entry->slot = s;
            return free_entry;
        }
    }
    if (!oldest) return NULL;
    int slot = oldest->slot;
    memset(oldest, 0, sizeof(*oldest));
    oldest->used = 1;
    oldest->big = big;
    oldest->slot = slot;
    return oldest;
}

void bo_gfx_effect_sprite(BOShaderEffect effect, const BOTexture *texture, int columns, int rows, int col, int row,
                          float x, float y, float w, float h, float value1, float value2, float time) {
    if (effect < 0 || effect >= BO_FX_COUNT || !texture || !texture->id || !gEffects[effect].program) return;
    if (!cache_ready()) return;
    gLastTime = time;
    BOCacheEntry *e = find_entry(effect, texture, columns, rows, col, row);
    if (!e) {
        int big = effect == BO_FX_NEGATIVE;
        e = new_entry(big);
        if (!e) return;
        e->effect = effect;
        e->texture = texture->id;
        e->textureW = texture->width;
        e->textureH = texture->height;
        e->columns = columns;
        e->rows = rows;
        e->col = col;
        e->row = row;
        // A shine renders at half size; a Negative card at full size.
        float scale = big ? 1.0f : 0.5f;
        float maxW = (big ? BIG_W : SMALL_W) - 2, maxH = (big ? BIG_H : SMALL_H) - 2;
        e->renderW = w * scale < maxW ? w * scale : maxW;
        e->renderH = h * scale < maxH ? h * scale : maxH;
        e->value1 = value1;
        e->value2 = value2;
        e->time = time;
        render_entry(e);
    }
    e->lastUsed = gFrame;
    e->value1 = value1;
    e->value2 = value2;
    float sx, sy;
    slot_origin(e, &sx, &sy);
    // The slot rows are upside down in the texture: v counts from the bottom.
    float u0 = (sx + 1) / CACHE_SIZE, u1 = (sx + 1 + e->renderW) / CACHE_SIZE;
    float v0 = 1 - (sy + 1) / CACHE_SIZE, v1 = 1 - (sy + 1 + e->renderH) / CACHE_SIZE;
    BOColor white = {1, 1, 1, 1};
    push(gCacheTexture, u0, v0, u1, v1, x, y, w, h, 0, white);
}

// --- The background -----------------------------------------------------------

enum { BACK_W = 120, BACK_H = 80 };

static GLuint gBackFramebuffer;
static struct {
    int set;
    float time, spinTime, contrast, spinAmount;
    BOColor colour1, colour2, colour3;
} gBack;

void bo_gfx_set_background(float time, float spin_time, BOColor colour1, BOColor colour2, BOColor colour3,
                           float contrast, float spin_amount) {
    gBack.set = 1;
    gBack.time = time;
    gBack.spinTime = spin_time;
    gBack.colour1 = colour1;
    gBack.colour2 = colour2;
    gBack.colour3 = colour3;
    gBack.contrast = contrast;
    gBack.spinAmount = spin_amount;
}

// The shader runs on each pixel of the small texture.
static void render_background(void) {
    BOEffectProgram *p = &gEffects[BO_FX_BACKGROUND];
    if (!gBack.set || !p->program) return;
    gBack.set = 0;
    if (!gBackFramebuffer) {
        unsigned char *empty = calloc(BACK_W * BACK_H, 4);
        if (!empty) return;
        BOTexture t = bo_gfx_texture(empty, BACK_W, BACK_H);
        free(empty);
        gBackTexture = t.id;
        glGenFramebuffers(1, &gBackFramebuffer);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, gBackFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBackTexture, 0);
    glViewport(0, 0, BACK_W, BACK_H);
    glDisable(GL_BLEND);
    glUseProgram(p->program);
    glUniform2f(p->screen, BACK_W, BACK_H);
    glUniform4f(p->screenSize, BACK_W, BACK_H, 1, 1);
    glUniform1f(p->time, gBack.time);
    glUniform1f(p->spinTime, gBack.spinTime);
    glUniform4f(p->colour1, gBack.colour1.r, gBack.colour1.g, gBack.colour1.b, gBack.colour1.a);
    glUniform4f(p->colour2, gBack.colour2.r, gBack.colour2.g, gBack.colour2.b, gBack.colour2.a);
    glUniform4f(p->colour3, gBack.colour3.r, gBack.colour3.g, gBack.colour3.b, gBack.colour3.a);
    glUniform1f(p->contrast, gBack.contrast);
    glUniform1f(p->spinAmount, gBack.spinAmount);
    BOColor white = {1, 1, 1, 1};
    push(gCircle.id, 0.5f, 0.5f, 0.5f, 0.5f, 0, 0, BACK_W, BACK_H, 0, white);
    bo_gfx_flush();
    glEnable(GL_BLEND);
    glUseProgram(gProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)gMainFramebuffer);
    gBackReady = 1;
}
