/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTwoPointConicalGradient.h"
#include "SkTwoPointConicalGradient_gpu.h"

struct TwoPtRadialContext {
    const TwoPtRadial&  fRec;
    float               fRelX, fRelY;
    const float         fIncX, fIncY;
    float               fB;
    const float         fDB;

    TwoPtRadialContext(const TwoPtRadial& rec, SkScalar fx, SkScalar fy,
                       SkScalar dfx, SkScalar dfy);
    SkFixed nextT();
};

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
static int find_quad_roots(float A, float B, float C, float roots[2], bool descendingOrder = false) {
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
    if (descendingOrder) {
        SkTSwap(roots[0], roots[1]);
    }
    return 2;
}

static float lerp(float x, float dx, float t) {
    return x + t * dx;
}

static float sqr(float x) { return x * x; }

void TwoPtRadial::init(const SkPoint& center0, SkScalar rad0,
                       const SkPoint& center1, SkScalar rad1,
                       bool flipped) {
    fCenterX = SkScalarToFloat(center0.fX);
    fCenterY = SkScalarToFloat(center0.fY);
    fDCenterX = SkScalarToFloat(center1.fX) - fCenterX;
    fDCenterY = SkScalarToFloat(center1.fY) - fCenterY;
    fRadius = SkScalarToFloat(rad0);
    fDRadius = SkScalarToFloat(rad1) - fRadius;

    fA = sqr(fDCenterX) + sqr(fDCenterY) - sqr(fDRadius);
    fRadius2 = sqr(fRadius);
    fRDR = fRadius * fDRadius;

    fFlipped = flipped;
}

TwoPtRadialContext::TwoPtRadialContext(const TwoPtRadial& rec, SkScalar fx, SkScalar fy,
                                       SkScalar dfx, SkScalar dfy)
    : fRec(rec)
    , fRelX(SkScalarToFloat(fx) - rec.fCenterX)
    , fRelY(SkScalarToFloat(fy) - rec.fCenterY)
    , fIncX(SkScalarToFloat(dfx))
    , fIncY(SkScalarToFloat(dfy))
    , fB(-2 * (rec.fDCenterX * fRelX + rec.fDCenterY * fRelY + rec.fRDR))
    , fDB(-2 * (rec.fDCenterX * fIncX + rec.fDCenterY * fIncY)) {}

SkFixed TwoPtRadialContext::nextT() {
    float roots[2];

    float C = sqr(fRelX) + sqr(fRelY) - fRec.fRadius2;
    int countRoots = find_quad_roots(fRec.fA, fB, C, roots, fRec.fFlipped);

    fRelX += fIncX;
    fRelY += fIncY;
    fB += fDB;

    if (0 == countRoots) {
        return TwoPtRadial::kDontDrawT;
    }

    // Prefer the bigger t value if both give a radius(t) > 0
    // find_quad_roots returns the values sorted, so we start with the last
    float t = roots[countRoots - 1];
    float r = lerp(fRec.fRadius, fRec.fDRadius, t);
    if (r < 0) {
        t = roots[0];   // might be the same as roots[countRoots-1]
        r = lerp(fRec.fRadius, fRec.fDRadius, t);
        if (r < 0) {
            return TwoPtRadial::kDontDrawT;
        }
    }
    return SkFloatToFixed(t);
}

typedef void (*TwoPointConicalProc)(TwoPtRadialContext* rec, SkPMColor* dstC,
                                    const SkPMColor* cache, int toggle, int count);

static void twopoint_clamp(TwoPtRadialContext* rec, SkPMColor* SK_RESTRICT dstC,
                           const SkPMColor* SK_RESTRICT cache, int toggle,
                           int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = SkClampMax(t, 0xFFFF);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[toggle +
                            (index >> SkGradientShaderBase::kCache32Shift)];
        }
        toggle = next_dither_toggle(toggle);
    }
}

static void twopoint_repeat(TwoPtRadialContext* rec, SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache, int toggle,
                            int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = repeat_tileproc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[toggle +
                            (index >> SkGradientShaderBase::kCache32Shift)];
        }
        toggle = next_dither_toggle(toggle);
    }
}

