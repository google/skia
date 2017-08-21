/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
#include "SkBitmapController.h"
#include "SkBitmapProcState.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkShader.h"   // for tilemodes
#include "SkUtilsArm.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkImageEncoder.h"
#include "SkResourceCache.h"

#if defined(SK_ARM_HAS_NEON)
// These are defined in src/opts/SkBitmapProcState_arm_neon.cpp
extern const SkBitmapProcState::SampleProc32 gSkBitmapProcStateSample32_neon[];
#endif

extern void Clamp_S32_opaque_D32_nofilter_DX_shaderproc(const void*, int, int, uint32_t*, int);

#define   NAME_WRAP(x)  x
#include "SkBitmapProcState_filter.h"
#include "SkBitmapProcState_procs.h"

SkBitmapProcInfo::SkBitmapProcInfo(const SkBitmapProvider& provider,
                                   SkShader::TileMode tmx, SkShader::TileMode tmy)
    : fProvider(provider)
    , fTileModeX(tmx)
    , fTileModeY(tmy)
    , fBMState(nullptr)
{}

SkBitmapProcInfo::~SkBitmapProcInfo() {
    SkInPlaceDeleteCheck(fBMState, fBMStateStorage.get());
}

///////////////////////////////////////////////////////////////////////////////

// true iff the matrix has a scale and no more than an optional translate.
static bool matrix_only_scale_translate(const SkMatrix& m) {
    return (m.getType() & ~SkMatrix::kTranslate_Mask) == SkMatrix::kScale_Mask;
}

/**
 *  For the purposes of drawing bitmaps, if a matrix is "almost" translate
 *  go ahead and treat it as if it were, so that subsequent code can go fast.
 */
static bool just_trans_general(const SkMatrix& matrix) {
    SkASSERT(matrix_only_scale_translate(matrix));

    const SkScalar tol = SK_Scalar1 / 32768;

    return SkScalarNearlyZero(matrix[SkMatrix::kMScaleX] - SK_Scalar1, tol)
        && SkScalarNearlyZero(matrix[SkMatrix::kMScaleY] - SK_Scalar1, tol);
}

/**
 *  Determine if the matrix can be treated as integral-only-translate,
 *  for the purpose of filtering.
 */
static bool just_trans_integral(const SkMatrix& m) {
#ifdef SK_SUPPORT_LEGACY_BILERP
    return false;
#else
    static constexpr SkScalar tol = SK_Scalar1 / 256;

    return m.getType() <= SkMatrix::kTranslate_Mask
        && SkScalarNearlyEqual(m.getTranslateX(), SkScalarRoundToScalar(m.getTranslateX()), tol)
        && SkScalarNearlyEqual(m.getTranslateY(), SkScalarRoundToScalar(m.getTranslateY()), tol);
#endif
}

static bool valid_for_filtering(unsigned dimension) {
    // for filtering, width and height must fit in 14bits, since we use steal
    // 2 bits from each to store our 4bit subpixel data
    return (dimension & ~0x3FFF) == 0;
}

bool SkBitmapProcInfo::init(const SkMatrix& inv, const SkPaint& paint) {
    SkASSERT(!inv.hasPerspective());

    fPixmap.reset();
    fInvMatrix = inv;
    fFilterQuality = paint.getFilterQuality();

    SkDefaultBitmapController controller;
    fBMState = controller.requestBitmap(fProvider, inv, paint.getFilterQuality(),
                                        fBMStateStorage.get(), fBMStateStorage.size());
    // Note : we allow the controller to return an empty (zero-dimension) result. Should we?
    if (nullptr == fBMState || fBMState->pixmap().info().isEmpty()) {
        return false;
    }
    fPixmap = fBMState->pixmap();
    fInvMatrix = fBMState->invMatrix();
    fRealInvMatrix = fBMState->invMatrix();
    fPaintColor = paint.getColor();
    fFilterQuality = fBMState->quality();
    SkASSERT(fFilterQuality <= kLow_SkFilterQuality);
    SkASSERT(fPixmap.addr());

#ifdef SK_SUPPORT_LEGACY_BILERP2
    bool integral_translate_only = false;
#else
    bool integral_translate_only = just_trans_integral(fInvMatrix);
#endif
    if (!integral_translate_only) {
        // Most of the scanline procs deal with "unit" texture coordinates, as this
        // makes it easy to perform tiling modes (repeat = (x & 0xFFFF)). To generate
        // those, we divide the matrix by its dimensions here.
        //
        // We don't do this if we're either trivial (can ignore the matrix) or clamping
        // in both X and Y since clamping to width,height is just as easy as to 0xFFFF.

        if (fTileModeX != SkShader::kClamp_TileMode ||
            fTileModeY != SkShader::kClamp_TileMode) {
            fInvMatrix.postIDiv(fPixmap.width(), fPixmap.height());
        }

        // Now that all possible changes to the matrix have taken place, check
        // to see if we're really close to a no-scale matrix.  If so, explicitly
        // set it to be so.  Subsequent code may inspect this matrix to choose
        // a faster path in this case.

        // This code will only execute if the matrix has some scale component;
        // if it's already pure translate then we won't do this inversion.

        if (matrix_only_scale_translate(fInvMatrix)) {
            SkMatrix forward;
            if (fInvMatrix.invert(&forward) && just_trans_general(forward)) {
                fInvMatrix.setTranslate(-forward.getTranslateX(), -forward.getTranslateY());
            }
        }

        // Recompute the flag after matrix adjustments.
        integral_translate_only = just_trans_integral(fInvMatrix);
    }

    fInvType = fInvMatrix.getType();

    if (kLow_SkFilterQuality == fFilterQuality &&
        (!valid_for_filtering(fPixmap.width() | fPixmap.height()) ||
         integral_translate_only)) {
        fFilterQuality = kNone_SkFilterQuality;
    }

    return true;
}

