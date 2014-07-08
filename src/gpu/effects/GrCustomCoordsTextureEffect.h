/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomCoordsTextureEffect_DEFINED
#define GrCustomCoordsTextureEffect_DEFINED

#include "GrEffect.h"
#include "GrVertexEffect.h"

class GrGLCustomCoordsTextureEffect;

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute.
 */
class GrCustomCoordsTextureEffect : public GrVertexEffect {
public:
    static GrEffect* Create(GrTexture* tex, const GrTextureParams& p) {
        return SkNEW_ARGS(GrCustomCoordsTextureEffect, (tex, p));
    }

    virtual ~GrCustomCoordsTextureEffect() {}

    static const char* Name() { return "Texture"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    typedef GrGLCustomCoordsTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrCustomCoordsTextureEffect(GrTexture* texture, const GrTextureParams& params);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    GrTextureAccess fTextureAccess;

    GR_DECLARE_EFFECT_TEST;

    typedef GrVertexEffect INHERITED;
};

#endif
