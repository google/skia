
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "SkTwoPointRadialGradient.h"

/* Two-point radial gradients are specified by two circles, each with a center
   point and radius.  The gradient can be considered to be a series of
   concentric circles, with the color interpolated from the start circle
   (at t=0) to the end circle (at t=1).

   For each point (x, y) in the span, we want to find the
   interpolated circle that intersects that point.  The center
   of the desired circle (Cx, Cy) falls at some distance t
   along the line segment between the start point (Sx, Sy) and
   end point (Ex, Ey):

      Cx = (1 - t) * Sx + t * Ex        (0 <= t <= 1)
      Cy = (1 - t) * Sy + t * Ey

   The radius of the desired circle (r) is also a linear interpolation t
   between the start and end radii (Sr and Er):

      r = (1 - t) * Sr + t * Er

   But

      (x - Cx)^2 + (y - Cy)^2 = r^2

   so

     (x - ((1 - t) * Sx + t * Ex))^2
   + (y - ((1 - t) * Sy + t * Ey))^2
   = ((1 - t) * Sr + t * Er)^2

   Solving for t yields

     [(Sx - Ex)^2 + (Sy - Ey)^2 - (Er - Sr)^2)] * t^2
   + [2 * (Sx - Ex)(x - Sx) + 2 * (Sy - Ey)(y - Sy) - 2 * (Er - Sr) * Sr] * t
   + [(x - Sx)^2 + (y - Sy)^2 - Sr^2] = 0

   To simplify, let Dx = Sx - Ex, Dy = Sy - Ey, Dr = Er - Sr, dx = x - Sx, dy = y - Sy

     [Dx^2 + Dy^2 - Dr^2)] * t^2
   + 2 * [Dx * dx + Dy * dy - Dr * Sr] * t
   + [dx^2 + dy^2 - Sr^2] = 0

   A quadratic in t.  The two roots of the quadratic reflect the two
   possible circles on which the point may fall.  Solving for t yields
   the gradient value to use.

   If a<0, the start circle is entirely contained in the
   end circle, and one of the roots will be <0 or >1 (off the line
   segment).  If a>0, the start circle falls at least partially
   outside the end circle (or vice versa), and the gradient
   defines a "tube" where a point may be on one circle (on the
   inside of the tube) or the other (outside of the tube).  We choose
   one arbitrarily.

   In order to keep the math to within the limits of fixed point,
   we divide the entire quadratic by Dr^2, and replace
   (x - Sx)/Dr with x' and (y - Sy)/Dr with y', giving

   [Dx^2 / Dr^2 + Dy^2 / Dr^2 - 1)] * t^2
   + 2 * [x' * Dx / Dr + y' * Dy / Dr - Sr / Dr] * t
   + [x'^2 + y'^2 - Sr^2/Dr^2] = 0

   (x' and y' are computed by appending the subtract and scale to the
   fDstToIndex matrix in the constructor).

   Since the 'A' component of the quadratic is independent of x' and y', it
   is precomputed in the constructor.  Since the 'B' component is linear in
   x' and y', if x and y are linear in the span, 'B' can be computed
   incrementally with a simple delta (db below).  If it is not (e.g.,
   a perspective projection), it must be computed in the loop.

*/

namespace {

inline SkFixed two_point_radial(SkScalar b, SkScalar fx, SkScalar fy,
                                SkScalar sr2d2, SkScalar foura,
                                SkScalar oneOverTwoA, bool posRoot) {
    SkScalar c = SkScalarSquare(fx) + SkScalarSquare(fy) - sr2d2;
    if (0 == foura) {
        return SkScalarToFixed(SkScalarDiv(-c, b));
    }

    SkScalar discrim = SkScalarSquare(b) - SkScalarMul(foura, c);
    if (discrim < 0) {
        discrim = -discrim;
    }
    SkScalar rootDiscrim = SkScalarSqrt(discrim);
    SkScalar result;
    if (posRoot) {
        result = SkScalarMul(-b + rootDiscrim, oneOverTwoA);
    } else {
        result = SkScalarMul(-b - rootDiscrim, oneOverTwoA);
    }
    return SkScalarToFixed(result);
}

typedef void (* TwoPointRadialShadeProc)(SkScalar fx, SkScalar dx,
        SkScalar fy, SkScalar dy,
        SkScalar b, SkScalar db,
        SkScalar fSr2D2, SkScalar foura, SkScalar fOneOverTwoA, bool posRoot,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count);

void shadeSpan_twopoint_clamp(SkScalar fx, SkScalar dx,
        SkScalar fy, SkScalar dy,
        SkScalar b, SkScalar db,
        SkScalar fSr2D2, SkScalar foura, SkScalar fOneOverTwoA, bool posRoot,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count) {
    for (; count > 0; --count) {
        SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura,
                                     fOneOverTwoA, posRoot);
        SkFixed index = SkClampMax(t, 0xFFFF);
        SkASSERT(index <= 0xFFFF);
        *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        fx += dx;
        fy += dy;
        b += db;
    }
}
void shadeSpan_twopoint_mirror(SkScalar fx, SkScalar dx,
        SkScalar fy, SkScalar dy,
        SkScalar b, SkScalar db,
        SkScalar fSr2D2, SkScalar foura, SkScalar fOneOverTwoA, bool posRoot,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count) {
    for (; count > 0; --count) {
        SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura,
                                     fOneOverTwoA, posRoot);
        SkFixed index = mirror_tileproc(t);
        SkASSERT(index <= 0xFFFF);
        *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        fx += dx;
        fy += dy;
        b += db;
    }
}

