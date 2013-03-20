/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleTextureEffect_DEFINED
#define GrSimpleTextureEffect_DEFINED

#include "GrSingleTextureEffect.h"

class GrGLSimpleTextureEffect;

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * The coord to sample the texture is determine by a matrix. It allows explicit specification of
 * the filtering and wrap modes (GrTextureParams).
 */
class GrSimpleTextureEffect : public GrSingleTextureEffect {
public:
    /* unfiltered, clamp mode */
    static GrEffectRef* Create(GrTexture* tex, const SkMatrix& matrix) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix)));
        return CreateEffectRef(effect);
    }

    /* clamp mode */
    static GrEffectRef* Create(GrTexture* tex, const SkMatrix& matrix, bool bilerp) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix, bilerp)));
        return CreateEffectRef(effect);
    }

    static GrEffectRef* Create(GrTexture* tex, const SkMatrix& matrix, const GrTextureParams& p) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix, p)));
        return CreateEffectRef(effect);
    }

    virtual ~GrSimpleTextureEffect() {}

    static const char* Name() { return "Texture"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    typedef GrGLSimpleTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrSimpleTextureEffect(GrTexture* texture, const SkMatrix& matrix)
        : GrSingleTextureEffect(texture, matrix) {}
    GrSimpleTextureEffect(GrTexture* texture, const SkMatrix& matrix, bool bilerp)
        : GrSingleTextureEffect(texture, matrix, bilerp) {}
    GrSimpleTextureEffect(GrTexture* texture, const SkMatrix& matrix, const GrTextureParams& params)
        : GrSingleTextureEffect(texture, matrix, params) {}

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const GrSimpleTextureEffect& ste = CastEffect<GrSimpleTextureEffect>(other);
        return this->hasSameTextureParamsAndMatrix(ste);
    }

    GR_DECLARE_EFFECT_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
