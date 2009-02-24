#include "SkBitmapProcState.h"
#include "SkColorPriv.h"
#include "SkFilterProc.h"
#include "SkPaint.h"
#include "SkShader.h"   // for tilemodes

#ifdef SK_CPU_BENDIAN
    #define UNPACK_PRIMARY_SHORT(packed)    ((uint32_t)(packed) >> 16)
    #define UNPACK_SECONDARY_SHORT(packed)  ((packed) & 0xFFFF)
#else
    #define UNPACK_PRIMARY_SHORT(packed)    ((packed) & 0xFFFF)
    #define UNPACK_SECONDARY_SHORT(packed)  ((uint32_t)(packed) >> 16)
#endif

static inline SkPMColor Filter_32(unsigned x, unsigned y,
                                  SkPMColor a00, SkPMColor a01,
                                  SkPMColor a10, SkPMColor a11) {
    SkASSERT((unsigned)x <= 0xF);
    SkASSERT((unsigned)y <= 0xF);
    
    int xy = x * y;
    uint32_t mask = gMask_00FF00FF; //0xFF00FF;
    
    int scale = 256 - 16*y - 16*x + xy;
    uint32_t lo = (a00 & mask) * scale;
    uint32_t hi = ((a00 >> 8) & mask) * scale;
    
    scale = 16*x - xy;
    lo += (a01 & mask) * scale;
    hi += ((a01 >> 8) & mask) * scale;
    
    scale = 16*y - xy;
    lo += (a10 & mask) * scale;
    hi += ((a10 >> 8) & mask) * scale;
    
    lo += (a11 & mask) * xy;
    hi += ((a11 >> 8) & mask) * xy;
    
    return ((lo >> 8) & mask) | (hi & ~mask);
}

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

#define FILTER_PROC(x, y, a, b, c, d)   Filter_32(x, y, a, b, c, d)

#define MAKENAME(suffix)        S32_opaque_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_8888_Config); \
                                SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          src
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        c
#include "SkBitmapProcState_sample.h"

#define MAKENAME(suffix)        S32_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_8888_Config); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned scale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(src, scale)
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        SkAlphaMulQ(c, scale)
#include "SkBitmapProcState_sample.h"

// SRC == 565

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d)   Filter_565_Expanded(x, y, a, b, c, d)

#define MAKENAME(suffix)        S16_opaque_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kRGB_565_Config); \
                                SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          SkPixel16ToPixel32(src)
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        SkExpanded_565_To_PMColor(c)
#include "SkBitmapProcState_sample.h"

#define MAKENAME(suffix)        S16_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kRGB_565_Config); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned scale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(SkPixel16ToPixel32(src), scale)
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        SkAlphaMulQ(SkExpanded_565_To_PMColor(c), scale)
#include "SkBitmapProcState_sample.h"

// SRC == Index8

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d)   Filter_32(x, y, a, b, c, d)

#define MAKENAME(suffix)        SI8_opaque_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kIndex8_Config); \
                                SkASSERT(state.fAlphaScale == 256)
#define PREAMBLE(state)         const SkPMColor* SK_RESTRICT table = state.fBitmap->getColorTable()->lockColors()
#define RETURNDST(src)          table[src]
#define SRC_TO_FILTER(src)      table[src]
#define FILTER_TO_DST(c)        c
#define POSTAMBLE(state)        state.fBitmap->getColorTable()->unlockColors(false)
#include "SkBitmapProcState_sample.h"

#define MAKENAME(suffix)        SI8_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kIndex8_Config); \
                                SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned scale = state.fAlphaScale; \
                                const SkPMColor* SK_RESTRICT table = state.fBitmap->getColorTable()->lockColors()
#define RETURNDST(src)          SkAlphaMulQ(table[src], scale)
#define SRC_TO_FILTER(src)      table[src]
#define FILTER_TO_DST(c)        SkAlphaMulQ(c, scale)
#define POSTAMBLE(state)        state.fBitmap->getColorTable()->unlockColors(false)
#include "SkBitmapProcState_sample.h"

// SRC == 4444

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d)   Filter_4444_D32(x, y, a, b, c, d)

#define MAKENAME(suffix)        S4444_opaque_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor16
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_4444_Config); \
SkASSERT(state.fAlphaScale == 256)
#define RETURNDST(src)          SkPixel4444ToPixel32(src)
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        c
#include "SkBitmapProcState_sample.h"

