
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

/*****************************************************************************
 *
 *  D32 functions
 *
 */

// SRC == 8888

#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_opaque)(x, y, a, b, c, d, dst)

#define MAKENAME(suffix)        NAME_WRAP(S32_opaque_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(4 == state.fPixmap.info().bytesPerPixel()); \
                                SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          src
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_alpha)(x, y, a, b, c, d, dst, alphaScale)

#define MAKENAME(suffix)        NAME_WRAP(S32_alpha_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(4 == state.fPixmap.info().bytesPerPixel()); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(src, alphaScale)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

// SRC == 565

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst) \
    do {                                                        \
        uint32_t tmp = Filter_565_Expanded(x, y, a, b, c, d);   \
        *(dst) = SkExpanded_565_To_PMColor(tmp);                \
    } while (0)

#define MAKENAME(suffix)        NAME_WRAP(S16_opaque_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(kRGB_565_SkColorType == state.fPixmap.colorType()); \
                                SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          SkPixel16ToPixel32(src)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst) \
    do {                                                                    \
        uint32_t tmp = Filter_565_Expanded(x, y, a, b, c, d);               \
        *(dst) = SkAlphaMulQ(SkExpanded_565_To_PMColor(tmp), alphaScale);   \
    } while (0)

#define MAKENAME(suffix)        NAME_WRAP(S16_alpha_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(kRGB_565_SkColorType == state.fPixmap.colorType()); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(SkPixel16ToPixel32(src), alphaScale)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

// SRC == Index8

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_opaque)(x, y, a, b, c, d, dst)

#define MAKENAME(suffix)        NAME_WRAP(SI8_opaque_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(kIndex_8_SkColorType == state.fPixmap.colorType()); \
                                SkASSERT(state.fAlphaScale == 256)
#define PREAMBLE(state)         const SkPMColor* SK_RESTRICT table = state.fPixmap.ctable()->readColors()
#define RETURNDST(src)          table[src]
#define SRC_TO_FILTER(src)      table[src]
#define POSTAMBLE(state)
#include "SkBitmapProcState_sample.h"

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_alpha)(x, y, a, b, c, d, dst, alphaScale)

#define MAKENAME(suffix)        NAME_WRAP(SI8_alpha_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(kIndex_8_SkColorType == state.fPixmap.colorType()); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale; \
                                const SkPMColor* SK_RESTRICT table = state.fPixmap.ctable()->readColors()
#define RETURNDST(src)          SkAlphaMulQ(table[src], alphaScale)
#define SRC_TO_FILTER(src)      table[src]
#define POSTAMBLE(state)
#include "SkBitmapProcState_sample.h"

// SRC == 4444

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)  *(dst) = Filter_4444_D32(x, y, a, b, c, d)

#define MAKENAME(suffix)        NAME_WRAP(S4444_opaque_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor16
#define CHECKSTATE(state)       SkASSERT(kARGB_4444_SkColorType == state.fPixmap.colorType()); \
                                SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          SkPixel4444ToPixel32(src)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)  \
    do {                                                    \
        uint32_t tmp = Filter_4444_D32(x, y, a, b, c, d);   \
        *(dst) = SkAlphaMulQ(tmp, alphaScale);              \
    } while (0)

#define MAKENAME(suffix)        NAME_WRAP(S4444_alpha_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor16
#define CHECKSTATE(state)       SkASSERT(kARGB_4444_SkColorType == state.fPixmap.colorType()); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(SkPixel4444ToPixel32(src), alphaScale)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

// SRC == A8

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst) \
    do {                                                        \
        unsigned tmp = Filter_8(x, y, a, b, c, d);              \
        *(dst) = SkAlphaMulQ(pmColor, SkAlpha255To256(tmp));    \
    } while (0)

#define MAKENAME(suffix)        NAME_WRAP(SA8_alpha_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(kAlpha_8_SkColorType == state.fPixmap.colorType());
#define PREAMBLE(state)         const SkPMColor pmColor = state.fPaintPMColor;
#define RETURNDST(src)          SkAlphaMulQ(pmColor, SkAlpha255To256(src))
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

// SRC == Gray8

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst) \
    do {                                                        \
        unsigned tmp = Filter_8(x, y, a, b, c, d);              \
        SkPMColor color = SkPackARGB32(0xFF, tmp, tmp, tmp);    \
        *(dst) = SkAlphaMulQ(color, alphaScale);                \
    } while (0)

