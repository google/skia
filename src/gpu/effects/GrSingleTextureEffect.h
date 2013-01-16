/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSingleTextureEffect_DEFINED
#define GrSingleTextureEffect_DEFINED

#include "GrEffect.h"
#include "SkMatrix.h"

class GrGLSingleTextureEffect;
class GrTexture;

/**
 * An effect that draws a single texture with a texture matrix; commonly used as a base class. The
 * output color is the texture color is modulated against the input color.
 */
class GrSingleTextureEffect : public GrEffect {
public:
    /* unfiltered, clamp mode */
    static GrEffectRef* Create(GrTexture* tex, const SkMatrix& matrix) {
        SkAutoTUnref<GrEffect> effect(SkNEW_ARGS(GrSingleTextureEffect, (tex, matrix)));
        return CreateEffectPtr(effect);
    }

    /* clamp mode */
    static GrEffectRef* Create(GrTexture* tex, const SkMatrix& matrix, bool bilerp) {
        SkAutoTUnref<GrEffect> effect(SkNEW_ARGS(GrSingleTextureEffect, (tex, matrix, bilerp)));
        return CreateEffectPtr(effect);
    }

    static GrEffectRef* Create(GrTexture* tex, const SkMatrix& matrix, const GrTextureParams& p) {
        SkAutoTUnref<GrEffect> effect(SkNEW_ARGS(GrSingleTextureEffect, (tex, matrix, p)));
        return CreateEffectPtr(effect);
    }

    virtual ~GrSingleTextureEffect();

    static const char* Name() { return "Single Texture"; }

    /** Note that if this class is sub-classed, the subclass may have to override this function.
     */
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    const SkMatrix& getMatrix() const { return fMatrix; }

    typedef GrGLSingleTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    virtual bool isEqual(const GrEffect& effect) const SK_OVERRIDE {
        const GrSingleTextureEffect& ste = static_cast<const GrSingleTextureEffect&>(effect);
        return INHERITED::isEqual(effect) && fMatrix.cheapEqualTo(ste.getMatrix());
    }

protected:
    GrSingleTextureEffect(GrTexture*, const SkMatrix&); /* unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, bool bilerp); /* clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, const GrTextureParams&);

private:

    GR_DECLARE_EFFECT_TEST;

    GrTextureAccess fTextureAccess;
    SkMatrix        fMatrix;

    typedef GrEffect INHERITED;
};

#endif
