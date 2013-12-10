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

    /**
     * Create a simple Mitchell filter effect.
     */
    static GrEffectRef* Create(GrTexture* tex) {
        return Create(tex, gMitchellCoefficients);
    }

    /**
     * Create a simple filter effect with custom bicubic coefficients.
     */
    static GrEffectRef* Create(GrTexture* tex, const SkScalar coefficients[16]) {
        const SkShader::TileMode tm[] = { SkShader::kClamp_TileMode, SkShader::kClamp_TileMode };
        return Create(tex, coefficients, MakeDivByTextureWHMatrix(tex), tm);
    }

    /**
     * Create a Mitchell filter effect with specified texture matrix and x/y tile modes.
     */
    static GrEffectRef* Create(GrTexture* tex,
                               const SkMatrix& matrix,
                               SkShader::TileMode tileModes[2]) {
        return Create(tex, gMitchellCoefficients, matrix, tileModes);
    }

    /**
     * The most general Create method. This allows specification of the bicubic coefficients, the
     * texture matrix, and the x/y tilemodes.
     */
    static GrEffectRef* Create(GrTexture* tex, const SkScalar coefficients[16],
                               const SkMatrix& matrix,
                               const SkShader::TileMode tileModes[2]) {
        AutoEffectUnref effect(SkNEW_ARGS(GrBicubicEffect, (tex, coefficients, matrix, tileModes)));
        return CreateEffectRef(effect);
    }

private:
    GrBicubicEffect(GrTexture*, const SkScalar coefficients[16],
                    const SkMatrix &matrix, const SkShader::TileMode tileModes[2]);
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;
    float    fCoefficients[16];

    GR_DECLARE_EFFECT_TEST;

    static const SkScalar gMitchellCoefficients[16];

    typedef GrSingleTextureEffect INHERITED;
};

#endif
