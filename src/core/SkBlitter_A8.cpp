/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCoreBlitters.h"
#include "SkColorPriv.h"
#include "SkShader.h"
#include "SkXfermodePriv.h"

SkA8_Coverage_Blitter::SkA8_Coverage_Blitter(const SkPixmap& device,
                             const SkPaint& paint) : SkRasterBlitter(device) {
    SkASSERT(nullptr == paint.getShader());
    SkASSERT(paint.isSrcOver());
    SkASSERT(nullptr == paint.getColorFilter());
}

void SkA8_Coverage_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                      const int16_t runs[]) {
    uint8_t* device = fDevice.writable_addr8(x, y);
    SkDEBUGCODE(int totalCount = 0;)

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count == 0) {
            return;
        }
        if (antialias[0]) {
            memset(device, antialias[0], count);
        }
        runs += count;
        antialias += count;
        device += count;

        SkDEBUGCODE(totalCount += count;)
    }
    SkASSERT(fDevice.width() == totalCount);
}

void SkA8_Coverage_Blitter::blitH(int x, int y, int width) {
    memset(fDevice.writable_addr8(x, y), 0xFF, width);
}

void SkA8_Coverage_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (0 == alpha) {
        return;
    }

    uint8_t* dst = fDevice.writable_addr8(x, y);
    const size_t dstRB = fDevice.rowBytes();
    while (--height >= 0) {
        *dst = alpha;
        dst += dstRB;
    }
}

void SkA8_Coverage_Blitter::blitRect(int x, int y, int width, int height) {
    uint8_t* dst = fDevice.writable_addr8(x, y);
    const size_t dstRB = fDevice.rowBytes();
    while (--height >= 0) {
        memset(dst, 0xFF, width);
        dst += dstRB;
    }
}

void SkA8_Coverage_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(SkMask::kA8_Format == mask.fFormat);

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();

    uint8_t* dst = fDevice.writable_addr8(x, y);
    const uint8_t* src = mask.getAddr8(x, y);
    const size_t srcRB = mask.fRowBytes;
    const size_t dstRB = fDevice.rowBytes();

    while (--height >= 0) {
        memcpy(dst, src, width);
        dst += dstRB;
        src += srcRB;
    }
}

const SkPixmap* SkA8_Coverage_Blitter::justAnOpaqueColor(uint32_t*) {
    return nullptr;
}
