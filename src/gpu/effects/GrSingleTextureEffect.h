/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSingleTextureEffect_DEFINED
#define GrSingleTextureEffect_DEFINED

#include "GrEffect.h"

class GrGLSingleTextureEffect;

/**
 * An effect that merely blits a single texture; commonly used as a base class.
 */
class GrSingleTextureEffect : public GrEffect {

public:
    /** Uses default texture params (unfiltered, clamp) */
    GrSingleTextureEffect(GrTexture* texture);

    /** Uses default tile mode (clamp) */
    GrSingleTextureEffect(GrTexture* texture, bool bilerp);

    GrSingleTextureEffect(GrTexture* texture, const GrTextureParams&);

    virtual ~GrSingleTextureEffect();

    virtual const GrTextureAccess& textureAccess(int index) const SK_OVERRIDE;

    static const char* Name() { return "Single Texture"; }

    typedef GrGLSingleTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GR_DECLARE_EFFECT_TEST;

    GrTextureAccess fTextureAccess;

    typedef GrEffect INHERITED;
};

#endif