/*
 *  Analyze filter-quality and matrix, and decide how to implement that.
 *
 *  In general, we cascade down the request level [ High ... None ]
 *  - for a given level, if we can fulfill it, fine, else
 *    - else we downgrade to the next lower level and try again.
 *  We can always fulfill requests for Low and None
 *  - sometimes we will "ignore" Low and give None, but this is likely a legacy perf hack
 *    and may be removed.
 */
bool SkBitmapProcState::chooseProcs() {
    fInvProc            = fInvMatrix.getMapXYProc();
    fInvSx              = SkScalarToFixed(fInvMatrix.getScaleX());
    fInvSxFractionalInt = SkScalarToFractionalInt(fInvMatrix.getScaleX());
    fInvKy              = SkScalarToFixed(fInvMatrix.getSkewY());
    fInvKyFractionalInt = SkScalarToFractionalInt(fInvMatrix.getSkewY());

    fAlphaScale = SkAlpha255To256(SkColorGetA(fPaintColor));

    fShaderProc32 = nullptr;
    fShaderProc16 = nullptr;
    fSampleProc32 = nullptr;

    const bool trivialMatrix = (fInvMatrix.getType() & ~SkMatrix::kTranslate_Mask) == 0;
    const bool clampClamp = SkShader::kClamp_TileMode == fTileModeX &&
                            SkShader::kClamp_TileMode == fTileModeY;

    return this->chooseScanlineProcs(trivialMatrix, clampClamp);
}

bool SkBitmapProcState::chooseScanlineProcs(bool trivialMatrix, bool clampClamp) {
    SkASSERT(fPixmap.colorType() == kN32_SkColorType);

    fMatrixProc = this->chooseMatrixProc(trivialMatrix);
    // TODO(dominikg): SkASSERT(fMatrixProc) instead? chooseMatrixProc never returns nullptr.
    if (nullptr == fMatrixProc) {
        return false;
    }

    const SkAlphaType at = fPixmap.alphaType();
    if (kPremul_SkAlphaType != at && kOpaque_SkAlphaType != at) {
        return false;
    }

    // No need to do this if we're doing HQ sampling; if filter quality is
    // still set to HQ by the time we get here, then we must have installed
    // the shader procs above and can skip all this.

    if (fFilterQuality < kHigh_SkFilterQuality) {
        int index = 0;
        if (fAlphaScale < 256) {  // note: this distinction is not used for D16
            index |= 1;
        }
        if (fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
            index |= 2;
        }
        if (fFilterQuality > kNone_SkFilterQuality) {
            index |= 4;
        }

#if !defined(SK_ARM_HAS_NEON)
        static const SampleProc32 gSkBitmapProcStateSample32[] = {
            S32_opaque_D32_nofilter_DXDY,
            S32_alpha_D32_nofilter_DXDY,
            S32_opaque_D32_nofilter_DX,
            S32_alpha_D32_nofilter_DX,
            S32_opaque_D32_filter_DXDY,
            S32_alpha_D32_filter_DXDY,
            S32_opaque_D32_filter_DX,
            S32_alpha_D32_filter_DX,
        };
#endif

        fSampleProc32 = SK_ARM_NEON_WRAP(gSkBitmapProcStateSample32)[index];

        // our special-case shaderprocs
        if (S32_opaque_D32_nofilter_DX == fSampleProc32 && clampClamp) {
            fShaderProc32 = Clamp_S32_opaque_D32_nofilter_DX_shaderproc;
        }

        if (nullptr == fShaderProc32) {
            fShaderProc32 = this->chooseShaderProc32();
        }
    }

    // see if our platform has any accelerated overrides
    this->platformProcs();

    return true;
}

