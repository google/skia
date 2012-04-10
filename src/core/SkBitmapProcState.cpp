
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBitmapProcState.h"
#include "SkBitmapProcState_filter.h"
#include "SkColorPriv.h"
#include "SkFilterProc.h"
#include "SkPaint.h"
#include "SkShader.h"   // for tilemodes

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

#define FILTER_PROC(x, y, a, b, c, d, dst)   Filter_32_opaque(x, y, a, b, c, d, dst)

#define MAKENAME(suffix)        S32_opaque_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_8888_Config); \
                                SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          src
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   Filter_32_alpha(x, y, a, b, c, d, dst, alphaScale)

#define MAKENAME(suffix)        S32_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_8888_Config); \
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

#define MAKENAME(suffix)        S16_opaque_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kRGB_565_Config); \
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

#define MAKENAME(suffix)        S16_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kRGB_565_Config); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(SkPixel16ToPixel32(src), alphaScale)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_sample.h"

// SRC == Index8

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   Filter_32_opaque(x, y, a, b, c, d, dst)

#define MAKENAME(suffix)        SI8_opaque_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kIndex8_Config); \
                                SkASSERT(state.fAlphaScale == 256)
#define PREAMBLE(state)         const SkPMColor* SK_RESTRICT table = state.fBitmap->getColorTable()->lockColors()
#define RETURNDST(src)          table[src]
#define SRC_TO_FILTER(src)      table[src]
#define POSTAMBLE(state)        state.fBitmap->getColorTable()->unlockColors(false)
#include "SkBitmapProcState_sample.h"

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   Filter_32_alpha(x, y, a, b, c, d, dst, alphaScale)

#define MAKENAME(suffix)        SI8_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kIndex8_Config); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned alphaScale = state.fAlphaScale; \
                                const SkPMColor* SK_RESTRICT table = state.fBitmap->getColorTable()->lockColors()
#define RETURNDST(src)          SkAlphaMulQ(table[src], alphaScale)
#define SRC_TO_FILTER(src)      table[src]
#define POSTAMBLE(state)        state.fBitmap->getColorTable()->unlockColors(false)
#include "SkBitmapProcState_sample.h"

// SRC == 4444

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)  *(dst) = Filter_4444_D32(x, y, a, b, c, d)

#define MAKENAME(suffix)        S4444_opaque_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor16
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_4444_Config); \
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

#define MAKENAME(suffix)        S4444_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor16
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_4444_Config); \
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

#define MAKENAME(suffix)        SA8_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kA8_Config);
#define PREAMBLE(state)         const SkPMColor pmColor = state.fPaintPMColor;
#define RETURNDST(src)          SkAlphaMulQ(pmColor, SkAlpha255To256(src))
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
        Filter_32_opaque(x, y, a, b, c, d, &dstColor);  \
        (*dst) = SkPixel32ToPixel16(dstColor);          \
    } while (0)

#define MAKENAME(suffix)        S32_D16 ## suffix
#define DSTSIZE                 16
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_8888_Config); \
                                SkASSERT(state.fBitmap->isOpaque())
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

#define MAKENAME(suffix)        S16_D16 ## suffix
#define DSTSIZE                 16
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kRGB_565_Config)
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

#define MAKENAME(suffix)        SI8_D16 ## suffix
#define DSTSIZE                 16
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kIndex8_Config); \
                                SkASSERT(state.fBitmap->isOpaque())
#define PREAMBLE(state)         const uint16_t* SK_RESTRICT table = state.fBitmap->getColorTable()->lock16BitCache()
#define RETURNDST(src)          table[src]
#define SRC_TO_FILTER(src)      table[src]
#define POSTAMBLE(state)        state.fBitmap->getColorTable()->unlock16BitCache()
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

#define MAKENAME(suffix)        Clamp_S16_D16 ## suffix
#define SRCTYPE                 uint16_t
#define DSTTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kRGB_565_Config)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_shaderproc.h"


#define TILEX_PROCF(fx, max)    (((fx) & 0xFFFF) * ((max) + 1) >> 16)
#define TILEY_PROCF(fy, max)    (((fy) & 0xFFFF) * ((max) + 1) >> 16)
#define TILEX_LOW_BITS(fx, max) ((((fx) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) ((((fy) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)

