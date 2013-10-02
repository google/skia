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
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). It can use
 * local coords, positions, or a custom vertex attribute as input texture coords. The input coords
 * can have a matrix applied in the VS in both the local and position cases but not with a custom
 * attribute coords at this time. It will add a varying to input interpolate texture coords to the
 * FS.
 */
class GrSimpleTextureEffect : public GrSingleTextureEffect {
public:
    /* unfiltered, clamp mode */
    static GrEffectRef* Create(GrTexture* tex,
                               const SkMatrix& matrix,
                               GrCoordSet coordSet = kLocal_GrCoordSet) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix, GrTextureParams::kNone_FilterMode, coordSet)));
        return CreateEffectRef(effect);
    }

    /* clamp mode */
    static GrEffectRef* Create(GrTexture* tex,
                               const SkMatrix& matrix,
                               GrTextureParams::FilterMode filterMode,
                               GrCoordSet coordSet = kLocal_GrCoordSet) {
        AutoEffectUnref effect(
            SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix, filterMode, coordSet)));
        return CreateEffectRef(effect);
    }

    static GrEffectRef* Create(GrTexture* tex,
                               const SkMatrix& matrix,
                               const GrTextureParams& p,
                               GrCoordSet coordSet = kLocal_GrCoordSet) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix, p, coordSet)));
        return CreateEffectRef(effect);
    }

    virtual ~GrSimpleTextureEffect() {}

    static const char* Name() { return "Texture"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    typedef GrGLSimpleTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrSimpleTextureEffect(GrTexture* texture,
                          const SkMatrix& matrix,
                          GrTextureParams::FilterMode filterMode,
                          GrCoordSet coordSet)
        : GrSingleTextureEffect(texture, matrix, filterMode, coordSet) {
    }

    GrSimpleTextureEffect(GrTexture* texture,
                          const SkMatrix& matrix,
                          const GrTextureParams& params,
                          GrCoordSet coordSet)
        : GrSingleTextureEffect(texture, matrix, params, coordSet) {
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const GrSimpleTextureEffect& ste = CastEffect<GrSimpleTextureEffect>(other);
        return this->hasSameTextureParamsMatrixAndSourceCoords(ste);
    }

    GR_DECLARE_EFFECT_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
