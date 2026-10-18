#import <CoreGraphics/CoreGraphics.h>
#import <ImageIO/ImageIO.h>
#include <math.h>
#include <stdlib.h>
#include "bo_image.h"

unsigned char *bo_image_decode(const unsigned char *png, size_t size, float scale, int *outWidth, int *outHeight) {
    CFDataRef data = CFDataCreateWithBytesNoCopy(NULL, png, (CFIndex)size, kCFAllocatorNull);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    CFRelease(data);
    if (!source) return NULL;
    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
    CFRelease(source);
    if (!image) return NULL;

    int width = (int)lroundf(CGImageGetWidth(image) * scale);
    int height = (int)lroundf(CGImageGetHeight(image) * scale);
    unsigned char *pixels = calloc((size_t)width * height, 4);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(pixels, width, height, 8, width * 4, space,
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(space);
    if (!context) {
        free(pixels);
        CGImageRelease(image);
        return NULL;
    }
    CGContextSetInterpolationQuality(context, scale == 1 ? kCGInterpolationNone : kCGInterpolationHigh);
    CGContextSetBlendMode(context, kCGBlendModeCopy);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
    CGContextRelease(context);
    CGImageRelease(image);
    *outWidth = width;
    *outHeight = height;
    return pixels;
}

BOTexture bo_image_texture(const unsigned char *png, size_t size, float scale) {
    BOTexture none = {0, 0, 0};
    int width = 0, height = 0;
    unsigned char *pixels = bo_image_decode(png, size, scale, &width, &height);
    if (!pixels) return none;
    BOTexture texture = bo_gfx_texture(pixels, width, height);
    free(pixels);
    return texture;
}
