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
#include "gl/GrGLEffectMatrix.h"
#include "GrTBackendEffectFactory.h"

class GrGLBicubicEffect;

class GrBicubicEffect : public GrSingleTextureEffect {
public:
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
                               CoordsType coordsType = kLocal_CoordsType) {
        AutoEffectUnref effect(SkNEW_ARGS(GrBicubicEffect, (tex, coefficients, matrix, p, coordsType)));
        return CreateEffectRef(effect);
    }

    static GrEffectRef* Create(GrTexture* tex) {
        return Create(tex, gMitchellCoefficients);
    }

    static GrEffectRef* Create(GrTexture* tex, 
                               const SkMatrix& matrix,
                               const GrTextureParams& p,
                               CoordsType coordsType = kLocal_CoordsType) {
        return Create(tex, gMitchellCoefficients, matrix, p, coordsType);
    }

private:
    GrBicubicEffect(GrTexture*, const SkScalar coefficients[16]);
    GrBicubicEffect(GrTexture*, const SkScalar coefficients[16], 
                    const SkMatrix &matrix, const GrTextureParams &p, CoordsType coordsType);
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;
    float    fCoefficients[16];

    GR_DECLARE_EFFECT_TEST;
    
    static const SkScalar gMitchellCoefficients[16];

    typedef GrSingleTextureEffect INHERITED;
};

#endif
