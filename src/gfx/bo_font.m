#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>
#include <math.h>
#include <stdlib.h>
#include "bo_font.h"

#define BO_FIRST_CHAR 32
#define BO_LAST_CHAR 126
#define BO_CHARS (BO_LAST_CHAR - BO_FIRST_CHAR + 1)
#define BO_COLUMNS 16

typedef struct {
    float x, y;     // The place of the cell in the texture.
    float advance;  // The step to the next character, in texels.
} BOGlyph;

static BOTexture gTexture;
static BOGlyph gGlyphs[BO_CHARS];
static float gCellWidth;
static float gLineHeight;

int bo_font_load(const unsigned char *ttf, size_t size, float pixels) {
    CFDataRef data = CFDataCreate(NULL, ttf, (CFIndex)size);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
    CFRelease(data);
    CGFontRef cgFont = CGFontCreateWithDataProvider(provider);
    CGDataProviderRelease(provider);
    if (!cgFont) return 0;
    CTFontRef font = CTFontCreateWithGraphicsFont(cgFont, pixels, NULL, NULL);
    CGFontRelease(cgFont);
    if (!font) return 0;

    UniChar chars[BO_CHARS];
    CGGlyph glyphs[BO_CHARS];
    CGSize advances[BO_CHARS];
    for (int i = 0; i < BO_CHARS; i++) chars[i] = (UniChar)(BO_FIRST_CHAR + i);
    CTFontGetGlyphsForCharacters(font, chars, glyphs, BO_CHARS);
    CTFontGetAdvancesForGlyphs(font, kCTFontOrientationHorizontal, glyphs, advances, BO_CHARS);

    CGFloat ascent = CTFontGetAscent(font), descent = CTFontGetDescent(font);
    gLineHeight = ceilf((float)(ascent + descent));
    float widest = 0;
    for (int i = 0; i < BO_CHARS; i++) widest = fmaxf(widest, (float)advances[i].width);
    gCellWidth = ceilf(widest) + 2;
    float cellHeight = gLineHeight + 2;
    int rows = (BO_CHARS + BO_COLUMNS - 1) / BO_COLUMNS;
    int width = (int)(gCellWidth * BO_COLUMNS), height = (int)(cellHeight * rows);

    unsigned char *alpha = calloc((size_t)width * height, 1);
    CGContextRef context = CGBitmapContextCreate(alpha, width, height, 8, width, NULL, kCGImageAlphaOnly);
    CGContextSetGrayFillColor(context, 1, 1);
    for (int i = 0; i < BO_CHARS; i++) {
        float cx = (float)(i % BO_COLUMNS) * gCellWidth + 1;
        float cy = (float)(i / BO_COLUMNS) * cellHeight + 1;
        gGlyphs[i].x = cx;
        gGlyphs[i].y = cy;
        gGlyphs[i].advance = (float)advances[i].width;
        // Core Graphics has its origin at the bottom left.
        CGPoint position = CGPointMake(cx, height - cy - (float)ascent);
        CTFontDrawGlyphs(font, &glyphs[i], &position, 1, context);
    }
    CGContextRelease(context);
    CFRelease(font);

    // White text with premultiplied alpha: each channel is the alpha.
    unsigned char *rgba = malloc((size_t)width * height * 4);
    for (int i = 0; i < width * height; i++) {
        rgba[i * 4 + 0] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = rgba[i * 4 + 3] = alpha[i];
    }
    free(alpha);
    gTexture = bo_gfx_texture(rgba, width, height);
    free(rgba);
    return 1;
}

static const BOGlyph *glyph_for(char c) {
    if (c < BO_FIRST_CHAR || c > BO_LAST_CHAR) c = '?';
    return &gGlyphs[c - BO_FIRST_CHAR];
}

float bo_font_width(const char *text, float height) {
    if (!gLineHeight) return 0;
    float scale = height / gLineHeight, width = 0;
    for (const char *p = text; *p; p++) width += glyph_for(*p)->advance * scale;
    return width;
}

float bo_font_draw(const char *text, float x, float y, float height, BOColor color, int align) {
    if (!gTexture.id) return 0;
    float width = bo_font_width(text, height);
    if (align == BO_ALIGN_CENTER) x -= width / 2;
    if (align == BO_ALIGN_RIGHT) x -= width;
    float scale = height / gLineHeight;
    for (const char *p = text; *p; p++) {
        const BOGlyph *g = glyph_for(*p);
        if (*p != ' ') {
            bo_gfx_sprite(&gTexture, g->x, g->y, gCellWidth - 1, gLineHeight, x, y, (gCellWidth - 1) * scale,
                          height, 0, color);
        }
        x += g->advance * scale;
    }
    return width;
}
