/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkImageInfo.h"
#include "SkMath.h"
#include "SkUnPreMultiply.h"

#include "sk_bitmap.h"

#include "sk_types_priv.h"

// converters

static inline void copyAlpha8ToColor(size_t size, const uint8_t* pixels, sk_color_t* colors) {
    while (size-- != 0) {
        const uint8_t* addr = pixels++;
        *colors++ = SkColorSetA(0, *addr);
    }
}

static inline void copyGray8ToColor(size_t size, const uint8_t* pixels, sk_color_t* colors) {
    while (size-- != 0) {
        const uint8_t* addr = pixels++;
        *colors++ = SkColorSetRGB(*addr, *addr, *addr);
    }
}

static inline void copyRgb565ToColor(size_t size, const uint16_t* pixels, sk_color_t* colors) {
    while (size-- != 0) {
        const uint16_t* addr = pixels++;
        *colors++ = SkPixel16ToColor(*addr);
    }
}

static inline void copy8888ToColor(size_t size, const uint32_t* pixels, sk_color_t* colors) {
    while (size-- != 0) {
        const uint32_t* addr = pixels++;
        *colors++ = SkUnPreMultiply::PMColorToColor(*addr);
    }
}

static inline void copyAlpha8FromColor(size_t size, const sk_color_t* colors, uint8_t* pixels) {
    while (size-- != 0) {
        *pixels++ = SkColorGetA(*colors++);
    }
}

static inline void copyGray8FromColor(size_t size, const sk_color_t* colors, uint8_t* pixels) {
    while (size-- != 0) {
        SkColor c = *colors++;

        uint8_t r = SkColorGetR(c);
        uint8_t g = SkColorGetG(c);
        uint8_t b = SkColorGetB(c);
        uint8_t a = SkColorGetA(c);
        if (255 != a) {
            r = SkMulDiv255Round(r, a);
            g = SkMulDiv255Round(g, a);
            b = SkMulDiv255Round(b, a);
        }
        *pixels++ = SkComputeLuminance(r, g, b);
    }
}

static inline void copyRgb565FromColor(size_t width, size_t height, const sk_color_t* colors, uint16_t* pixels) {
    for (size_t y = 0; y < height; y++) {
        DITHER_565_SCAN(y);
        for (size_t x = 0; x < width; x++) {
            SkColor c = *colors++;
            *pixels++ = SkDitherRGBTo565(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c), DITHER_VALUE(x));
        }
    }
}

static inline void copy8888FromColor(size_t size, const sk_color_t* colors, uint32_t* pixels) {
    while (size-- != 0) {
        *pixels++ = SkPreMultiplyColor(*colors++);
    }
}

// C API

void sk_bitmap_destructor(sk_bitmap_t* cbitmap) {
    delete AsBitmap(cbitmap);
}

sk_bitmap_t* sk_bitmap_new() {
    return ToBitmap(new SkBitmap());
}

void sk_bitmap_get_info(sk_bitmap_t* cbitmap, sk_imageinfo_t* info) {
    *info = ToImageInfo(AsBitmap(cbitmap)->info());
}

void* sk_bitmap_get_pixels(sk_bitmap_t* cbitmap, size_t* length) {
    SkBitmap* bmp = AsBitmap(cbitmap);
    *length = bmp->computeByteSize();
    return bmp->getPixels();
}

size_t sk_bitmap_get_row_bytes(sk_bitmap_t* cbitmap) {
    return AsBitmap(cbitmap)->rowBytes();
}

size_t sk_bitmap_get_byte_count(sk_bitmap_t* cbitmap) {
    return AsBitmap(cbitmap)->computeByteSize();
}

void sk_bitmap_reset(sk_bitmap_t* cbitmap) {
    AsBitmap(cbitmap)->reset();
}

bool sk_bitmap_is_null(sk_bitmap_t* cbitmap) {
    return AsBitmap(cbitmap)->isNull();
}

bool sk_bitmap_is_immutable(sk_bitmap_t* cbitmap) {
    return AsBitmap(cbitmap)->isImmutable();
}

void sk_bitmap_set_immutable(sk_bitmap_t* cbitmap) {
    AsBitmap(cbitmap)->setImmutable();
}

bool sk_bitmap_is_volatile(sk_bitmap_t* cbitmap) {
    return AsBitmap(cbitmap)->isVolatile();
}

void sk_bitmap_set_volatile(sk_bitmap_t* cbitmap, bool value) {
    AsBitmap(cbitmap)->setIsVolatile(value);
}

void sk_bitmap_erase(sk_bitmap_t* cbitmap, sk_color_t color) {
    AsBitmap(cbitmap)->eraseColor(color);
}

void sk_bitmap_erase_rect(sk_bitmap_t* cbitmap, sk_color_t color, sk_irect_t* rect) {
    AsBitmap(cbitmap)->erase(color, *AsIRect(rect));
}

uint8_t sk_bitmap_get_addr_8(sk_bitmap_t* cbitmap, int x, int y) {
    return *(AsBitmap(cbitmap)->getAddr8(x, y));
}

uint16_t sk_bitmap_get_addr_16(sk_bitmap_t* cbitmap, int x, int y) {
    return *(AsBitmap(cbitmap)->getAddr16(x, y));
}

uint32_t sk_bitmap_get_addr_32(sk_bitmap_t* cbitmap, int x, int y) {
    return *(AsBitmap(cbitmap)->getAddr32(x, y));
}

void* sk_bitmap_get_addr(sk_bitmap_t* cbitmap, int x, int y) {
    return AsBitmap(cbitmap)->getAddr(x, y);
}

