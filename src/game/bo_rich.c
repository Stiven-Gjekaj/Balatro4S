#include "bo_rich.h"

#include <stdlib.h>
#include <string.h>

#include "bo_font.h"

typedef struct {
    const char *name;
    unsigned int hex;
} BONamedColour;

// loc_colour of misc_functions.lua, with the colours of globals.lua.
static const BONamedColour kColours[] = {
    {"red", 0xfe5f55},       {"mult", 0xfe5f55},      {"blue", 0x009dff},     {"chips", 0x009dff},
    {"green", 0x4bc292},     {"money", 0xf3b958},     {"gold", 0xeac058},     {"attention", 0xff9a00},
    {"purple", 0x8867a5},    {"white", 0xffffff},     {"inactive", 0x888888}, {"spades", 0x403995},
    {"hearts", 0xf03464},    {"clubs", 0x235955},     {"diamonds", 0xf06b3f}, {"tarot", 0xa782d1},
    {"planet", 0x13afce},    {"spectral", 0x4584fa},  {"edition", 0x4ca893},  {"dark_edition", 0x5d5dff},
    {"legendary", 0xb26cbb}, {"enhanced", 0x8389dd},  {"black", 0x374244},    {"filter", 0xff9a00},
};

BOColor bo_rich_colour(const char *name, BOColor fallback) {
    for (size_t i = 0; i < sizeof(kColours) / sizeof(kColours[0]); i++) {
        if (strcmp(kColours[i].name, name) == 0) {
            BOColor c = bo_rgb(kColours[i].hex);
            if (strcmp(name, "inactive") == 0) c.a = 0.6f;
            return c;
        }
    }
    return fallback;
}

typedef struct {
    char text[128];
    BOColor colour;
    int has_box;
    BOColor box;
    float scale;
} BOSegment;

// loc_parse_string: splits a line into parts, each with its controls.
static int parse(const char *line, BOColor fallback, BOSegment *out, int max) {
    int count = 0;
    BOSegment current;
    memset(&current, 0, sizeof(current));
    current.colour = fallback;
    current.scale = 1;
    int length = 0;
    for (const char *p = line; *p;) {
        if (*p == '{') {
            if (length > 0 && count < max) {
                current.text[length] = 0;
                out[count++] = current;
            }
            length = 0;
            memset(&current, 0, sizeof(current));
            current.colour = fallback;
            current.scale = 1;
            const char *end = strchr(p, '}');
            if (!end) break;
            // The controls, as "C:mult,s:1.1".
            char controls[96];
            size_t n = (size_t)(end - p - 1);
            if (n >= sizeof(controls)) n = sizeof(controls) - 1;
            memcpy(controls, p + 1, n);
            controls[n] = 0;
            for (char *part = strtok(controls, ","); part; part = strtok(NULL, ",")) {
                char *colon = strchr(part, ':');
                if (!colon) continue;
                *colon = 0;
                const char *value = colon + 1;
                if (strcmp(part, "C") == 0) current.colour = bo_rich_colour(value, fallback);
                if (strcmp(part, "X") == 0) {
                    current.has_box = 1;
                    current.box = bo_rich_colour(value, bo_rgb(0xfe5f55));
                }
                if (strcmp(part, "s") == 0) current.scale = (float)atof(value);
                if (strcmp(part, "V") == 0) current.colour = bo_rich_colour("attention", fallback);
            }
            p = end + 1;
            continue;
        }
        if (length < (int)sizeof(current.text) - 1) current.text[length++] = *p;
        p++;
    }
    if (length > 0 && count < max) {
        current.text[length] = 0;
        out[count++] = current;
    }
    return count;
}

float bo_rich_width(const char *line, float height) {
    BOSegment segments[24];
    int count = parse(line, bo_rgb(0xffffff), segments, 24);
    float width = 0;
    for (int i = 0; i < count; i++) {
        float size = height * (segments[i].scale > 0 ? segments[i].scale : 1);
        if (size > height * 1.15f) size = height * 1.15f;
        width += bo_font_width(segments[i].text, size);
    }
    return width;
}

float bo_rich_draw(const char *line, float x, float y, float height, BOColor fallback, int align) {
    BOSegment segments[24];
    int count = parse(line, fallback, segments, 24);
    float width = bo_rich_width(line, height);
    if (align == BO_ALIGN_CENTER) x -= width / 2;
    if (align == BO_ALIGN_RIGHT) x -= width;
    for (int i = 0; i < count; i++) {
        BOSegment *s = &segments[i];
        float size = height * (s->scale > 0 ? s->scale : 1);
        if (size > height * 1.15f) size = height * 1.15f;
        float w = bo_font_width(s->text, size);
        float sy = y + (height - size) / 2;
        if (s->has_box) bo_gfx_round_rect(x, y - height * 0.05f, w, height * 1.1f, height * 0.2f, s->box);
        bo_font_draw(s->text, x, sy, size, s->colour, BO_ALIGN_LEFT);
        x += w;
    }
    return width;
}