void shadeSpan_twopoint_repeat(SkScalar fx, SkScalar dx,
        SkScalar fy, SkScalar dy,
        SkScalar b, SkScalar db,
        SkScalar fSr2D2, SkScalar foura, SkScalar fOneOverTwoA, bool posRoot,
        SkPMColor* SK_RESTRICT dstC, const SkPMColor* SK_RESTRICT cache,
        int count) {
    for (; count > 0; --count) {
        SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura,
                                     fOneOverTwoA, posRoot);
        SkFixed index = repeat_tileproc(t);
        SkASSERT(index <= 0xFFFF);
        *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        fx += dx;
        fy += dy;
        b += db;
    }
}
}

SkTwoPointRadialGradient::SkTwoPointRadialGradient(
    const SkPoint& start, SkScalar startRadius,
    const SkPoint& end, SkScalar endRadius,
    const SkColor colors[], const SkScalar pos[],
    int colorCount, SkShader::TileMode mode,
    SkUnitMapper* mapper)
    : SkGradientShaderBase(colors, pos, colorCount, mode, mapper),
      fCenter1(start),
      fCenter2(end),
      fRadius1(startRadius),
      fRadius2(endRadius) {
    init();
}

SkShader::BitmapType SkTwoPointRadialGradient::asABitmap(
    SkBitmap* bitmap,
    SkMatrix* matrix,
    SkShader::TileMode* xy) const {
    if (bitmap) {
        this->commonAsABitmap(bitmap);
    }
    SkScalar diffL = 0; // just to avoid gcc warning
    if (matrix) {
        diffL = SkScalarSqrt(SkScalarSquare(fDiff.fX) +
                             SkScalarSquare(fDiff.fY));
    }
    if (matrix) {
        if (diffL) {
            SkScalar invDiffL = SkScalarInvert(diffL);
            matrix->setSinCos(-SkScalarMul(invDiffL, fDiff.fY),
                              SkScalarMul(invDiffL, fDiff.fX));
        } else {
            matrix->reset();
        }
        matrix->preConcat(fPtsToUnit);
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kTwoPointRadial_BitmapType;
}

SkShader::GradientType SkTwoPointRadialGradient::asAGradient(
    SkShader::GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
    }
    return kRadial2_GradientType;
}

GrCustomStage* SkTwoPointRadialGradient::asNewCustomStage(
    GrContext* context, GrSamplerState* sampler) const {
    SkASSERT(NULL != context && NULL != sampler);
    SkScalar diffLen = fDiff.length();
    if (0 != diffLen) {
        SkScalar invDiffLen = SkScalarInvert(diffLen);
        sampler->matrix()->setSinCos(-SkScalarMul(invDiffLen, fDiff.fY),
                                     SkScalarMul(invDiffLen, fDiff.fX));
    } else {
        sampler->matrix()->reset();
    }
    sampler->matrix()->preConcat(fPtsToUnit);
    sampler->textureParams()->setTileModeX(fTileMode);
    sampler->textureParams()->setTileModeY(kClamp_TileMode);
    sampler->textureParams()->setBilerp(true);
    return SkNEW_ARGS(GrRadial2Gradient, (context, *this, sampler, 
        diffLen, fStartRadius, fDiffRadius));
}

