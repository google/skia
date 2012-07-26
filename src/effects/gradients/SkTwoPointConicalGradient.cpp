
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTwoPointConicalGradient.h"

static int valid_divide(float numer, float denom, float* ratio) {
    SkASSERT(ratio);
    if (0 == denom) {
        return 0;
    }
    *ratio = numer / denom;
    return 1;
}

// Return the number of distinct real roots, and write them into roots[] in
// ascending order
static int find_quad_roots(float A, float B, float C, float roots[2]) {
    SkASSERT(roots);
    
    if (A == 0) {
        return valid_divide(-C, B, roots);
    }
    
    float R = B*B - 4*A*C;
    if (R < 0) {
        return 0;
    }
    R = sk_float_sqrt(R);

#if 1
    float Q = B;
    if (Q < 0) {
        Q -= R;
    } else {
        Q += R;
    }
#else
    // on 10.6 this was much slower than the above branch :(
    float Q = B + copysignf(R, B);
#endif
    Q *= -0.5f;
    if (0 == Q) {
        roots[0] = 0;
        return 1;
    }

    float r0 = Q / A;
    float r1 = C / Q;
    roots[0] = r0 < r1 ? r0 : r1;
    roots[1] = r0 > r1 ? r0 : r1;
    return 2;
}

static float lerp(float x, float dx, float t) {
    return x + t * dx;
}

static float sqr(float x) { return x * x; }

void TwoPtRadial::init(const SkPoint& center0, SkScalar rad0,
                       const SkPoint& center1, SkScalar rad1) {
    fCenterX = SkScalarToFloat(center0.fX);
    fCenterY = SkScalarToFloat(center0.fY);
    fDCenterX = SkScalarToFloat(center1.fX) - fCenterX;
    fDCenterY = SkScalarToFloat(center1.fY) - fCenterY;
    fRadius = SkScalarToFloat(rad0);
    fDRadius = SkScalarToFloat(rad1) - fRadius;

    fA = sqr(fDCenterX) + sqr(fDCenterY) - sqr(fDRadius);
    fRadius2 = sqr(fRadius);
    fRDR = fRadius * fDRadius;
}

void TwoPtRadial::setup(SkScalar fx, SkScalar fy, SkScalar dfx, SkScalar dfy) {
    fRelX = SkScalarToFloat(fx) - fCenterX;
    fRelY = SkScalarToFloat(fy) - fCenterY;
    fIncX = SkScalarToFloat(dfx);
    fIncY = SkScalarToFloat(dfy);
    fB = -2 * (fDCenterX * fRelX + fDCenterY * fRelY + fRDR);
    fDB = -2 * (fDCenterX * fIncX + fDCenterY * fIncY);
}

SkFixed TwoPtRadial::nextT() {
    float roots[2];
    
    float C = sqr(fRelX) + sqr(fRelY) - fRadius2;
    int countRoots = find_quad_roots(fA, fB, C, roots);

    fRelX += fIncX;
    fRelY += fIncY;
    fB += fDB;

    if (0 == countRoots) {
        return kDontDrawT;
    }

    // Prefer the bigger t value if both give a radius(t) > 0
    // find_quad_roots returns the values sorted, so we start with the last
    float t = roots[countRoots - 1];
    float r = lerp(fRadius, fDRadius, t);
    if (r <= 0) {
        t = roots[0];   // might be the same as roots[countRoots-1]
        r = lerp(fRadius, fDRadius, t);
        if (r <= 0) {
            return kDontDrawT;
        }
    }
    return SkFloatToFixed(t);
}

typedef void (*TwoPointRadialProc)(TwoPtRadial* rec, SkPMColor* dstC,
                                   const SkPMColor* cache, int count);

static void twopoint_clamp(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                           const SkPMColor* SK_RESTRICT cache, int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = SkClampMax(t, 0xFFFF);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        }
    }
}

static void twopoint_repeat(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache, int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = repeat_tileproc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        }
    }
}

static void twopoint_mirror(TwoPtRadial* rec, SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache, int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = mirror_tileproc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[index >> SkGradientShaderBase::kCache32Shift];
        }
    }
}

void SkTwoPointConicalGradient::init() {
    fRec.init(fCenter1, fRadius1, fCenter2, fRadius2);
    fPtsToUnit.reset();
}

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
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
    // this is degenerate, and should be caught by our caller
    SkASSERT(fCenter1 != fCenter2 || fRadius1 != fRadius2);
    this->init();
}

