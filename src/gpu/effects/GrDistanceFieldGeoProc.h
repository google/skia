/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldGeoProc_DEFINED
#define GrDistanceFieldGeoProc_DEFINED

#include "GrProcessor.h"
#include "GrGeometryProcessor.h"

class GrGLDistanceFieldA8TextGeoProc;
class GrGLDistanceFieldPathGeoProc;
class GrGLDistanceFieldLCDTextGeoProc;
class GrInvariantOutput;

enum GrDistanceFieldEffectFlags {
    kSimilarity_DistanceFieldEffectFlag   = 0x01, // ctm is similarity matrix
    kScaleOnly_DistanceFieldEffectFlag    = 0x02, // ctm has only scale and translate
    kPerspective_DistanceFieldEffectFlag  = 0x04, // ctm has perspective (and positions are x,y,w)
    kUseLCD_DistanceFieldEffectFlag       = 0x08, // use lcd text
    kBGR_DistanceFieldEffectFlag          = 0x10, // lcd display has bgr order
    kPortrait_DistanceFieldEffectFlag     = 0x20, // lcd display is in portrait mode (not used yet)
    kGammaCorrect_DistanceFieldEffectFlag = 0x40, // assume gamma-correct output (linear blending)
    kAliased_DistanceFieldEffectFlag      = 0x80, // monochrome output

    kInvalid_DistanceFieldEffectFlag      = 0x100,   // invalid state (for initialization)

    kUniformScale_DistanceFieldEffectMask = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag,
    // The subset of the flags relevant to GrDistanceFieldA8TextGeoProc
    kNonLCD_DistanceFieldEffectMask       = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag |
                                            kPerspective_DistanceFieldEffectFlag |
                                            kGammaCorrect_DistanceFieldEffectFlag |
                                            kAliased_DistanceFieldEffectFlag,
    // The subset of the flags relevant to GrDistanceFieldLCDTextGeoProc
    kLCD_DistanceFieldEffectMask          = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag |
                                            kPerspective_DistanceFieldEffectFlag |
                                            kUseLCD_DistanceFieldEffectFlag |
                                            kBGR_DistanceFieldEffectFlag |
                                            kGammaCorrect_DistanceFieldEffectFlag,
};

/**
 * The output color of this effect is a modulation of the input color and a sample from a
 * distance field texture (using a smoothed step function near 0.5).
 * It allows explicit specification of the filtering and wrap modes (GrSamplerState). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldA8TextGeoProc : public GrGeometryProcessor {
public:
    static constexpr int kMaxTextures = 4;

    /** The local matrix should be identity if local coords are not required by the GrPipeline. */
#ifdef SK_GAMMA_APPLY_TO_A8
    static sk_sp<GrGeometryProcessor> Make(const GrShaderCaps& caps,
                                           const sk_sp<GrTextureProxy>* proxies,
                                           int numActiveProxies,
                                           const GrSamplerState& params, float lum, uint32_t flags,
                                           const SkMatrix& localMatrixIfUsesLocalCoords) {
        return sk_sp<GrGeometryProcessor>(new GrDistanceFieldA8TextGeoProc(
                caps, proxies, numActiveProxies, params, lum, flags, localMatrixIfUsesLocalCoords));
    }
#else
    static sk_sp<GrGeometryProcessor> Make(const GrShaderCaps& caps,
                                           const sk_sp<GrTextureProxy>* proxies,
                                           int numActiveProxies,
                                           const GrSamplerState& params, uint32_t flags,
                                           const SkMatrix& localMatrixIfUsesLocalCoords) {
        return sk_sp<GrGeometryProcessor>(new GrDistanceFieldA8TextGeoProc(
                caps, proxies, numActiveProxies, params, flags, localMatrixIfUsesLocalCoords));
    }
#endif

    ~GrDistanceFieldA8TextGeoProc() override {}

    const char* name() const override { return "DistanceFieldA8Text"; }

    const Attribute& inPosition() const { return fInPosition; }
    const Attribute& inColor() const { return fInColor; }
    const Attribute& inTextureCoords() const { return fInTextureCoords; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
#ifdef SK_GAMMA_APPLY_TO_A8
    float getDistanceAdjust() const { return fDistanceAdjust; }
#endif
    uint32_t getFlags() const { return fFlags; }
    const SkISize& atlasSize() const { return fAtlasSize; }

    void addNewProxies(const sk_sp<GrTextureProxy>* proxies, int numProxies, const GrSamplerState&);

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    GrDistanceFieldA8TextGeoProc(const GrShaderCaps& caps,
                                 const sk_sp<GrTextureProxy>* proxies,
                                 int numActiveProxies,
                                 const GrSamplerState& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                 float distanceAdjust,
#endif
                                 uint32_t flags, const SkMatrix& localMatrix);

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSamplers[i]; }

    TextureSampler   fTextureSamplers[kMaxTextures];
    SkISize          fAtlasSize;  // size for all textures used with fTextureSamplers[].
    SkMatrix         fLocalMatrix;
    Attribute        fInPosition;
    Attribute        fInColor;
    Attribute        fInTextureCoords;
    uint32_t         fFlags;
#ifdef SK_GAMMA_APPLY_TO_A8
    float            fDistanceAdjust;