#define MAKENAME(suffix)        Repeat_S16_D16 ## suffix
#define SRCTYPE                 uint16_t
#define DSTTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kRGB_565_Config)
#define SRC_TO_FILTER(src)      src
#include "SkBitmapProcState_shaderproc.h"


#define TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
#define TILEY_PROCF(fy, max)    SkClampMax((fy) >> 16, max)
#define TILEX_LOW_BITS(fx, max) (((fx) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) (((fy) >> 12) & 0xF)

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d, dst)   Filter_32_opaque(x, y, a, b, c, d, dst)
#define MAKENAME(suffix)        Clamp_SI8_opaque_D32 ## suffix
#define SRCTYPE                 uint8_t
#define DSTTYPE                 uint32_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kIndex8_Config)
#define PREAMBLE(state)         const SkPMColor* SK_RESTRICT table = state.fBitmap->getColorTable()->lockColors()
#define SRC_TO_FILTER(src)      table[src]
#define POSTAMBLE(state)        state.fBitmap->getColorTable()->unlockColors(false)
#include "SkBitmapProcState_shaderproc.h"

///////////////////////////////////////////////////////////////////////////////

static bool valid_for_filtering(unsigned dimension) {
    // for filtering, width and height must fit in 14bits, since we use steal
    // 2 bits from each to store our 4bit subpixel data
    return (dimension & ~0x3FFF) == 0;
}

