
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

#define   NAME_WRAP(x)  x
#include "SkBitmapProcState_filter.h"
#include "SkBitmapProcState_procs.h"

///////////////////////////////////////////////////////////////////////////////

/**
 *  For the purposes of drawing bitmaps, if a matrix is "almost" translate
 *  go ahead and treat it as if it were, so that subsequent code can go fast.
 */
static bool just_trans_clamp(const SkMatrix& matrix, const SkBitmap& bitmap) {
    SkMatrix::TypeMask mask = matrix.getType();

    if (mask & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
        return false;
    }
    if (mask & SkMatrix::kScale_Mask) {
        SkScalar sx = matrix[SkMatrix::kMScaleX];
        SkScalar sy = matrix[SkMatrix::kMScaleY];
        int w = bitmap.width();
        int h = bitmap.height();
        int sw = SkScalarRound(SkScalarMul(sx, SkIntToScalar(w)));
        int sh = SkScalarRound(SkScalarMul(sy, SkIntToScalar(h)));
        return sw == w && sh == h;
    }
    // if we got here, we're either kTranslate_Mask or identity
    return true;
}

static bool just_trans_general(const SkMatrix& matrix) {
    SkMatrix::TypeMask mask = matrix.getType();

    if (mask & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
        return false;
    }
    if (mask & SkMatrix::kScale_Mask) {
        const SkScalar tol = SK_Scalar1 / 32768;

        if (!SkScalarNearlyZero(matrix[SkMatrix::kMScaleX] - SK_Scalar1, tol)) {
            return false;
        }
        if (!SkScalarNearlyZero(matrix[SkMatrix::kMScaleY] - SK_Scalar1, tol)) {
            return false;
        }
    }
    // if we got here, treat us as either kTranslate_Mask or identity
    return true;
}

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

    // wack our matrix to exactly no-scale, if we're really close to begin with
    {
        bool fixupMatrix = clamp_clamp ?
        just_trans_clamp(*m, *fBitmap) : just_trans_general(*m);
        if (fixupMatrix) {
            // If we can be treated just like translate, construct that inverse
            // such that we landed in the proper place. Given that m may have
            // some slight scale, we have to invert it to compute this new
            // matrix.
            SkMatrix forward;
            if (m->invert(&forward)) {
                SkScalar tx = -SkScalarRoundToScalar(forward.getTranslateX());
                SkScalar ty = -SkScalarRoundToScalar(forward.getTranslateY());
                fUnitInvMatrix.setTranslate(tx, ty);
                m = &fUnitInvMatrix;
                // now the following code will sniff m, and decide to take the
                // fast case (since m is purely translate).
            }
        }
    }

    // Below this point, we should never refer to the inv parameter, since we
    // may be using a munged version for "our" inverse.

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
                (fInvType > SkMatrix::kTranslate_Mask &&
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

        // A8 treats alpha/opaque the same (equally efficient)
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

    if (NULL == fShaderProc32) {
        fShaderProc32 = this->chooseShaderProc32();
    }

    // see if our platform has any accelerated overrides
    this->platformProcs();
    return true;
}