#define MAKENAME(suffix)        S4444_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 SkPMColor16
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_4444_Config); \
SkASSERT(state.fAlphaScale < 256)
#define PREAMBLE(state)         unsigned scale = state.fAlphaScale
#define RETURNDST(src)          SkAlphaMulQ(SkPixel4444ToPixel32(src), scale)
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        SkAlphaMulQ(c, scale)
#include "SkBitmapProcState_sample.h"

// SRC == A8

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d)   Filter_8(x, y, a, b, c, d)

#define MAKENAME(suffix)        SA8_alpha_D32 ## suffix
#define DSTSIZE                 32
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kA8_Config); \
                                SkASSERT(state.fAlphaScale == 256)
#define PREAMBLE(state)         const SkPMColor pmColor = state.fPaintPMColor;
#define RETURNDST(src)          SkAlphaMulQ(pmColor, SkAlpha255To256(src))
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        SkAlphaMulQ(pmColor, SkAlpha255To256(c))
#include "SkBitmapProcState_sample.h"

/*****************************************************************************
 *
 *  D16 functions
 *
 */

// SRC == 8888

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d)   Filter_32(x, y, a, b, c, d)

#define MAKENAME(suffix)        S32_D16 ## suffix
#define DSTSIZE                 16
#define SRCTYPE                 SkPMColor
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kARGB_8888_Config); \
                                SkASSERT(state.fBitmap->isOpaque())
#define RETURNDST(src)          SkPixel32ToPixel16(src)
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        SkPixel32ToPixel16(c)
#include "SkBitmapProcState_sample.h"

// SRC == 565

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d)   Filter_565_Expanded(x, y, a, b, c, d)

#define MAKENAME(suffix)        S16_D16 ## suffix
#define DSTSIZE                 16
#define SRCTYPE                 uint16_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kRGB_565_Config)
#define RETURNDST(src)          src
#define SRC_TO_FILTER(src)      src
#define FILTER_TO_DST(c)        SkCompact_rgb_16((c) >> 5)
#include "SkBitmapProcState_sample.h"

// SRC == Index8

#undef FILTER_PROC
#define FILTER_PROC(x, y, a, b, c, d)   Filter_565_Expanded(x, y, a, b, c, d)

#define MAKENAME(suffix)        SI8_D16 ## suffix
#define DSTSIZE                 16
#define SRCTYPE                 uint8_t
#define CHECKSTATE(state)       SkASSERT(state.fBitmap->config() == SkBitmap::kIndex8_Config); \
                                SkASSERT(state.fBitmap->isOpaque())
#define PREAMBLE(state)         const uint16_t* SK_RESTRICT table = state.fBitmap->getColorTable()->lock16BitCache()
#define RETURNDST(src)          table[src]
#define SRC_TO_FILTER(src)      table[src]
#define FILTER_TO_DST(c)        SkCompact_rgb_16(c >> 5)
#define POSTAMBLE(state)        state.fBitmap->getColorTable()->unlock16BitCache()
#include "SkBitmapProcState_sample.h"

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
    
    if (SkShader::kClamp_TileMode == fTileModeX &&
            SkShader::kClamp_TileMode == fTileModeY) {
        m = &inv;
    } else {
        fUnitInvMatrix = inv;
        fUnitInvMatrix.postIDiv(fOrigBitmap.width(), fOrigBitmap.height());
        m = &fUnitInvMatrix;
    }
    
    fBitmap = &fOrigBitmap;
#ifdef SK_SUPPORT_MIPMAP
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
#endif

    fInvMatrix      = m;
    fInvProc        = m->getMapXYProc();
    fInvType        = m->getType();
    fInvSx          = SkScalarToFixed(m->getScaleX());
    fInvSy          = SkScalarToFixed(m->getScaleY());
    fInvKy          = SkScalarToFixed(m->getSkewY());

    fAlphaScale = SkAlpha255To256(paint.getAlpha());

    // pick-up filtering from the paint, but only if the matrix is
    // more complex than identity/translate (i.e. no need to pay the cost
    // of filtering if we're not scaled etc.).
    // note: we explicitly check inv, since m might be scaled due to unitinv
    //       trickery, but we don't want to see that for this test
    fDoFilter = paint.isFilterBitmap() &&
                (inv.getType() > SkMatrix::kTranslate_Mask &&
                 valid_for_filtering(fBitmap->width() | fBitmap->height()));

    fMatrixProc = this->chooseMatrixProc();
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

    return true;
}