void SkTwoPointConicalGradient::shadeSpan(int x, int y, SkPMColor* dstCParam,
                                          int count) {
    SkASSERT(count > 0);
    
    SkPMColor* SK_RESTRICT dstC = dstCParam;
    
    SkMatrix::MapXYProc dstProc = fDstToIndexProc;
    TileProc            proc = fTileProc;
    const SkPMColor* SK_RESTRICT cache = this->getCache32();

    TwoPointRadialProc shadeProc = twopoint_repeat;
    if (SkShader::kClamp_TileMode == fTileMode) {
        shadeProc = twopoint_clamp;
    } else if (SkShader::kMirror_TileMode == fTileMode) {
        shadeProc = twopoint_mirror;
    } else {
        SkASSERT(SkShader::kRepeat_TileMode == fTileMode);
    }
    
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

        fRec.setup(fx, fy, dx, dy);
        (*shadeProc)(&fRec, dstC, cache, count);
    } else {    // perspective case
        SkScalar dstX = SkIntToScalar(x);
        SkScalar dstY = SkIntToScalar(y);
        for (; count > 0; --count) {
            SkPoint srcPt;
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            dstX += SK_Scalar1;
            
            fRec.setup(srcPt.fX, srcPt.fY, 0, 0);
            (*shadeProc)(&fRec, dstC, cache, 1);
        }
    }
}

bool SkTwoPointConicalGradient::setContext(const SkBitmap& device,
                                           const SkPaint& paint,
                                           const SkMatrix& matrix) {
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }
    
    // we don't have a span16 proc
    fFlags &= ~kHasSpan16_Flag;
    
    // in general, we might discard based on computed-radius, so clear
    // this flag (todo: sometimes we can detect that we never discard...)
    fFlags &= ~kOpaqueAlpha_Flag;

    return true;
}

SkShader::BitmapType SkTwoPointConicalGradient::asABitmap(
    SkBitmap* bitmap, SkMatrix* matrix, SkShader::TileMode* xy) const {
    SkPoint diff = fCenter2 - fCenter1;
    SkScalar diffRadius = fRadius2 - fRadius1;
    SkScalar startRadius = fRadius1;
    SkScalar diffLen = 0;

    if (bitmap) {
        this->commonAsABitmap(bitmap);
    }
    if (matrix) {
        diffLen = diff.length();
    }
    if (matrix) {
        if (diffLen) {
            SkScalar invDiffLen = SkScalarInvert(diffLen);
            // rotate to align circle centers with the x-axis
            matrix->setSinCos(-SkScalarMul(invDiffLen, diff.fY),
                              SkScalarMul(invDiffLen, diff.fX));
        } else {
            matrix->reset();
        }
        matrix->preTranslate(-fCenter1.fX, -fCenter1.fY);
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kTwoPointConical_BitmapType;
}

SkShader::GradientType SkTwoPointConicalGradient::asAGradient(
    GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
    }
    return kConical_GradientType;
}

GrCustomStage* SkTwoPointConicalGradient::asNewCustomStage(
    GrContext* context, GrSamplerState* sampler) const {
    SkASSERT(NULL != context && NULL != sampler);
    SkPoint diff = fCenter2 - fCenter1;
    SkScalar diffLen = diff.length();
    if (0 != diffLen) {
        SkScalar invDiffLen = SkScalarInvert(diffLen);
        sampler->matrix()->setSinCos(-SkScalarMul(invDiffLen, diff.fY),
                          SkScalarMul(invDiffLen, diff.fX));
    } else {
        sampler->matrix()->reset();
    }
    sampler->matrix()->preTranslate(-fCenter1.fX, -fCenter1.fY);
    sampler->textureParams()->setTileModeX(fTileMode);
    sampler->textureParams()->setTileModeY(kClamp_TileMode);
    sampler->textureParams()->setBilerp(true);
    return SkNEW_ARGS(GrConical2Gradient, (context, *this, sampler, 
                      diffLen, fRadius1, fRadius2 - fRadius1));
}

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
    SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer),
    fCenter1(buffer.readPoint()),
    fCenter2(buffer.readPoint()),
    fRadius1(buffer.readScalar()),
    fRadius2(buffer.readScalar()) {
    this->init();
};

void SkTwoPointConicalGradient::flatten(
    SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter1);
    buffer.writePoint(fCenter2);
    buffer.writeScalar(fRadius1);
    buffer.writeScalar(fRadius2);
}

