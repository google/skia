/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSingleTextureEffect_DEFINED
#define GrSingleTextureEffect_DEFINED

#include "GrEffect.h"
#include "GrMatrix.h"

class GrGLSingleTextureEffect;

/**
 * An effect that merely blits a single texture; commonly used as a base class.
 */
class GrSingleTextureEffect : public GrEffect {

public:
    /** These three constructors assume an identity matrix */
    GrSingleTextureEffect(GrTexture* texture); /* unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture* texture, bool bilerp); /* clamp mode */
    GrSingleTextureEffect(GrTexture* texture, const GrTextureParams&);

    /** These three constructors take an explicit matrix */
    GrSingleTextureEffect(GrTexture*, const GrMatrix&); /* unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture*, const GrMatrix&, bool bilerp); /* clamp mode */
    GrSingleTextureEffect(GrTexture*, const GrMatrix&, const GrTextureParams&);

    virtual ~GrSingleTextureEffect();

    virtual const GrTextureAccess& textureAccess(int index) const SK_OVERRIDE;

    static const char* Name() { return "Single Texture"; }

    const GrMatrix& getMatrix() const { return fMatrix; }

    typedef GrGLSingleTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    virtual bool isEqual(const GrEffect& effect) const SK_OVERRIDE {
        const GrSingleTextureEffect& ste = static_cast<const GrSingleTextureEffect&>(effect);
        return INHERITED::isEqual(effect) && fMatrix.cheapEqualTo(ste.getMatrix());
    }
private:
    GR_DECLARE_EFFECT_TEST;

    GrTextureAccess fTextureAccess;
    GrMatrix        fMatrix;

    typedef GrEffect INHERITED;
};

#endif