static void twopoint_mirror(TwoPtRadialContext* rec, SkPMColor* SK_RESTRICT dstC,
                            const SkPMColor* SK_RESTRICT cache, int toggle,
                            int count) {
    for (; count > 0; --count) {
        SkFixed t = rec->nextT();
        if (TwoPtRadial::DontDrawT(t)) {
            *dstC++ = 0;
        } else {
            SkFixed index = mirror_tileproc(t);
            SkASSERT(index <= 0xFFFF);
            *dstC++ = cache[toggle +
                            (index >> SkGradientShaderBase::kCache32Shift)];
        }
        toggle = next_dither_toggle(toggle);
    }
}

/////////////////////////////////////////////////////////////////////

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
        const SkPoint& start, SkScalar startRadius,
        const SkPoint& end, SkScalar endRadius,
        bool flippedGrad, const Descriptor& desc)
    : SkGradientShaderBase(desc, SkMatrix::I())
    , fCenter1(start)
    , fCenter2(end)
    , fRadius1(startRadius)
    , fRadius2(endRadius)
    , fFlippedGrad(flippedGrad)
{
    // this is degenerate, and should be caught by our caller
    SkASSERT(fCenter1 != fCenter2 || fRadius1 != fRadius2);
    fRec.init(fCenter1, fRadius1, fCenter2, fRadius2, fFlippedGrad);
}

bool SkTwoPointConicalGradient::isOpaque() const {
    // Because areas outside the cone are left untouched, we cannot treat the
    // shader as opaque even if the gradient itself is opaque.
    // TODO(junov): Compute whether the cone fills the plane crbug.com/222380
    return false;
}

size_t SkTwoPointConicalGradient::onContextSize(const ContextRec&) const {
    return sizeof(TwoPointConicalGradientContext);
}

SkShader::Context* SkTwoPointConicalGradient::onCreateContext(const ContextRec& rec,
                                                              void* storage) const {
    return new (storage) TwoPointConicalGradientContext(*this, rec);
}

SkTwoPointConicalGradient::TwoPointConicalGradientContext::TwoPointConicalGradientContext(
        const SkTwoPointConicalGradient& shader, const ContextRec& rec)
    : INHERITED(shader, rec)
{
    // in general, we might discard based on computed-radius, so clear
    // this flag (todo: sometimes we can detect that we never discard...)
    fFlags &= ~kOpaqueAlpha_Flag;
}

void SkTwoPointConicalGradient::TwoPointConicalGradientContext::shadeSpan(
        int x, int y, SkPMColor* dstCParam, int count) {
    const SkTwoPointConicalGradient& twoPointConicalGradient =
            static_cast<const SkTwoPointConicalGradient&>(fShader);

    int toggle = init_dither_toggle(x, y);

    SkASSERT(count > 0);

    SkPMColor* SK_RESTRICT dstC = dstCParam;

    SkMatrix::MapXYProc dstProc = fDstToIndexProc;

    const SkPMColor* SK_RESTRICT cache = fCache->getCache32();

    TwoPointConicalProc shadeProc = twopoint_repeat;
    if (SkShader::kClamp_TileMode == twoPointConicalGradient.fTileMode) {
        shadeProc = twopoint_clamp;
    } else if (SkShader::kMirror_TileMode == twoPointConicalGradient.fTileMode) {
        shadeProc = twopoint_mirror;
    } else {
        SkASSERT(SkShader::kRepeat_TileMode == twoPointConicalGradient.fTileMode);
    }

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        SkPoint srcPt;
        dstProc(fDstToIndex, SkIntToScalar(x) + SK_ScalarHalf,
                SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkScalar dx, fx = srcPt.fX;
        SkScalar dy, fy = srcPt.fY;

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            const auto step = fDstToIndex.fixedStepInX(SkIntToScalar(y));
            dx = step.fX;
            dy = step.fY;
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = fDstToIndex.getScaleX();
            dy = fDstToIndex.getSkewY();
        }

        TwoPtRadialContext rec(twoPointConicalGradient.fRec, fx, fy, dx, dy);
        (*shadeProc)(&rec, dstC, cache, toggle, count);
    } else {    // perspective case
        SkScalar dstX = SkIntToScalar(x) + SK_ScalarHalf;
        SkScalar dstY = SkIntToScalar(y) + SK_ScalarHalf;
        for (; count > 0; --count) {
            SkPoint srcPt;
            dstProc(fDstToIndex, dstX, dstY, &srcPt);
            TwoPtRadialContext rec(twoPointConicalGradient.fRec, srcPt.fX, srcPt.fY, 0, 0);
            (*shadeProc)(&rec, dstC, cache, toggle, 1);

            dstX += SK_Scalar1;
            toggle = next_dither_toggle(toggle);
            dstC += 1;
        }
    }
}