static void Clamp_S32_D32_nofilter_trans_shaderproc(const SkBitmapProcState& s,
                                                    int x, int y,
                                                    SkPMColor* SK_RESTRICT colors,
                                                    int count) {
    SkASSERT(((s.fInvType & ~SkMatrix::kTranslate_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(!s.fDoFilter);

    const int maxX = s.fBitmap->width() - 1;
    const int maxY = s.fBitmap->height() - 1;
    int ix = s.fFilterOneX + x;
    int iy = SkClampMax(s.fFilterOneY + y, maxY);
#ifdef SK_DEBUG
    {
        SkPoint pt;
        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                   SkIntToScalar(y) + SK_ScalarHalf, &pt);
        int iy2 = SkClampMax(SkScalarFloorToInt(pt.fY), maxY);
        int ix2 = SkScalarFloorToInt(pt.fX);

        SkASSERT(iy == iy2);
        SkASSERT(ix == ix2);
    }
#endif
    const SkPMColor* row = s.fBitmap->getAddr32(0, iy);

    // clamp to the left
    if (ix < 0) {
        int n = SkMin32(-ix, count);
        sk_memset32(colors, row[0], n);
        count -= n;
        if (0 == count) {
            return;
        }
        colors += n;
        SkASSERT(-ix == n);
        ix = 0;
    }
    // copy the middle
    if (ix <= maxX) {
        int n = SkMin32(maxX - ix + 1, count);
        memcpy(colors, row + ix, n * sizeof(SkPMColor));
        count -= n;
        if (0 == count) {
            return;
        }
        colors += n;
    }
    SkASSERT(count > 0);
    // clamp to the right
    sk_memset32(colors, row[maxX], count);
}

static inline int sk_int_mod(int x, int n) {
    SkASSERT(n > 0);
    if ((unsigned)x >= (unsigned)n) {
        if (x < 0) {
            x = n + ~(~x % n);
        } else {
            x = x % n;
        }
    }
    return x;
}

static inline int sk_int_mirror(int x, int n) {
    x = sk_int_mod(x, 2 * n);
    if (x >= n) {
        x = n + ~(x - n);
    }
    return x;
}

static void Repeat_S32_D32_nofilter_trans_shaderproc(const SkBitmapProcState& s,
                                                     int x, int y,
                                                     SkPMColor* SK_RESTRICT colors,
                                                     int count) {
    SkASSERT(((s.fInvType & ~SkMatrix::kTranslate_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(!s.fDoFilter);

    const int stopX = s.fBitmap->width();
    const int stopY = s.fBitmap->height();
    int ix = s.fFilterOneX + x;
    int iy = sk_int_mod(s.fFilterOneY + y, stopY);
#ifdef SK_DEBUG
    {
        SkPoint pt;
        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                   SkIntToScalar(y) + SK_ScalarHalf, &pt);
        int iy2 = sk_int_mod(SkScalarFloorToInt(pt.fY), stopY);
        int ix2 = SkScalarFloorToInt(pt.fX);

        SkASSERT(iy == iy2);
        SkASSERT(ix == ix2);
    }
#endif
    const SkPMColor* row = s.fBitmap->getAddr32(0, iy);

    ix = sk_int_mod(ix, stopX);
    for (;;) {
        int n = SkMin32(stopX - ix, count);
        memcpy(colors, row + ix, n * sizeof(SkPMColor));
        count -= n;
        if (0 == count) {
            return;
        }
        colors += n;
        ix = 0;
    }
}

static void S32_D32_constX_shaderproc(const SkBitmapProcState& s,
                                      int x, int y,
                                      SkPMColor* SK_RESTRICT colors,
                                      int count) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(1 == s.fBitmap->width());

    int iY0;
    int iY1   SK_INIT_TO_AVOID_WARNING;
    int iSubY SK_INIT_TO_AVOID_WARNING;

    if (s.fDoFilter) {
        SkBitmapProcState::MatrixProc mproc = s.getMatrixProc();
        uint32_t xy[2];

        mproc(s, xy, 1, x, y);

        iY0 = xy[0] >> 18;
        iY1 = xy[0] & 0x3FFF;
        iSubY = (xy[0] >> 14) & 0xF;
    } else {
        int yTemp;

        if (s.fInvType > SkMatrix::kTranslate_Mask) {
            SkPoint pt;
            s.fInvProc(*s.fInvMatrix,
                       SkIntToScalar(x) + SK_ScalarHalf,
                       SkIntToScalar(y) + SK_ScalarHalf,
                       &pt);
            // When the matrix has a scale component the setup code in
            // chooseProcs multiples the inverse matrix by the inverse of the
            // bitmap's width and height. Since this method is going to do
            // its own tiling and sampling we need to undo that here.
            if (SkShader::kClamp_TileMode != s.fTileModeX ||
                SkShader::kClamp_TileMode != s.fTileModeY) {
                yTemp = SkScalarFloorToInt(pt.fY * s.fBitmap->height());
            } else {
                yTemp = SkScalarFloorToInt(pt.fY);
            }
        } else {
            yTemp = s.fFilterOneY + y;
        }

        const int stopY = s.fBitmap->height();
        switch (s.fTileModeY) {
            case SkShader::kClamp_TileMode:
                iY0 = SkClampMax(yTemp, stopY-1);
                break;
            case SkShader::kRepeat_TileMode:
                iY0 = sk_int_mod(yTemp, stopY);
                break;
            case SkShader::kMirror_TileMode:
            default:
                iY0 = sk_int_mirror(yTemp, stopY);
                break;
        }

#ifdef SK_DEBUG
        {
            SkPoint pt;
            s.fInvProc(*s.fInvMatrix,
                       SkIntToScalar(x) + SK_ScalarHalf,
                       SkIntToScalar(y) + SK_ScalarHalf,
                       &pt);
            if (s.fInvType > SkMatrix::kTranslate_Mask &&
                (SkShader::kClamp_TileMode != s.fTileModeX ||
                 SkShader::kClamp_TileMode != s.fTileModeY)) {
                pt.fY *= s.fBitmap->height();
            }
            int iY2;

            switch (s.fTileModeY) {
            case SkShader::kClamp_TileMode:
                iY2 = SkClampMax(SkScalarFloorToInt(pt.fY), stopY-1);
                break;
            case SkShader::kRepeat_TileMode:
                iY2 = sk_int_mod(SkScalarFloorToInt(pt.fY), stopY);
                break;
            case SkShader::kMirror_TileMode:
            default:
                iY2 = sk_int_mirror(SkScalarFloorToInt(pt.fY), stopY);
                break;
            }

            SkASSERT(iY0 == iY2);
        }
#endif
    }

    const SkPMColor* row0 = s.fBitmap->getAddr32(0, iY0);
    SkPMColor color;

    if (s.fDoFilter) {
        const SkPMColor* row1 = s.fBitmap->getAddr32(0, iY1);

        if (s.fAlphaScale < 256) {
            Filter_32_alpha(iSubY, *row0, *row1, &color, s.fAlphaScale);
        } else {
            Filter_32_opaque(iSubY, *row0, *row1, &color);
        }
    } else {
        if (s.fAlphaScale < 256) {
            color = SkAlphaMulQ(*row0, s.fAlphaScale);
        } else {
            color = *row0;
        }
    }

    sk_memset32(colors, color, count);
}

static void DoNothing_shaderproc(const SkBitmapProcState&, int x, int y,
                                 SkPMColor* SK_RESTRICT colors, int count) {
    // if we get called, the matrix is too tricky, so we just draw nothing
    sk_memset32(colors, 0, count);
}

bool SkBitmapProcState::setupForTranslate() {
    SkPoint pt;
    fInvProc(*fInvMatrix, SK_ScalarHalf, SK_ScalarHalf, &pt);

    /*
     *  if the translate is larger than our ints, we can get random results, or
     *  worse, we might get 0x80000000, which wreaks havoc on us, since we can't
     *  negate it.
     */
    const SkScalar too_big = SkIntToScalar(1 << 30);
    if (SkScalarAbs(pt.fX) > too_big || SkScalarAbs(pt.fY) > too_big) {
        return false;
    }

    // Since we know we're not filtered, we re-purpose these fields allow
    // us to go from device -> src coordinates w/ just an integer add,
    // rather than running through the inverse-matrix
    fFilterOneX = SkScalarFloorToInt(pt.fX);
    fFilterOneY = SkScalarFloorToInt(pt.fY);
    return true;
}

SkBitmapProcState::ShaderProc32 SkBitmapProcState::chooseShaderProc32() {

    if (SkBitmap::kARGB_8888_Config != fBitmap->config()) {
        return NULL;
    }

    static const unsigned kMask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;

    if (1 == fBitmap->width() && 0 == (fInvType & ~kMask)) {
        if (!fDoFilter && fInvType <= SkMatrix::kTranslate_Mask && !this->setupForTranslate()) {
            return DoNothing_shaderproc;
        }
        return S32_D32_constX_shaderproc;
    }

    if (fAlphaScale < 256) {
        return NULL;
    }
    if (fInvType > SkMatrix::kTranslate_Mask) {
        return NULL;
    }
    if (fDoFilter) {
        return NULL;
    }

    SkShader::TileMode tx = (SkShader::TileMode)fTileModeX;
    SkShader::TileMode ty = (SkShader::TileMode)fTileModeY;

    if (SkShader::kClamp_TileMode == tx && SkShader::kClamp_TileMode == ty) {
        if (this->setupForTranslate()) {
            return Clamp_S32_D32_nofilter_trans_shaderproc;
        }
        return DoNothing_shaderproc;
    }
    if (SkShader::kRepeat_TileMode == tx && SkShader::kRepeat_TileMode == ty) {
        if (this->setupForTranslate()) {
            return Repeat_S32_D32_nofilter_trans_shaderproc;
        }
        return DoNothing_shaderproc;
    }
    return NULL;
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