void SkTwoPointRadialGradient::shadeSpan(int x, int y, SkPMColor* dstCParam,
                                         int count) {
    SkASSERT(count > 0);

    SkPMColor* SK_RESTRICT dstC = dstCParam;

    // Zero difference between radii:  fill with transparent black.
    if (fDiffRadius == 0) {
      sk_bzero(dstC, count * sizeof(*dstC));
      return;
    }
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = fTileProc;
    const SkPMColor* SK_RESTRICT cache = this->getCache32();

    SkScalar foura = fA * 4;
    bool posRoot = fDiffRadius < 0;
    if (fDstToIndexClass != kPerspective_MatrixClass) {
        SkPoint srcPt;
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                             SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkScalar dx, fx = srcPt.fX;
        SkScalar dy, fy = srcPt.fY;

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed fixedX, fixedY;
            (void)fDstToIndex.fixedStepInX(SkIntToScalar(y), &fixedX, &fixedY);
            dx = SkFixedToScalar(fixedX);
            dy = SkFixedToScalar(fixedY);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = fDstToIndex.getScaleX();
            dy = fDstToIndex.getSkewY();
        }
        SkScalar b = (SkScalarMul(fDiff.fX, fx) +
                     SkScalarMul(fDiff.fY, fy) - fStartRadius) * 2;
        SkScalar db = (SkScalarMul(fDiff.fX, dx) +
                      SkScalarMul(fDiff.fY, dy)) * 2;

        TwoPointRadialShadeProc shadeProc = shadeSpan_twopoint_repeat;
        if (SkShader::kClamp_TileMode == fTileMode) {
            shadeProc = shadeSpan_twopoint_clamp;
        } else if (SkShader::kMirror_TileMode == fTileMode) {
            shadeProc = shadeSpan_twopoint_mirror;
        } else {
            SkASSERT(SkShader::kRepeat_TileMode == fTileMode);
        }
        (*shadeProc)(fx, dx, fy, dy, b, db,
                     fSr2D2, foura, fOneOverTwoA, posRoot,
                     dstC, cache, count);
    } else {    // perspective case
        SkScalar dstX = SkIntToScalar(x);
        SkScalar dstY = SkIntToScalar(y);
        for (; count > 0; --count) {
            SkPoint             srcPt;
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            SkScalar fx = srcPt.fX;
            SkScalar fy = srcPt.fY;
            SkScalar b = (SkScalarMul(fDiff.fX, fx) +
                         SkScalarMul(fDiff.fY, fy) - fStartRadius) * 2;
            SkFixed t = two_point_radial(b, fx, fy, fSr2D2, foura,
                                         fOneOverTwoA, posRoot);
            SkFixed index = proc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
            dstX += SK_Scalar1;
        }
    }
}

bool SkTwoPointRadialGradient::setContext(
    const SkBitmap& device,
    const SkPaint& paint,
    const SkMatrix& matrix){
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    // For now, we might have divided by zero, so detect that
    if (0 == fDiffRadius) {
        return false;
    }

    // we don't have a span16 proc
    fFlags &= ~kHasSpan16_Flag;
    return true;
}

SkTwoPointRadialGradient::SkTwoPointRadialGradient(
    SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer),
      fCenter1(buffer.readPoint()),
      fCenter2(buffer.readPoint()),
      fRadius1(buffer.readScalar()),
      fRadius2(buffer.readScalar()) {
    init();
};

void SkTwoPointRadialGradient::flatten(
    SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter1);
    buffer.writePoint(fCenter2);
    buffer.writeScalar(fRadius1);
    buffer.writeScalar(fRadius2);
}

void SkTwoPointRadialGradient::init() {
    fDiff = fCenter1 - fCenter2;
    fDiffRadius = fRadius2 - fRadius1;
    // hack to avoid zero-divide for now
    SkScalar inv = fDiffRadius ? SkScalarInvert(fDiffRadius) : 0;
    fDiff.fX = SkScalarMul(fDiff.fX, inv);
    fDiff.fY = SkScalarMul(fDiff.fY, inv);
    fStartRadius = SkScalarMul(fRadius1, inv);
    fSr2D2 = SkScalarSquare(fStartRadius);
    fA = SkScalarSquare(fDiff.fX) + SkScalarSquare(fDiff.fY) - SK_Scalar1;
    fOneOverTwoA = fA ? SkScalarInvert(fA * 2) : 0;

    fPtsToUnit.setTranslate(-fCenter1.fX, -fCenter1.fY);
    fPtsToUnit.postScale(inv, inv);
}

