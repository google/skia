
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBitmapProcState.h"
#include "SkColorPriv.h"
#include "SkFilterProc.h"
#include "SkPaint.h"
#include "SkShader.h"   // for tilemodes
#include "SkUtilsArm.h"

#if !SK_ARM_NEON_IS_NONE
// These are defined in src/opts/SkBitmapProcState_arm_neon.cpp
extern const SkBitmapProcState::SampleProc16 gSkBitmapProcStateSample16_neon[];
extern const SkBitmapProcState::SampleProc32 gSkBitmapProcStateSample32_neon[];
extern void  S16_D16_filter_DX_neon(const SkBitmapProcState&, const uint32_t*, int, uint16_t*);
extern void  Clamp_S16_D16_filter_DX_shaderproc_neon(const SkBitmapProcState&, int, int, uint16_t*, int);
extern void  Repeat_S16_D16_filter_DX_shaderproc_neon(const SkBitmapProcState&, int, int, uint16_t*, int);
extern void  SI8_opaque_D32_filter_DX_neon(const SkBitmapProcState&, const uint32_t*, int, SkPMColor*);
extern void  SI8_opaque_D32_filter_DX_shaderproc_neon(const SkBitmapProcState&, int, int, uint32_t*, int);
extern void  Clamp_SI8_opaque_D32_filter_DX_shaderproc_neon(const SkBitmapProcState&, int, int, uint32_t*, int);
#endif

#if !SK_ARM_NEON_IS_ALWAYS
#define   NAME_WRAP(x)  x
#include "SkBitmapProcState_filter.h"
#include "SkBitmapProcState_procs.h"
#endif

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

#if !SK_ARM_NEON_IS_ALWAYS
    static const SampleProc32 gSkBitmapProcStateSample32[] = {
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

    static const SampleProc16 gSkBitmapProcStateSample16[] = {
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
#endif

    fSampleProc32 = SK_ARM_NEON_WRAP(gSkBitmapProcStateSample32)[index];
    index >>= 1;    // shift away any opaque/alpha distinction
    fSampleProc16 = SK_ARM_NEON_WRAP(gSkBitmapProcStateSample16)[index];

    // our special-case shaderprocs
    if (SK_ARM_NEON_WRAP(S16_D16_filter_DX) == fSampleProc16) {
        if (clamp_clamp) {
            fShaderProc16 = SK_ARM_NEON_WRAP(Clamp_S16_D16_filter_DX_shaderproc);
        } else if (SkShader::kRepeat_TileMode == fTileModeX &&
                   SkShader::kRepeat_TileMode == fTileModeY) {
            fShaderProc16 = SK_ARM_NEON_WRAP(Repeat_S16_D16_filter_DX_shaderproc);
        }
    } else if (SK_ARM_NEON_WRAP(SI8_opaque_D32_filter_DX) == fSampleProc32 && clamp_clamp) {
        fShaderProc32 = SK_ARM_NEON_WRAP(Clamp_SI8_opaque_D32_filter_DX_shaderproc);
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