static void Clamp_S32_D32_nofilter_trans_shaderproc(const void* sIn,
                                                    int x, int y,
                                                    SkPMColor* SK_RESTRICT colors,
                                                    int count) {
    const SkBitmapProcState& s = *static_cast<const SkBitmapProcState*>(sIn);
    SkASSERT(((s.fInvType & ~SkMatrix::kTranslate_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(kNone_SkFilterQuality == s.fFilterQuality);

    const int maxX = s.fPixmap.width() - 1;
    const int maxY = s.fPixmap.height() - 1;
    int ix = s.fFilterOneX + x;
    int iy = SkClampMax(s.fFilterOneY + y, maxY);
    const SkPMColor* row = s.fPixmap.addr32(0, iy);

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

static void Repeat_S32_D32_nofilter_trans_shaderproc(const void* sIn,
                                                     int x, int y,
                                                     SkPMColor* SK_RESTRICT colors,
                                                     int count) {
    const SkBitmapProcState& s = *static_cast<const SkBitmapProcState*>(sIn);
    SkASSERT(((s.fInvType & ~SkMatrix::kTranslate_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(kNone_SkFilterQuality == s.fFilterQuality);

    const int stopX = s.fPixmap.width();
    const int stopY = s.fPixmap.height();
    int ix = s.fFilterOneX + x;
    int iy = sk_int_mod(s.fFilterOneY + y, stopY);
    const SkPMColor* row = s.fPixmap.addr32(0, iy);

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

static void S32_D32_constX_shaderproc(const void* sIn,
                                      int x, int y,
                                      SkPMColor* SK_RESTRICT colors,
                                      int count) {
    const SkBitmapProcState& s = *static_cast<const SkBitmapProcState*>(sIn);
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != nullptr);
    SkASSERT(1 == s.fPixmap.width());

    int iY0;
    int iY1   SK_INIT_TO_AVOID_WARNING;
    int iSubY SK_INIT_TO_AVOID_WARNING;

    if (kNone_SkFilterQuality != s.fFilterQuality) {
        SkBitmapProcState::MatrixProc mproc = s.getMatrixProc();
        uint32_t xy[2];

        mproc(s, xy, 1, x, y);

        iY0 = xy[0] >> 18;
        iY1 = xy[0] & 0x3FFF;
        iSubY = (xy[0] >> 14) & 0xF;
    } else {
        int yTemp;

        if (s.fInvType > SkMatrix::kTranslate_Mask) {
            const SkBitmapProcStateAutoMapper mapper(s, x, y);

            // When the matrix has a scale component the setup code in
            // chooseProcs multiples the inverse matrix by the inverse of the
            // bitmap's width and height. Since this method is going to do
            // its own tiling and sampling we need to undo that here.
            if (SkShader::kClamp_TileMode != s.fTileModeX ||
                SkShader::kClamp_TileMode != s.fTileModeY) {
                yTemp = SkFractionalIntToInt(mapper.fractionalIntY() * s.fPixmap.height());
            } else {
                yTemp = mapper.intY();
            }
        } else {
            yTemp = s.fFilterOneY + y;
        }

        const int stopY = s.fPixmap.height();
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
            const SkBitmapProcStateAutoMapper mapper(s, x, y);
            int iY2;

            if (s.fInvType > SkMatrix::kTranslate_Mask &&
                (SkShader::kClamp_TileMode != s.fTileModeX ||
                 SkShader::kClamp_TileMode != s.fTileModeY)) {
                iY2 = SkFractionalIntToInt(mapper.fractionalIntY() * s.fPixmap.height());
            } else {
                iY2 = mapper.intY();
            }

            switch (s.fTileModeY) {
            case SkShader::kClamp_TileMode:
                iY2 = SkClampMax(iY2, stopY-1);
                break;
            case SkShader::kRepeat_TileMode:
                iY2 = sk_int_mod(iY2, stopY);
                break;
            case SkShader::kMirror_TileMode:
            default:
                iY2 = sk_int_mirror(iY2, stopY);
                break;
            }

            SkASSERT(iY0 == iY2);
        }
#endif
    }

    const SkPMColor* row0 = s.fPixmap.addr32(0, iY0);
    SkPMColor color;

    if (kNone_SkFilterQuality != s.fFilterQuality) {
        const SkPMColor* row1 = s.fPixmap.addr32(0, iY1);

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

static void DoNothing_shaderproc(const void*, int x, int y,
                                 SkPMColor* SK_RESTRICT colors, int count) {
    // if we get called, the matrix is too tricky, so we just draw nothing
    sk_memset32(colors, 0, count);
}

bool SkBitmapProcState::setupForTranslate() {
    SkPoint pt;
    const SkBitmapProcStateAutoMapper mapper(*this, 0, 0, &pt);

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
    fFilterOneX = mapper.intX();
    fFilterOneY = mapper.intY();

    return true;
}

SkBitmapProcState::ShaderProc32 SkBitmapProcState::chooseShaderProc32() {

    if (kN32_SkColorType != fPixmap.colorType()) {
        return nullptr;
    }

    static const unsigned kMask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;

    if (1 == fPixmap.width() && 0 == (fInvType & ~kMask)) {
        if (kNone_SkFilterQuality == fFilterQuality &&
            fInvType <= SkMatrix::kTranslate_Mask &&
            !this->setupForTranslate()) {
            return DoNothing_shaderproc;
        }
        return S32_D32_constX_shaderproc;
    }

    if (fAlphaScale < 256) {
        return nullptr;
    }
    if (fInvType > SkMatrix::kTranslate_Mask) {
        return nullptr;
    }
    if (kNone_SkFilterQuality != fFilterQuality) {
        return nullptr;
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
    return nullptr;
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
        proc = state.fFilterQuality != kNone_SkFilterQuality ?
                    check_scale_filter : check_scale_nofilter;
    } else {
        proc = state.fFilterQuality != kNone_SkFilterQuality ?
                    check_affine_filter : check_affine_nofilter;
    }
    proc(bitmapXY, count, state.fPixmap.width(), state.fPixmap.height());
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
    affine              filter        N * (Y Y X X)
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

    if (fFilterQuality != kNone_SkFilterQuality) {
        size >>= 1;
    }

    return size;
}

///////////////////////

void  Clamp_S32_opaque_D32_nofilter_DX_shaderproc(const void* sIn, int x, int y,
                                                  SkPMColor* SK_RESTRICT dst, int count) {
    const SkBitmapProcState& s = *static_cast<const SkBitmapProcState*>(sIn);
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);

    const unsigned maxX = s.fPixmap.width() - 1;
    SkFractionalInt fx;
    int dstY;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        const unsigned maxY = s.fPixmap.height() - 1;
        dstY = SkClampMax(mapper.intY(), maxY);
        fx = mapper.fractionalIntX();
    }

    const SkPMColor* SK_RESTRICT src = s.fPixmap.addr32(0, dstY);
    const SkFractionalInt dx = s.fInvSxFractionalInt;

    // Check if we're safely inside [0...maxX] so no need to clamp each computed index.
    //
    if ((uint64_t)SkFractionalIntToInt(fx) <= maxX &&
        (uint64_t)SkFractionalIntToInt(fx + dx * (count - 1)) <= maxX)
    {
        int count4 = count >> 2;
        for (int i = 0; i < count4; ++i) {
            SkPMColor src0 = src[SkFractionalIntToInt(fx)]; fx += dx;
            SkPMColor src1 = src[SkFractionalIntToInt(fx)]; fx += dx;
            SkPMColor src2 = src[SkFractionalIntToInt(fx)]; fx += dx;
            SkPMColor src3 = src[SkFractionalIntToInt(fx)]; fx += dx;
            dst[0] = src0;
            dst[1] = src1;
            dst[2] = src2;
            dst[3] = src3;
            dst += 4;
        }
        for (int i = (count4 << 2); i < count; ++i) {
            unsigned index = SkFractionalIntToInt(fx);
            SkASSERT(index <= maxX);
            *dst++ = src[index];
            fx += dx;
        }
    } else {
        for (int i = 0; i < count; ++i) {
            dst[i] = src[SkClampMax(SkFractionalIntToInt(fx), maxX)];
            fx += dx;
        }
    }
}
