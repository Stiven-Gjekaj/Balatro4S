#include "bo_gfx.h"

#include <OpenGLES/ES2/gl.h>
#include <math.h>
#include <stdio.h>
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
static GLuint gWhite;
static int gDrawCalls;
static int gLastDrawCalls;

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

    // A texture of one white texel, for the plain rectangles.
    unsigned char white[4] = {255, 255, 255, 255};
    gWhite = bo_gfx_texture(white, 1, 1).id;
    return 1;
}

void bo_gfx_begin(int width, int height, BOColor clear) {
    gLastDrawCalls = gDrawCalls;
    gDrawCalls = 0;
    glViewport(0, 0, width, height);
    glUniform2f(gScreenUniform, (float)width, (float)height);
    glClearColor(clear.r, clear.g, clear.b, clear.a);
    glClear(GL_COLOR_BUFFER_BIT);
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
    push(gWhite, 0, 0, 1, 1, x, y, w, h, 0, color);
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
