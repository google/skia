/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fGradientBase.h"

namespace {

// true when x is in [k1,k2)
bool in_range(SkScalar x, SkScalar k1, SkScalar k2) {
    SkASSERT(k1 != k2);
    return (k1 < k2)
        ? (x >= k1 && x < k2)
        : (x >= k2 && x < k1);
}

} // anonymous namespace

SkGradientShaderBase::GradientShaderBase4fContext::
Interval::Interval(SkPMColor c0, SkScalar p0,
                   SkPMColor c1, SkScalar p1,
                   const Sk4f& componentScale)
    : fP0(p0)
    , fP1(p1)
    , fZeroRamp(c0 == c1) {
    SkASSERT(p0 != p1);

    const Sk4f c4f0 = SkNx_cast<float>(Sk4b::Load(&c0)) * componentScale;
    const Sk4f c4f1 = SkNx_cast<float>(Sk4b::Load(&c1)) * componentScale;
    const Sk4f dc4f = (c4f1 - c4f0) / (p1 - p0);

    c4f0.store(&fC0.fVec);
    dc4f.store(&fDc.fVec);
}

SkGradientShaderBase::GradientShaderBase4fContext::
Interval::Interval(const Sk4f& c0, const Sk4f& dc,
                   SkScalar p0, SkScalar p1)
    : fP0(p0)
    , fP1(p1)
    , fZeroRamp((dc == 0).allTrue()) {
    c0.store(fC0.fVec);
    dc.store(fDc.fVec);
}

bool SkGradientShaderBase::GradientShaderBase4fContext::
Interval::contains(SkScalar fx) const {
    return in_range(fx, fP0, fP1);
}

SkGradientShaderBase::
GradientShaderBase4fContext::GradientShaderBase4fContext(const SkGradientShaderBase& shader,
                                                         const ContextRec& rec)
    : INHERITED(shader, rec)
    , fFlags(this->INHERITED::getFlags())
#ifdef SK_SUPPORT_LEGACY_GRADIENT_DITHERING
    , fDither(true)
#else
    , fDither(rec.fPaint->isDither())
#endif
{
    const SkMatrix& inverse = this->getTotalInverse();
    fDstToPos.setConcat(shader.fPtsToUnit, inverse);
    fDstToPosProc = fDstToPos.getMapXYProc();
    fDstToPosClass = static_cast<uint8_t>(INHERITED::ComputeMatrixClass(fDstToPos));

    if (shader.fColorsAreOpaque && this->getPaintAlpha() == SK_AlphaOPAQUE) {
        fFlags |= kOpaqueAlpha_Flag;
    }

    fColorsArePremul =
        (shader.fGradFlags & SkGradientShader::kInterpolateColorsInPremul_Flag)
        || shader.fColorsAreOpaque;
}