sk_color_t sk_bitmap_get_pixel_color(sk_bitmap_t* cbitmap, int x, int y) {
    return AsBitmap(cbitmap)->getColor(x, y);
}

void sk_bitmap_set_pixel_color(sk_bitmap_t* cbitmap, int x, int y, sk_color_t color) {
    SkBitmap* bmp = AsBitmap(cbitmap);

    switch (bmp->colorType()) {
    case kAlpha_8_SkColorType:
        copyAlpha8FromColor(1, &color, (uint8_t*)bmp->getAddr8(x, y));
        break;
    case kGray_8_SkColorType:
        copyGray8FromColor(1, &color, (uint8_t*)bmp->getAddr8(x, y));
        break;
    case kRGB_565_SkColorType:
        copyRgb565FromColor(1, 1, &color, (uint16_t*)bmp->getAddr16(x, y));
        break;
    case kBGRA_8888_SkColorType:
    case kRGBA_8888_SkColorType:
        copy8888FromColor(1, &color, (uint32_t*)bmp->getAddr32(x, y));
        break;
    default:
        break;
    }
}

bool sk_bitmap_ready_to_draw(sk_bitmap_t* cbitmap) {
    return AsBitmap(cbitmap)->readyToDraw();
}

void sk_bitmap_get_pixel_colors(sk_bitmap_t* cbitmap, sk_color_t* colors) {
    SkBitmap* bmp = AsBitmap(cbitmap);

    size_t size = bmp->height() * bmp->width();
    const void* pixels = bmp->getPixels();

    switch (bmp->colorType()) {
    case kAlpha_8_SkColorType:
        copyAlpha8ToColor(size, (const uint8_t*)pixels, colors);
        break;
    case kGray_8_SkColorType:
        copyGray8ToColor(size, (const uint8_t*)pixels, colors);
        break;
    case kRGB_565_SkColorType:
        copyRgb565ToColor(size, (const uint16_t*)pixels, colors);
        break;
    case kBGRA_8888_SkColorType:
    case kRGBA_8888_SkColorType:
        copy8888ToColor(size, (const uint32_t*)pixels, colors);
        break;
    default:
        break;
    }
}

void sk_bitmap_set_pixel_colors(sk_bitmap_t* cbitmap, const sk_color_t* colors) {
    SkBitmap* bmp = AsBitmap(cbitmap);

    size_t width = bmp->width();
    size_t height = bmp->height();
    size_t size = height * width;
    void* pixels = bmp->getPixels();

    switch (bmp->colorType()) {
    case kAlpha_8_SkColorType:
        copyAlpha8FromColor(size, colors, (uint8_t*)pixels);
        break;
    case kGray_8_SkColorType:
        copyGray8FromColor(size, colors, (uint8_t*)pixels);
        break;
    case kRGB_565_SkColorType:
        copyRgb565FromColor(width, height, colors, (uint16_t*)pixels);
        break;
    case kBGRA_8888_SkColorType:
    case kRGBA_8888_SkColorType:
        copy8888FromColor(size, colors, (uint32_t*)pixels);
        break;
    default:
        break;
    }
}

bool sk_bitmap_install_pixels(sk_bitmap_t* cbitmap, const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes, const sk_bitmap_release_proc releaseProc, void* context) {
    return AsBitmap(cbitmap)->installPixels(AsImageInfo(cinfo), pixels, rowBytes, releaseProc, context);
}

bool sk_bitmap_install_pixels_with_pixmap(sk_bitmap_t* cbitmap, const sk_pixmap_t* cpixmap) {
    return AsBitmap(cbitmap)->installPixels(*AsPixmap(cpixmap));
}

bool sk_bitmap_install_mask_pixels(sk_bitmap_t* cbitmap, const sk_mask_t* cmask) {
    return AsBitmap(cbitmap)->installMaskPixels(*AsMask(cmask));
}

bool sk_bitmap_try_alloc_pixels(sk_bitmap_t* cbitmap, const sk_imageinfo_t* requestedInfo, size_t rowBytes) {
    return AsBitmap(cbitmap)->tryAllocPixels(AsImageInfo(requestedInfo), rowBytes);
}

bool sk_bitmap_try_alloc_pixels_with_flags(sk_bitmap_t* cbitmap, const sk_imageinfo_t* requestedInfo, uint32_t flags) {
    return AsBitmap(cbitmap)->tryAllocPixelsFlags(AsImageInfo(requestedInfo), flags);
}

void sk_bitmap_set_pixels(sk_bitmap_t* cbitmap, void* pixels) {
    AsBitmap(cbitmap)->setPixels(pixels);
}

bool sk_bitmap_peek_pixels(sk_bitmap_t* cbitmap, sk_pixmap_t* cpixmap) {
    return AsBitmap(cbitmap)->peekPixels(AsPixmap(cpixmap));
}

bool sk_bitmap_extract_subset(sk_bitmap_t* cbitmap, sk_bitmap_t* cdst, sk_irect_t* subset) {
    return AsBitmap(cbitmap)->extractSubset(AsBitmap(cdst), *AsIRect(subset));
}

bool sk_bitmap_extract_alpha(sk_bitmap_t* cbitmap, sk_bitmap_t* cdst, const sk_paint_t* paint, sk_ipoint_t* offset) {
    return AsBitmap(cbitmap)->extractAlpha(AsBitmap(cdst), AsPaint(paint), AsIPoint(offset));
}

void sk_bitmap_notify_pixels_changed(sk_bitmap_t* cbitmap) {
    AsBitmap(cbitmap)->notifyPixelsChanged();
}

void sk_bitmap_swap(sk_bitmap_t* cbitmap, sk_bitmap_t* cother) {
    AsBitmap(cbitmap)->swap(*AsBitmap(cother));
}
