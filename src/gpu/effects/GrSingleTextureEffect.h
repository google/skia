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
#include "GrTexture.h"

class GrGLSingleTextureEffect;

/**
 * An effect that draws a single texture with a texture matrix; commonly used as a base class. The
 * output color is the texture color is modulated against the input color.
 */
class GrSingleTextureEffect : public GrEffect {

public:
    /** These three constructors assume an identity matrix. TODO: Remove these.*/
    GrSingleTextureEffect(GrTexture* texture); /* unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture* texture, bool bilerp); /* clamp mode */
    GrSingleTextureEffect(GrTexture* texture, const GrTextureParams&);

    /** These three constructors take an explicit matrix */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&); /* unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, bool bilerp); /* clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, const GrTextureParams&);

    virtual ~GrSingleTextureEffect();

    virtual const GrTextureAccess& textureAccess(int index) const SK_OVERRIDE;

    static const char* Name() { return "Single Texture"; }

    const SkMatrix& getMatrix() const { return fMatrix; }

    typedef GrGLSingleTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    virtual bool isEqual(const GrEffect& effect) const SK_OVERRIDE {
        const GrSingleTextureEffect& ste = static_cast<const GrSingleTextureEffect&>(effect);
        return INHERITED::isEqual(effect) && fMatrix.cheapEqualTo(ste.getMatrix());
    }

    static inline SkMatrix MakeDivByTextureWHMatrix(const GrTexture* texture) {
        GrAssert(NULL != texture);
        SkMatrix mat;
        mat.setIDiv(texture->width(), texture->height());
        return mat;
    }

private:
    GR_DECLARE_EFFECT_TEST;

    GrTextureAccess fTextureAccess;
    SkMatrix        fMatrix;

    typedef GrEffect INHERITED;
};

#endif