#define MAKENAME(suffix)        NAME_WRAP(SG8_alpha_D32 ## suffix)
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(kGray_8_SkColorType == state.fPixmap.colorType());
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(SkPackARGB32(0xFF, src, src, src), alphaScale)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

/*****************************************************************************
 *
 *  D16 functions
 *
 */

// SRC == 8888

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst) \
    do {                                                \
        SkPMColor dstColor;                             \
        NAME_WRAP(Filter_32_opaque)(x, y, a, b, c, d, &dstColor);  \
        (*dst) = SkPixel32ToPixel16(dstColor);          \
    } while (0)

#define MAKENAME(suffix)        NAME_WRAP(S32_D16 ## suffix)
#define DSTSIZE                 16
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(4 == state.fPixmap.info().bytesPerPixel()); \
                                SkASSERT(state.fPixmap.isOpaque())
#define RETURNDST(src)          SkPixel32ToPixel16(src)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

// SRC == 565

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst) \
    do {                                                        \
        uint32_t tmp = Filter_565_Expanded(x, y, a, b, c, d);   \
        *(dst) = SkCompact_rgb_16((tmp) >> 5);                  \
    } while (0)

#define MAKENAME(suffix)        NAME_WRAP(S16_D16 ## suffix)
#define DSTSIZE                 16
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(kRGB_565_SkColorType == state.fPixmap.colorType())
#define RETURNDST(src)          src
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

// SRC == Index8

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst) \
    do {                                                        \
        uint32_t tmp = Filter_565_Expanded(x, y, a, b, c, d);   \
        *(dst) = SkCompact_rgb_16((tmp) >> 5);                  \
    } while (0)

#define MAKENAME(suffix)        NAME_WRAP(SI8_D16 ## suffix)
#define DSTSIZE                 16
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(kIndex_8_SkColorType == state.fPixmap.colorType()); \
                                SkASSERT(state.fPixmap.isOpaque())
#define PREAMBLE(state)         const uint16_t* SK_RESTRICT table = state.fPixmap.ctable()->read16BitCache()
#define RETURNDST(src)          table[src]
#define SRC_TO_FILTER(src)      table[src]
#define POSTAMBLE(state)
#include "SkBitmapProcState_sample.h"

///////////////////////////////////////////////////////////////////////////////

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst) \
    do {                                                        \
        uint32_t tmp = Filter_565_Expanded(x, y, a, b, c, d);   \
        *(dst) = SkCompact_rgb_16((tmp) >> 5);                  \
    } while (0)


// clamp

#define TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
#define TILEY_PROCF(fy, max)    SkClampMax((fy) >> 16, max)
#define TILEX_LOW_BITS(fx, max) (((fx) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) (((fy) >> 12) & 0xF)

#define MAKENAME(suffix)        NAME_WRAP(Clamp_S16_D16 ## suffix)
#define SRCTYPE                 uint16_t
#define DSTTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(kRGB_565_SkColorType == state.fPixmap.colorType())
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_shaderproc.h"


#define TILEX_PROCF(fx, max)    (((fx) & 0xFFFF) * ((max) + 1) >> 16)
#define TILEY_PROCF(fy, max)    (((fy) & 0xFFFF) * ((max) + 1) >> 16)
#define TILEX_LOW_BITS(fx, max) ((((fx) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) ((((fy) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)

#define MAKENAME(suffix)        NAME_WRAP(Repeat_S16_D16 ## suffix)
#define SRCTYPE                 uint16_t
#define DSTTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(kRGB_565_SkColorType == state.fPixmap.colorType())
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_shaderproc.h"


#define TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
#define TILEY_PROCF(fy, max)    SkClampMax((fy) >> 16, max)
#define TILEX_LOW_BITS(fx, max) (((fx) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) (((fy) >> 12) & 0xF)

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   NAME_WRAP(Filter_32_opaque)(x, y, a, b, c, d, dst)
#define MAKENAME(suffix)        NAME_WRAP(Clamp_SI8_opaque_D32 ## suffix)
#define SRCTYPE                 uint8_t
#define DSTTYPE                 uint32_t
#define CHECKSTATE(state)       SkASSERT(kIndex_8_SkColorType == state.fPixmap.colorType())
#define PREAMBLE(state)         const SkPMColor* SK_RESTRICT table = state.fPixmap.ctable()->readColors()
#define SRC_TO_FILTER(src)      table[src]
#define POSTAMBLE(state)
#include "SkBitmapProcState_shaderproc.h"

#undef NAME_WRAP
