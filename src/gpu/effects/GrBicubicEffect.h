/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBicubicTextureEffect_DEFINED
#define GrBicubicTextureEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "GrDrawEffect.h"
#include "gl/GrGLEffect.h"
#include "GrTBackendEffectFactory.h"

class GrGLBicubicEffect;

class GrBicubicEffect : public GrSingleTextureEffect {
public:
    enum {
        kFilterTexelPad = 2, // Given a src rect in texels to be filtered, this number of
                             // surrounding texels are needed by the kernel in x and y.
    };
    virtual ~GrBicubicEffect();

    static const char* Name() { return "Bicubic"; }
    const float* coefficients() const { return fCoefficients; }

    typedef GrGLBicubicEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    static GrEffectRef* Create(GrTexture* tex, const SkScalar coefficients[16]) {
        AutoEffectUnref effect(SkNEW_ARGS(GrBicubicEffect, (tex, coefficients)));
        return CreateEffectRef(effect);
    }

    static GrEffectRef* Create(GrTexture* tex, const SkScalar coefficients[16],
                               const SkMatrix& matrix,
                               const GrTextureParams& p,
                               GrCoordSet coordSet = kLocal_GrCoordSet) {
        AutoEffectUnref effect(SkNEW_ARGS(GrBicubicEffect, (tex, coefficients, matrix, p, coordSet)));
        return CreateEffectRef(effect);
    }

    static GrEffectRef* Create(GrTexture* tex) {
        return Create(tex, gMitchellCoefficients);
    }

    static GrEffectRef* Create(GrTexture* tex,
                               const SkMatrix& matrix,
                               const GrTextureParams& p,
                               GrCoordSet coordSet = kLocal_GrCoordSet) {
        return Create(tex, gMitchellCoefficients, matrix, p, coordSet);
    }

private:
    GrBicubicEffect(GrTexture*, const SkScalar coefficients[16]);
    GrBicubicEffect(GrTexture*, const SkScalar coefficients[16],
                    const SkMatrix &matrix, const GrTextureParams &p, GrCoordSet coordSet);
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;
    float    fCoefficients[16];

    GR_DECLARE_EFFECT_TEST;

    static const SkScalar gMitchellCoefficients[16];

    typedef GrSingleTextureEffect INHERITED;
};

#endif
