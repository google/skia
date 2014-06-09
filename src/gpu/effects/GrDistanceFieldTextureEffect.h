/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldTextureEffect_DEFINED
#define GrDistanceFieldTextureEffect_DEFINED

#include "GrEffect.h"
#include "GrVertexEffect.h"

class GrGLDistanceFieldTextureEffect;
class GrGLDistanceFieldLCDTextureEffect;

/**
 * The output color of this effect is a modulation of the input color and a sample from a
 * distance field texture (using a smoothed step function near 0.5).
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldTextureEffect : public GrVertexEffect {
public:
#ifdef SK_GAMMA_APPLY_TO_A8
    static GrEffectRef* Create(GrTexture* tex, const GrTextureParams& params,
                               GrTexture* gamma, const GrTextureParams& gammaParams, float lum,
                               bool similarity) {
        AutoEffectUnref effect(SkNEW_ARGS(GrDistanceFieldTextureEffect, (tex, params,
                                                                         gamma, gammaParams, lum,
                                                                         similarity)));
#else
    static GrEffectRef* Create(GrTexture* tex, const GrTextureParams& params,
                               bool similarity) {
        AutoEffectUnref effect(SkNEW_ARGS(GrDistanceFieldTextureEffect, (tex, params,
                                                                         similarity)));
#endif
        return CreateEffectRef(effect);
    }

    virtual ~GrDistanceFieldTextureEffect() {}

    static const char* Name() { return "DistanceFieldTexture"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;
#ifdef SK_GAMMA_APPLY_TO_A8
    float getLuminance() const { return fLuminance; }
#endif
    bool isSimilarity() const { return fIsSimilarity; }

    typedef GrGLDistanceFieldTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrDistanceFieldTextureEffect(GrTexture* texture, const GrTextureParams& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                 GrTexture* gamma, const GrTextureParams& gammaParams, float lum,
#endif
                                 bool uniformScale);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    GrTextureAccess fTextureAccess;
#ifdef SK_GAMMA_APPLY_TO_A8
    GrTextureAccess fGammaTextureAccess;
    float           fLuminance;
#endif
    bool            fIsSimilarity;

    GR_DECLARE_EFFECT_TEST;

    typedef GrVertexEffect INHERITED;
};

/**
 * The output color of this effect is a modulation of the input color and samples from a
 * distance field texture (using a smoothed step function near 0.5), adjusted for LCD displays.
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldLCDTextureEffect : public GrVertexEffect {
public:
    static GrEffectRef* Create(GrTexture* tex, const GrTextureParams& params,
                               GrTexture* gamma, const GrTextureParams& gammaParams, 
                               SkColor textColor,
                               bool uniformScale, bool useBGR) {
        AutoEffectUnref effect(SkNEW_ARGS(GrDistanceFieldLCDTextureEffect,
                                          (tex, params, gamma, gammaParams, textColor, uniformScale,
                                           useBGR)));
        return CreateEffectRef(effect);
    }

    virtual ~GrDistanceFieldLCDTextureEffect() {}

    static const char* Name() { return "DistanceFieldLCDTexture"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;
    GrColor getTextColor() const { return fTextColor; }
    bool isUniformScale() const { return fUniformScale; }
    bool useBGR() const { return fUseBGR; }

    typedef GrGLDistanceFieldLCDTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrDistanceFieldLCDTextureEffect(GrTexture* texture, const GrTextureParams& params,
                                    GrTexture* gamma, const GrTextureParams& gammaParams,
                                    SkColor textColor,
                                    bool uniformScale, bool useBGR);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    GrTextureAccess fTextureAccess;
    GrTextureAccess fGammaTextureAccess;
    GrColor         fTextColor;
    bool            fUniformScale;
    bool            fUseBGR;

    GR_DECLARE_EFFECT_TEST;

    typedef GrVertexEffect INHERITED;
};

#endif
