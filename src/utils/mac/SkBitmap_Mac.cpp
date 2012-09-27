/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkMath.h"

#if defined(SK_BUILD_FOR_MAC)

#include <ApplicationServices/ApplicationServices.h>

#ifndef __ppc__
    #define SWAP_16BIT
#endif

static void convertGL32_to_Mac32(uint32_t dst[], const SkBitmap& bm) {
    memcpy(dst, bm.getPixels(), bm.getSize());
    return;

    uint32_t* stop = dst + (bm.getSize() >> 2);
    const uint8_t* src = (const uint8_t*)bm.getPixels();
    while (dst < stop) {
        *dst++ = src[2] << 24 | src[1] << 16 | src[0] << 8 | src[3] << 0;
        src += sizeof(uint32_t);
    }
}

static void convert565_to_32(uint32_t dst[], const SkBitmap& bm) {
    for (int y = 0; y < bm.height(); y++) {
        const uint16_t* src = bm.getAddr16(0, y);
        const uint16_t* stop = src + bm.width();
        while (src < stop) {
            unsigned c = *src++;
            unsigned r = SkPacked16ToR32(c);
            unsigned g = SkPacked16ToG32(c);
            unsigned b = SkPacked16ToB32(c);

            *dst++ = (b << 24) | (g << 16) | (r << 8) | 0xFF;
        }
    }
}

static void convert4444_to_555(uint16_t dst[], const uint16_t src[], int count)
{
    const uint16_t* stop = src + count;

    while (src < stop)
    {
        unsigned c = *src++;

        unsigned r = SkGetPackedR4444(c);
        unsigned g = SkGetPackedG4444(c);
        unsigned b = SkGetPackedB4444(c);
        // convert to 5 bits
        r = (r << 1) | (r >> 3);
        g = (g << 1) | (g >> 3);
        b = (b << 1) | (b >> 3);
        // build the 555
        c = (r << 10) | (g << 5) | b;

#ifdef SWAP_16BIT
        c = (c >> 8) | (c << 8);
#endif
        *dst++ = c;
    }
}

#include "SkTemplates.h"

static CGImageRef bitmap2imageref(const SkBitmap& bm) {
    size_t  bitsPerComp;
    size_t  bitsPerPixel;
    CGBitmapInfo info;
    CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGDataProviderRef data = CGDataProviderCreateWithData(NULL,
                                                           bm.getPixels(),
                                                           bm.getSize(),
                                                           NULL);
    SkAutoTCallVProc<CGDataProvider, CGDataProviderRelease> acp(data);
    SkAutoTCallVProc<CGColorSpace, CGColorSpaceRelease> acp2(cs);

    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            bitsPerComp = 8;
            bitsPerPixel = 32;
            info = kCGImageAlphaPremultipliedLast;
            break;
        case SkBitmap::kARGB_4444_Config:
            bitsPerComp = 4;
            bitsPerPixel = 16;
            info = kCGImageAlphaPremultipliedLast |  kCGBitmapByteOrder16Little;
            break;
#if 0   // not supported by quartz !!!
        case SkBitmap::kRGB_565_Config:
            bitsPerComp = 5;
            bitsPerPixel = 16;
            info = kCGImageAlphaNone | kCGBitmapByteOrder16Little;
            break;
#endif
        default:
            return NULL;
    }

    return CGImageCreate(bm.width(), bm.height(), bitsPerComp, bitsPerPixel,
                         bm.rowBytes(), cs, info, data,
                         NULL, false, kCGRenderingIntentDefault);
}

void SkBitmap::drawToPort(WindowRef wind, CGContextRef cg) const {
    if (fPixels == NULL || fWidth == 0 || fHeight == 0) {
        return;
    }

    bool useQD = false;
    if (NULL == cg) {
        SetPortWindowPort(wind);
        QDBeginCGContext(GetWindowPort(wind), &cg);
        useQD = true;
    }

    SkBitmap bm;
    if (this->config() == kRGB_565_Config) {
        this->copyTo(&bm, kARGB_8888_Config);
    } else {
        bm = *this;
    }
    bm.lockPixels();

    CGImageRef image = bitmap2imageref(bm);
    if (image) {
        CGRect rect;
        rect.origin.x = rect.origin.y = 0;
        rect.size.width = bm.width();
        rect.size.height = bm.height();

        CGContextDrawImage(cg, rect, image);
        CGImageRelease(image);
    }

    if (useQD) {
        QDEndCGContext(GetWindowPort(wind), &cg);
    }
}

#endif
