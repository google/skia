/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Define NAME_WRAP(x) before including this header to perform name-wrapping
// E.g. for ARM NEON, defined it as 'x ## _neon' to ensure all important
// identifiers have a _neon suffix.
#ifndef NAME_WRAP
    #error "Please define NAME_WRAP() before including this file"
#endif

#define FILTER_PROC(x, y, a, b, c, d, dst)  \
    NAME_WRAP(Filter_32_alpha)(x, y, a, b, c, d, dst, alphaScale)

#include "SkUtils.h"

void S32_D32_nofilter_DXDY(const SkBitmapProcState&, const uint32_t*, int, SkPMColor*);
void S32_D32_nofilter_DX  (const SkBitmapProcState&, const uint32_t*, int, SkPMColor*);
void S32_D32_filter_DX    (const SkBitmapProcState&, const uint32_t*, int, SkPMColor*);
void S32_D32_filter_DXDY  (const SkBitmapProcState&, const uint32_t*, int, SkPMColor*);

void S32_D32_nofilter_DXDY(const SkBitmapProcState& s,
                           const uint32_t* xy, int count, SkPMColor* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(kNone_SkFilterQuality == s.fFilterQuality);
    SkASSERT(4 == s.fPixmap.info().bytesPerPixel());

    unsigned alphaScale = s.fAlphaScale;
    const char* srcAddr = (const char*)s.fPixmap.addr();
    size_t rb = s.fPixmap.rowBytes();

    uint32_t XY;
    SkPMColor src;

    for (int i = (count >> 1); i > 0; --i) {
        XY = *xy++;
        SkASSERT((XY >> 16) < (unsigned)s.fPixmap.height() &&
                 (XY & 0xFFFF) < (unsigned)s.fPixmap.width());
        src = ((const SkPMColor*)(srcAddr + (XY >> 16) * rb))[XY & 0xFFFF];
        *colors++ = SkAlphaMulQ(src, alphaScale);

        XY = *xy++;
        SkASSERT((XY >> 16) < (unsigned)s.fPixmap.height() &&
                 (XY & 0xFFFF) < (unsigned)s.fPixmap.width());
        src = ((const SkPMColor*)(srcAddr + (XY >> 16) * rb))[XY & 0xFFFF];
        *colors++ = SkAlphaMulQ(src, alphaScale);
    }
    if (count & 1) {
        XY = *xy++;
        SkASSERT((XY >> 16) < (unsigned)s.fPixmap.height() &&
                 (XY & 0xFFFF) < (unsigned)s.fPixmap.width());
        src = ((const SkPMColor*)(srcAddr + (XY >> 16) * rb))[XY & 0xFFFF];
        *colors++ = SkAlphaMulQ(src, alphaScale);
    }

}

void S32_D32_nofilter_DX(const SkBitmapProcState& s,
                         const uint32_t* xy, int count, SkPMColor* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(kNone_SkFilterQuality == s.fFilterQuality);
    SkASSERT(4 == s.fPixmap.info().bytesPerPixel());

    unsigned alphaScale = s.fAlphaScale;
    const SkPMColor* srcAddr = (const SkPMColor*)s.fPixmap.addr();

    // buffer is y32, x16, x16, x16, x16, x16
    // bump srcAddr to the proper row, since we're told Y never changes
    SkASSERT((unsigned)xy[0] < (unsigned)s.fPixmap.height());
    srcAddr = (const SkPMColor*)((const char*)srcAddr +
                                                xy[0] * s.fPixmap.rowBytes());
    xy += 1;

    SkPMColor src;

    if (1 == s.fPixmap.width()) {
        src = srcAddr[0];
        SkPMColor dstValue = SkAlphaMulQ(src, alphaScale);
        sk_memset32(colors, dstValue, count);
    } else {
        int i;
        for (i = (count >> 2); i > 0; --i) {
            uint32_t xx0 = *xy++;
            uint32_t xx1 = *xy++;
            SkPMColor x0 = srcAddr[UNPACK_PRIMARY_SHORT(xx0)];
            SkPMColor x1 = srcAddr[UNPACK_SECONDARY_SHORT(xx0)];
            SkPMColor x2 = srcAddr[UNPACK_PRIMARY_SHORT(xx1)];
            SkPMColor x3 = srcAddr[UNPACK_SECONDARY_SHORT(xx1)];

            *colors++ = SkAlphaMulQ(x0, alphaScale);
            *colors++ = SkAlphaMulQ(x1, alphaScale);
            *colors++ = SkAlphaMulQ(x2, alphaScale);
            *colors++ = SkAlphaMulQ(x3, alphaScale);
        }
        const uint16_t* xx = (const uint16_t*)(xy);
        for (i = (count & 3); i > 0; --i) {
            SkASSERT(*xx < (unsigned)s.fPixmap.width());
            src = srcAddr[*xx++];
            *colors++ = SkAlphaMulQ(src, alphaScale);
        }
    }

}

void S32_D32_filter_DX(const SkBitmapProcState& s,
                       const uint32_t* xy, int count, SkPMColor* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
    SkASSERT(4 == s.fPixmap.info().bytesPerPixel());

    unsigned alphaScale = s.fAlphaScale;
    const char* srcAddr = (const char*)s.fPixmap.addr();
    size_t rb = s.fPixmap.rowBytes();
    unsigned subY;
    const SkPMColor* row0;
    const SkPMColor* row1;

    // setup row ptrs and update proc_table
    {
        uint32_t XY = *xy++;
        unsigned y0 = XY >> 14;
        row0 = (const SkPMColor*)(srcAddr + (y0 >> 4) * rb);
        row1 = (const SkPMColor*)(srcAddr + (XY & 0x3FFF) * rb);
        subY = y0 & 0xF;
    }

    do {
        uint32_t XX = *xy++;    // x0:14 | 4 | x1:14
        unsigned x0 = XX >> 14;
        unsigned x1 = XX & 0x3FFF;
        unsigned subX = x0 & 0xF;
        x0 >>= 4;

        FILTER_PROC(subX, subY,
                    row0[x0],
                    row0[x1],
                    row1[x0],
                    row1[x1],
                    colors);
        colors += 1;

    } while (--count != 0);
}

void S32_D32_filter_DXDY(const SkBitmapProcState& s,
                         const uint32_t* xy, int count, SkPMColor* colors) {
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(s.fFilterQuality != kNone_SkFilterQuality);
    SkASSERT(4 == s.fPixmap.info().bytesPerPixel());

    unsigned alphaScale = s.fAlphaScale;
    const char* srcAddr = (const char*)s.fPixmap.addr();
    size_t rb = s.fPixmap.rowBytes();

    do {
        uint32_t data = *xy++;
        unsigned y0 = data >> 14;
        unsigned y1 = data & 0x3FFF;
        unsigned subY = y0 & 0xF;
        y0 >>= 4;

        data = *xy++;
        unsigned x0 = data >> 14;
        unsigned x1 = data & 0x3FFF;
        unsigned subX = x0 & 0xF;
        x0 >>= 4;

        const SkPMColor* row0 = (const SkPMColor*)(srcAddr + y0 * rb);
        const SkPMColor* row1 = (const SkPMColor*)(srcAddr + y1 * rb);

        FILTER_PROC(subX, subY,
                    row0[x0],
                    row0[x1],
                    row1[x0],
                    row1[x1],
                    colors);
        colors += 1;
    } while (--count != 0);
}
#undef NAME_WRAP