bool SkBitmapProcState::chooseProcs(const SkMatrix& inv, const SkPaint& paint) {
    if (fOrigBitmap.width() == 0 || fOrigBitmap.height() == 0) {
        return false;
    }

    const SkMatrix* m;
    bool trivial_matrix = (inv.getType() & ~SkMatrix::kTranslate_Mask) == 0;
    bool clamp_clamp = SkShader::kClamp_TileMode == fTileModeX &&
                       SkShader::kClamp_TileMode == fTileModeY;

    if (clamp_clamp || trivial_matrix) {
        m = &inv;
    } else {
        fUnitInvMatrix = inv;
        fUnitInvMatrix.postIDiv(fOrigBitmap.width(), fOrigBitmap.height());
        m = &fUnitInvMatrix;
    }

    fBitmap = &fOrigBitmap;
    if (fOrigBitmap.hasMipMap()) {
        int shift = fOrigBitmap.extractMipLevel(&fMipBitmap,
                                                SkScalarToFixed(m->getScaleX()),
                                                SkScalarToFixed(m->getSkewY()));
        
        if (shift > 0) {
            if (m != &fUnitInvMatrix) {
                fUnitInvMatrix = *m;
                m = &fUnitInvMatrix;
            }

            SkScalar scale = SkFixedToScalar(SK_Fixed1 >> shift);
            fUnitInvMatrix.postScale(scale, scale);
            
            // now point here instead of fOrigBitmap
            fBitmap = &fMipBitmap;
        }
    }

    fInvMatrix      = m;
    fInvProc        = m->getMapXYProc();
    fInvType        = m->getType();
    fInvSx          = SkScalarToFixed(m->getScaleX());
    fInvSxFractionalInt = SkScalarToFractionalInt(m->getScaleX());
    fInvKy          = SkScalarToFixed(m->getSkewY());
    fInvKyFractionalInt = SkScalarToFractionalInt(m->getSkewY());

    fAlphaScale = SkAlpha255To256(paint.getAlpha());

    // pick-up filtering from the paint, but only if the matrix is
    // more complex than identity/translate (i.e. no need to pay the cost
    // of filtering if we're not scaled etc.).
    // note: we explicitly check inv, since m might be scaled due to unitinv
    //       trickery, but we don't want to see that for this test
    fDoFilter = paint.isFilterBitmap() &&
                (inv.getType() > SkMatrix::kTranslate_Mask &&
                 valid_for_filtering(fBitmap->width() | fBitmap->height()));

    fShaderProc32 = NULL;
    fShaderProc16 = NULL;
    fSampleProc32 = NULL;
    fSampleProc16 = NULL;

    fMatrixProc = this->chooseMatrixProc(trivial_matrix);
    if (NULL == fMatrixProc) {
        return false;
    }

    ///////////////////////////////////////////////////////////////////////
    
    int index = 0;
    if (fAlphaScale < 256) {  // note: this distinction is not used for D16
        index |= 1;
    }
    if (fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
        index |= 2;
    }
    if (fDoFilter) {
        index |= 4;
    }
    // bits 3,4,5 encoding the source bitmap format
    switch (fBitmap->config()) {
        case SkBitmap::kARGB_8888_Config:
            index |= 0;
            break;
        case SkBitmap::kRGB_565_Config:
            index |= 8;
            break;
        case SkBitmap::kIndex8_Config:
            index |= 16;
            break;
        case SkBitmap::kARGB_4444_Config:
            index |= 24;
            break;
        case SkBitmap::kA8_Config:
            index |= 32;
            fPaintPMColor = SkPreMultiplyColor(paint.getColor());
            break;
        default:
            return false;
    }

    static const SampleProc32 gSample32[] = {
        S32_opaque_D32_nofilter_DXDY,
        S32_alpha_D32_nofilter_DXDY,
        S32_opaque_D32_nofilter_DX,
        S32_alpha_D32_nofilter_DX,
        S32_opaque_D32_filter_DXDY,
        S32_alpha_D32_filter_DXDY,
        S32_opaque_D32_filter_DX,
        S32_alpha_D32_filter_DX,
        
        S16_opaque_D32_nofilter_DXDY,
        S16_alpha_D32_nofilter_DXDY,
        S16_opaque_D32_nofilter_DX,
        S16_alpha_D32_nofilter_DX,
        S16_opaque_D32_filter_DXDY,
        S16_alpha_D32_filter_DXDY,
        S16_opaque_D32_filter_DX,
        S16_alpha_D32_filter_DX,
        
        SI8_opaque_D32_nofilter_DXDY,
        SI8_alpha_D32_nofilter_DXDY,
        SI8_opaque_D32_nofilter_DX,
        SI8_alpha_D32_nofilter_DX,
        SI8_opaque_D32_filter_DXDY,
        SI8_alpha_D32_filter_DXDY,
        SI8_opaque_D32_filter_DX,
        SI8_alpha_D32_filter_DX,
        
        S4444_opaque_D32_nofilter_DXDY,
        S4444_alpha_D32_nofilter_DXDY,
        S4444_opaque_D32_nofilter_DX,
        S4444_alpha_D32_nofilter_DX,
        S4444_opaque_D32_filter_DXDY,
        S4444_alpha_D32_filter_DXDY,
        S4444_opaque_D32_filter_DX,
        S4444_alpha_D32_filter_DX,
        
        // A8 treats alpha/opauqe the same (equally efficient)
        SA8_alpha_D32_nofilter_DXDY,
        SA8_alpha_D32_nofilter_DXDY,
        SA8_alpha_D32_nofilter_DX,
        SA8_alpha_D32_nofilter_DX,
        SA8_alpha_D32_filter_DXDY,
        SA8_alpha_D32_filter_DXDY,
        SA8_alpha_D32_filter_DX,
        SA8_alpha_D32_filter_DX
    };
    
    static const SampleProc16 gSample16[] = {
        S32_D16_nofilter_DXDY,
        S32_D16_nofilter_DX,
        S32_D16_filter_DXDY,
        S32_D16_filter_DX,
        
        S16_D16_nofilter_DXDY,
        S16_D16_nofilter_DX,
        S16_D16_filter_DXDY,
        S16_D16_filter_DX,
        
        SI8_D16_nofilter_DXDY,
        SI8_D16_nofilter_DX,
        SI8_D16_filter_DXDY,
        SI8_D16_filter_DX,
        
        // Don't support 4444 -> 565
        NULL, NULL, NULL, NULL,
        // Don't support A8 -> 565
        NULL, NULL, NULL, NULL
    };

    fSampleProc32 = gSample32[index];
    index >>= 1;    // shift away any opaque/alpha distinction
    fSampleProc16 = gSample16[index];

    // our special-case shaderprocs
    if (S16_D16_filter_DX == fSampleProc16) {
        if (clamp_clamp) {
            fShaderProc16 = Clamp_S16_D16_filter_DX_shaderproc;
        } else if (SkShader::kRepeat_TileMode == fTileModeX &&
                   SkShader::kRepeat_TileMode == fTileModeY) {
            fShaderProc16 = Repeat_S16_D16_filter_DX_shaderproc;
        }
    } else if (SI8_opaque_D32_filter_DX == fSampleProc32 && clamp_clamp) {
        fShaderProc32 = Clamp_SI8_opaque_D32_filter_DX_shaderproc;
    }

    // see if our platform has any accelerated overrides
    this->platformProcs();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

static void check_scale_nofilter(uint32_t bitmapXY[], int count,
                                 unsigned mx, unsigned my) {
    unsigned y = *bitmapXY++;
    SkASSERT(y < my);
    
    const uint16_t* xptr = reinterpret_cast<const uint16_t*>(bitmapXY);
    for (int i = 0; i < count; ++i) {
        SkASSERT(xptr[i] < mx);
    }
}

static void check_scale_filter(uint32_t bitmapXY[], int count,
                                 unsigned mx, unsigned my) {
    uint32_t YY = *bitmapXY++;
    unsigned y0 = YY >> 18;
    unsigned y1 = YY & 0x3FFF;    
    SkASSERT(y0 < my);
    SkASSERT(y1 < my);
    
    for (int i = 0; i < count; ++i) {
        uint32_t XX = bitmapXY[i];
        unsigned x0 = XX >> 18;
        unsigned x1 = XX & 0x3FFF;
        SkASSERT(x0 < mx);
        SkASSERT(x1 < mx);
    }
}

static void check_affine_nofilter(uint32_t bitmapXY[], int count,
                                 unsigned mx, unsigned my) {
    for (int i = 0; i < count; ++i) {
        uint32_t XY = bitmapXY[i];
        unsigned x = XY & 0xFFFF;
        unsigned y = XY >> 16;
        SkASSERT(x < mx);
        SkASSERT(y < my);
    }
}

static void check_affine_filter(uint32_t bitmapXY[], int count,
                                 unsigned mx, unsigned my) {
    for (int i = 0; i < count; ++i) {
        uint32_t YY = *bitmapXY++;
        unsigned y0 = YY >> 18;
        unsigned y1 = YY & 0x3FFF;
        SkASSERT(y0 < my);
        SkASSERT(y1 < my);

        uint32_t XX = *bitmapXY++;
        unsigned x0 = XX >> 18;
        unsigned x1 = XX & 0x3FFF;
        SkASSERT(x0 < mx);
        SkASSERT(x1 < mx);
    }
}

void SkBitmapProcState::DebugMatrixProc(const SkBitmapProcState& state,
                                        uint32_t bitmapXY[], int count,
                                        int x, int y) {
    SkASSERT(bitmapXY);
    SkASSERT(count > 0);

    state.fMatrixProc(state, bitmapXY, count, x, y);

    void (*proc)(uint32_t bitmapXY[], int count, unsigned mx, unsigned my);

    // There are four formats possible:
    //  scale -vs- affine
    //  filter -vs- nofilter
    if (state.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
        proc = state.fDoFilter ? check_scale_filter : check_scale_nofilter;
    } else {
        proc = state.fDoFilter ? check_affine_filter : check_affine_nofilter;
    }
    proc(bitmapXY, count, state.fBitmap->width(), state.fBitmap->height());
}

SkBitmapProcState::MatrixProc SkBitmapProcState::getMatrixProc() const {
    return DebugMatrixProc;
}

#endif

///////////////////////////////////////////////////////////////////////////////
/*
    The storage requirements for the different matrix procs are as follows,
    where each X or Y is 2 bytes, and N is the number of pixels/elements:
 
    scale/translate     nofilter      Y(4bytes) + N * X
    affine/perspective  nofilter      N * (X Y)
    scale/translate     filter        Y Y + N * (X X)
    affine/perspective  filter        N * (Y Y X X)
 */
int SkBitmapProcState::maxCountForBufferSize(size_t bufferSize) const {
    int32_t size = static_cast<int32_t>(bufferSize);

    size &= ~3; // only care about 4-byte aligned chunks
    if (fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
        size -= 4;   // the shared Y (or YY) coordinate
        if (size < 0) {
            size = 0;
        }
        size >>= 1;
    } else {
        size >>= 2;
    }

    if (fDoFilter) {
        size >>= 1;
    }

    return size;
}