#endif

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

/**
 * The output color of this effect is a modulation of the input color and a sample from a
 * distance field texture (using a smoothed step function near 0.5).
 * It allows explicit specification of the filtering and wrap modes (GrSamplerState). The input
 * coords are a custom attribute. No gamma correct blending is applied. Used for paths only.
 */
class GrDistanceFieldPathGeoProc : public GrGeometryProcessor {
public:
    static constexpr int kMaxTextures = 4;

    /** The local matrix should be identity if local coords are not required by the GrPipeline. */
    static sk_sp<GrGeometryProcessor> Make(const GrShaderCaps& caps,
                                           const SkMatrix& matrix,
                                           bool wideColor,
                                           const sk_sp<GrTextureProxy>* proxies,
                                           int numActiveProxies,
                                           const GrSamplerState& params, uint32_t flags) {
        return sk_sp<GrGeometryProcessor>(
            new GrDistanceFieldPathGeoProc(caps, matrix, wideColor, proxies, numActiveProxies,
                                           params, flags));
    }

    ~GrDistanceFieldPathGeoProc() override {}

    const char* name() const override { return "DistanceFieldPath"; }

    const Attribute& inPosition() const { return fInPosition; }
    const Attribute& inColor() const { return fInColor; }
    const Attribute& inTextureCoords() const { return fInTextureCoords; }
    const SkMatrix& matrix() const { return fMatrix; }
    uint32_t getFlags() const { return fFlags; }
    const SkISize& atlasSize() const { return fAtlasSize; }

    void addNewProxies(const sk_sp<GrTextureProxy>*, int numActiveProxies, const GrSamplerState&);

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    GrDistanceFieldPathGeoProc(const GrShaderCaps& caps,
                               const SkMatrix& matrix,
                               bool wideColor,
                               const sk_sp<GrTextureProxy>* proxies,
                               int numActiveProxies,
                               const GrSamplerState&, uint32_t flags);

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSamplers[i]; }

    SkMatrix         fMatrix;     // view matrix if perspective, local matrix otherwise
    TextureSampler   fTextureSamplers[kMaxTextures];
    SkISize          fAtlasSize;  // size for all textures used with fTextureSamplers[].
    Attribute        fInPosition;
    Attribute        fInColor;
    Attribute        fInTextureCoords;
    uint32_t         fFlags;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

/**
 * The output color of this effect is a modulation of the input color and samples from a
 * distance field texture (using a smoothed step function near 0.5), adjusted for LCD displays.
 * It allows explicit specification of the filtering and wrap modes (GrSamplerState). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldLCDTextGeoProc : public GrGeometryProcessor {
public:
    static constexpr int kMaxTextures = 4;

    struct DistanceAdjust {
        SkScalar fR, fG, fB;
        static DistanceAdjust Make(SkScalar r, SkScalar g, SkScalar b) {
            DistanceAdjust result;
            result.fR = r; result.fG = g; result.fB = b;
            return result;
        }
        bool operator==(const DistanceAdjust& wa) const {
            return (fR == wa.fR && fG == wa.fG && fB == wa.fB);
        }
        bool operator!=(const DistanceAdjust& wa) const {
            return !(*this == wa);
        }
    };

    static sk_sp<GrGeometryProcessor> Make(const GrShaderCaps& caps,
                                           const sk_sp<GrTextureProxy>* proxies,
                                           int numActiveProxies,
                                           const GrSamplerState& params,
                                           DistanceAdjust distanceAdjust,
                                           uint32_t flags,
                                           const SkMatrix& localMatrixIfUsesLocalCoords) {
        return sk_sp<GrGeometryProcessor>(
            new GrDistanceFieldLCDTextGeoProc(caps, proxies, numActiveProxies, params,
                                              distanceAdjust, flags, localMatrixIfUsesLocalCoords));
    }

    ~GrDistanceFieldLCDTextGeoProc() override {}

    const char* name() const override { return "DistanceFieldLCDText"; }

    const Attribute& inPosition() const { return fInPosition; }
    const Attribute& inColor() const { return fInColor; }
    const Attribute& inTextureCoords() const { return fInTextureCoords; }
    DistanceAdjust getDistanceAdjust() const { return fDistanceAdjust; }
    uint32_t getFlags() const { return fFlags; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    const SkISize& atlasSize() const { return fAtlasSize; }

    void addNewProxies(const sk_sp<GrTextureProxy>*, int numActiveProxies, const GrSamplerState&);

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    GrDistanceFieldLCDTextGeoProc(const GrShaderCaps& caps, const sk_sp<GrTextureProxy>* proxies,
                                  int numActiveProxies, const GrSamplerState& params,
                                  DistanceAdjust wa, uint32_t flags, const SkMatrix& localMatrix);

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSamplers[i]; }

    TextureSampler   fTextureSamplers[kMaxTextures];
    SkISize          fAtlasSize;  // size for all textures used with fTextureSamplers[].
    const SkMatrix   fLocalMatrix;
    DistanceAdjust   fDistanceAdjust;
    Attribute        fInPosition;
    Attribute        fInColor;
    Attribute        fInTextureCoords;
    uint32_t         fFlags;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

#endif