// Returns the original non-sorted version of the gradient
SkShader::GradientType SkTwoPointConicalGradient::asAGradient(
    GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info, fFlippedGrad);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
        if (fFlippedGrad) {
            SkTSwap(info->fPoint[0], info->fPoint[1]);
            SkTSwap(info->fRadius[0], info->fRadius[1]);
        }
    }
    return kConical_GradientType;
}

sk_sp<SkFlattenable> SkTwoPointConicalGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    SkPoint c1 = buffer.readPoint();
    SkPoint c2 = buffer.readPoint();
    SkScalar r1 = buffer.readScalar();
    SkScalar r2 = buffer.readScalar();

    if (buffer.readBool()) {    // flipped
        SkTSwap(c1, c2);
        SkTSwap(r1, r2);

        SkColor* colors = desc.mutableColors();
        SkScalar* pos = desc.mutablePos();
        const int last = desc.fCount - 1;
        const int half = desc.fCount >> 1;
        for (int i = 0; i < half; ++i) {
            SkTSwap(colors[i], colors[last - i]);
            if (pos) {
                SkScalar tmp = pos[i];
                pos[i] = SK_Scalar1 - pos[last - i];
                pos[last - i] = SK_Scalar1 - tmp;
            }
        }
        if (pos) {
            if (desc.fCount & 1) {
                pos[half] = SK_Scalar1 - pos[half];
            }
        }
    }

    return SkGradientShader::MakeTwoPointConical(c1, r1, c2, r2, desc.fColors, desc.fPos,
                                                 desc.fCount, desc.fTileMode, desc.fGradFlags,
                                                 desc.fLocalMatrix);
}

void SkTwoPointConicalGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter1);
    buffer.writePoint(fCenter2);
    buffer.writeScalar(fRadius1);
    buffer.writeScalar(fRadius2);
    buffer.writeBool(fFlippedGrad);
}

#if SK_SUPPORT_GPU

#include "SkGr.h"

const GrFragmentProcessor* SkTwoPointConicalGradient::asFragmentProcessor(
                                                  GrContext* context,
                                                  const SkMatrix& viewM,
                                                  const SkMatrix* localMatrix,
                                                  SkFilterQuality) const {
    SkASSERT(context);
    SkASSERT(fPtsToUnit.isIdentity());
    SkAutoTUnref<const GrFragmentProcessor> inner(
        Gr2PtConicalGradientEffect::Create(context, *this, fTileMode, localMatrix));
    return GrFragmentProcessor::MulOutputByInputAlpha(inner);
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkTwoPointConicalGradient::toString(SkString* str) const {
    str->append("SkTwoPointConicalGradient: (");

    str->append("center1: (");
    str->appendScalar(fCenter1.fX);
    str->append(", ");
    str->appendScalar(fCenter1.fY);
    str->append(") radius1: ");
    str->appendScalar(fRadius1);
    str->append(" ");

    str->append("center2: (");
    str->appendScalar(fCenter2.fX);
    str->append(", ");
    str->appendScalar(fCenter2.fY);
    str->append(") radius2: ");
    str->appendScalar(fRadius2);
    str->append(" ");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
