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

// returns expanded * 5bits
static inline uint32_t Filter_565_Expanded(unsigned x, unsigned y,
                                           uint32_t a00, uint32_t a01,
                                           uint32_t a10, uint32_t a11) {
    SkASSERT((unsigned)x <= 0xF);
    SkASSERT((unsigned)y <= 0xF);

    a00 = SkExpand_rgb_16(a00);
    a01 = SkExpand_rgb_16(a01);
    a10 = SkExpand_rgb_16(a10);
    a11 = SkExpand_rgb_16(a11);

    int xy = x * y >> 3;
    return  a00 * (32 - 2*y - 2*x + xy) +
            a01 * (2*x - xy) +
            a10 * (2*y - xy) +
            a11 * xy;
}

// turn an expanded 565 * 5bits into SkPMColor
// g:11 | r:10 | x:1 | b:10
static inline SkPMColor SkExpanded_565_To_PMColor(uint32_t c) {
    unsigned r = (c >> 13) & 0xFF;
    unsigned g = (c >> 24);
    unsigned b = (c >> 2) & 0xFF;
    return SkPackARGB32(0xFF, r, g, b);
}

// returns answer in SkPMColor format
static inline SkPMColor Filter_4444_D32(unsigned x, unsigned y,
                                        uint32_t a00, uint32_t a01,
                                        uint32_t a10, uint32_t a11) {
    SkASSERT((unsigned)x <= 0xF);
    SkASSERT((unsigned)y <= 0xF);

    a00 = SkExpand_4444(a00);
    a01 = SkExpand_4444(a01);
    a10 = SkExpand_4444(a10);
    a11 = SkExpand_4444(a11);

    int xy = x * y >> 4;
    uint32_t result =   a00 * (16 - y - x + xy) +
                        a01 * (x - xy) +
                        a10 * (y - xy) +
                        a11 * xy;

    return SkCompact_8888(result);
}

static inline U8CPU Filter_8(unsigned x, unsigned y,
                             U8CPU a00, U8CPU a01,
                             U8CPU a10, U8CPU a11) {
    SkASSERT((unsigned)x <= 0xF);
    SkASSERT((unsigned)y <= 0xF);

    int xy = x * y;
    unsigned result =   a00 * (256 - 16*y - 16*x + xy) +
                        a01 * (16*x - xy) +
                        a10 * (16*y - xy) +
                        a11 * xy;

    return result >> 8;
}

// SRC == 8888

#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_opaque)(x, y, a, b, c, d, dst)

#define MAKENAME(suffix)        NAME_WRAP(S32_opaque_D32 ## suffix)
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(4 == state.fPixmap.info().bytesPerPixel()); \
                                SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          src
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_alpha)(x, y, a, b, c, d, dst, alphaScale)

#define MAKENAME(suffix)        NAME_WRAP(S32_alpha_D32 ## suffix)
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(4 == state.fPixmap.info().bytesPerPixel()); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(src, alphaScale)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef NAME_WRAP
