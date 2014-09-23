/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldTextureEffect_DEFINED
#define GrDistanceFieldTextureEffect_DEFINED

#include "GrProcessor.h"
#include "GrGeometryProcessor.h"

class GrGLDistanceFieldTextureEffect;
class GrGLDistanceFieldLCDTextureEffect;

enum GrDistanceFieldEffectFlags {
    kSimilarity_DistanceFieldEffectFlag = 0x01,   // ctm is similarity matrix
    kRectToRect_DistanceFieldEffectFlag = 0x02,   // ctm maps rects to rects
    kUseLCD_DistanceFieldEffectFlag     = 0x04,   // use lcd text
    kBGR_DistanceFieldEffectFlag        = 0x08,   // lcd display has bgr order
    kPortrait_DistanceFieldEffectFlag   = 0x10,   // lcd display is in portrait mode (not used yet)
    
    kUniformScale_DistanceFieldEffectMask = kSimilarity_DistanceFieldEffectFlag |
                                            kRectToRect_DistanceFieldEffectFlag,
    // The subset of the flags relevant to GrDistanceFieldTextureEffect
    kNonLCD_DistanceFieldEffectMask       = kSimilarity_DistanceFieldEffectFlag,
    // The subset of the flags relevant to GrDistanceFieldLCDTextureEffect
    kLCD_DistanceFieldEffectMask          = kSimilarity_DistanceFieldEffectFlag |
                                            kRectToRect_DistanceFieldEffectFlag |
                                            kUseLCD_DistanceFieldEffectFlag |
                                            kBGR_DistanceFieldEffectFlag,
};

/**
 * The output color of this effect is a modulation of the input color and a sample from a
 * distance field texture (using a smoothed step function near 0.5).
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldTextureEffect : public GrGeometryProcessor {
public:
#ifdef SK_GAMMA_APPLY_TO_A8
    static GrGeometryProcessor* Create(GrTexture* tex, const GrTextureParams& params,
                                       GrTexture* gamma, const GrTextureParams& gammaParams,
                                       float lum, uint32_t flags) {
       return SkNEW_ARGS(GrDistanceFieldTextureEffect, (tex, params, gamma, gammaParams, lum,
                                                        flags));
    }
#else
    static GrGeometryProcessor* Create(GrTexture* tex, const GrTextureParams& params,
                                       uint32_t flags) {
        return  SkNEW_ARGS(GrDistanceFieldTextureEffect, (tex, params, flags));
    }
#endif

    virtual ~GrDistanceFieldTextureEffect() {}

    static const char* Name() { return "DistanceFieldTexture"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    const GrShaderVar& inTextureCoords() const { return fInTextureCoords; }
#ifdef SK_GAMMA_APPLY_TO_A8
    float getLuminance() const { return fLuminance; }
#endif
    uint32_t getFlags() const { return fFlags; }

    typedef GrGLDistanceFieldTextureEffect GLProcessor;

    virtual const GrBackendGeometryProcessorFactory& getFactory() const SK_OVERRIDE;

private:
    GrDistanceFieldTextureEffect(GrTexture* texture, const GrTextureParams& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                 GrTexture* gamma, const GrTextureParams& gammaParams, float lum,
#endif
                                 uint32_t flags);

    virtual bool onIsEqual(const GrProcessor& other) const SK_OVERRIDE;

    GrTextureAccess    fTextureAccess;
#ifdef SK_GAMMA_APPLY_TO_A8
    GrTextureAccess    fGammaTextureAccess;
    float              fLuminance;
#endif
    uint32_t           fFlags;
    const GrShaderVar& fInTextureCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

/**
 * The output color of this effect is a modulation of the input color and samples from a
 * distance field texture (using a smoothed step function near 0.5), adjusted for LCD displays.
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldLCDTextureEffect : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(GrTexture* tex, const GrTextureParams& params,
                                       GrTexture* gamma, const GrTextureParams& gammaParams,
                                       SkColor textColor, uint32_t flags) {
        return SkNEW_ARGS(GrDistanceFieldLCDTextureEffect,
                          (tex, params, gamma, gammaParams, textColor, flags));
    }

    virtual ~GrDistanceFieldLCDTextureEffect() {}

    static const char* Name() { return "DistanceFieldLCDTexture"; }

    const GrShaderVar& inTextureCoords() const { return fInTextureCoords; }
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;
    GrColor getTextColor() const { return fTextColor; }
    uint32_t getFlags() const { return fFlags; }

    typedef GrGLDistanceFieldLCDTextureEffect GLProcessor;

    virtual const GrBackendGeometryProcessorFactory& getFactory() const SK_OVERRIDE;

private:
    GrDistanceFieldLCDTextureEffect(GrTexture* texture, const GrTextureParams& params,
                                    GrTexture* gamma, const GrTextureParams& gammaParams,
                                    SkColor textColor,
                                    uint32_t flags);

    virtual bool onIsEqual(const GrProcessor& other) const SK_OVERRIDE;

    GrTextureAccess    fTextureAccess;
    GrTextureAccess    fGammaTextureAccess;
    GrColor            fTextColor;
    uint32_t           fFlags;
    const GrShaderVar& fInTextureCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

#endif
