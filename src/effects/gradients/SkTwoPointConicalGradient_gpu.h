/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTwoPointConicalGradient_gpu_DEFINED
#define SkTwoPointConicalGradient_gpu_DEFINED

#if SK_SUPPORT_GPU

#include "SkGradientShaderPriv.h"

class GrEffectRef;
class SkTwoPointConicalGradient;
class GrGL2PtConicalGradientEffect;

class Gr2PtConicalGradientEffect : public GrGradientEffect {
public:

    static GrEffectRef* Create(GrContext* ctx,
                               const SkTwoPointConicalGradient& shader,
                               const SkMatrix& matrix,
                               SkShader::TileMode tm) {
        AutoEffectUnref effect(SkNEW_ARGS(Gr2PtConicalGradientEffect, (ctx, shader, matrix, tm)));
        return CreateEffectRef(effect);
    }

    virtual ~Gr2PtConicalGradientEffect() { }

    static const char* Name() { return "Two-Point Conical Gradient"; }
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return SkScalarAbs(fDiffRadius) == SkScalarAbs(fCenterX1); }
    SkScalar center() const { return fCenterX1; }
    SkScalar diffRadius() const { return fDiffRadius; }
    SkScalar radius() const { return fRadius0; }

    typedef GrGL2PtConicalGradientEffect GLEffect;

private:
    virtual bool onIsEqual(const GrEffect& sBase) const SK_OVERRIDE {
        const Gr2PtConicalGradientEffect& s = CastEffect<Gr2PtConicalGradientEffect>(sBase);
        return (INHERITED::onIsEqual(sBase) &&
                this->fCenterX1 == s.fCenterX1 &&
                this->fRadius0 == s.fRadius0 &&
                this->fDiffRadius == s.fDiffRadius);
    }

    Gr2PtConicalGradientEffect(GrContext* ctx,
                       const SkTwoPointConicalGradient& shader,
                       const SkMatrix& matrix,
                       SkShader::TileMode tm);

    GR_DECLARE_EFFECT_TEST;

    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrCoordTransform fBTransform;
    SkScalar         fCenterX1;
    SkScalar         fRadius0;
    SkScalar         fDiffRadius;

    // @}

    typedef GrGradientEffect INHERITED;
};
#endif
#endif
