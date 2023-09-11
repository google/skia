/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldGeoProc_DEFINED
#define GrDistanceFieldGeoProc_DEFINED

#if !defined(SK_DISABLE_SDF_TEXT)

#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "src/base/SkArenaAlloc.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrSamplerState.h"

#include <cstdint>
#include <memory>

class GrSurfaceProxyView;
namespace skgpu { class KeyBuilder; }
struct GrShaderCaps;

enum GrDistanceFieldEffectFlags {
    kSimilarity_DistanceFieldEffectFlag   = 0x001, // ctm is similarity matrix
    kScaleOnly_DistanceFieldEffectFlag    = 0x002, // ctm has only scale and translate
    kPerspective_DistanceFieldEffectFlag  = 0x004, // ctm has perspective (and positions are x,y,w)
    kUseLCD_DistanceFieldEffectFlag       = 0x008, // use lcd text
    kBGR_DistanceFieldEffectFlag          = 0x010, // lcd display has bgr order
    kPortrait_DistanceFieldEffectFlag     = 0x020, // lcd display is in portrait mode (not used yet)
    kGammaCorrect_DistanceFieldEffectFlag = 0x040, // assume gamma-correct output (linear blending)
    kAliased_DistanceFieldEffectFlag      = 0x080, // monochrome output
    kWideColor_DistanceFieldEffectFlag    = 0x100, // use wide color (only for path)

    kInvalid_DistanceFieldEffectFlag      = 0x200,   // invalid state (for initialization)

    kUniformScale_DistanceFieldEffectMask = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag,
    // The subset of the flags relevant to GrDistanceFieldA8TextGeoProc
    kNonLCD_DistanceFieldEffectMask       = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag |
                                            kPerspective_DistanceFieldEffectFlag |
                                            kGammaCorrect_DistanceFieldEffectFlag |
                                            kAliased_DistanceFieldEffectFlag,
    // The subset of the flags relevant to GrDistanceFieldPathGeoProc
    kPath_DistanceFieldEffectMask         = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag |
                                            kPerspective_DistanceFieldEffectFlag |
                                            kGammaCorrect_DistanceFieldEffectFlag |
                                            kAliased_DistanceFieldEffectFlag |
                                            kWideColor_DistanceFieldEffectFlag,
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
    inline static constexpr int kMaxTextures = 4;

    /** The local matrix should be identity if local coords are not required by the GrPipeline. */
#ifdef SK_GAMMA_APPLY_TO_A8
    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const GrShaderCaps& caps,
                                     const GrSurfaceProxyView* views,
                                     int numActiveViews,
                                     GrSamplerState params,
                                     float lum,
                                     uint32_t flags,
                                     const SkMatrix& localMatrixIfUsesLocalCoords) {
        return arena->make([&](void* ptr) {
            return new (ptr) GrDistanceFieldA8TextGeoProc(
                    caps, views, numActiveViews, params, lum, flags, localMatrixIfUsesLocalCoords);
        });
    }
#else
    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const GrShaderCaps& caps,
                                     const GrSurfaceProxyView* views,
                                     int numActiveViews,
                                     GrSamplerState params,
                                     uint32_t flags,
                                     const SkMatrix& localMatrixIfUsesLocalCoords) {
        return arena->make([&](void* ptr) {
            return new (ptr) GrDistanceFieldA8TextGeoProc(
                    caps, views, numActiveViews, params, flags, localMatrixIfUsesLocalCoords);
        });
    }
#endif

    ~GrDistanceFieldA8TextGeoProc() override {}

    const char* name() const override { return "DistanceFieldA8Text"; }

    void addNewViews(const GrSurfaceProxyView* views, int numViews, GrSamplerState);

    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

private:
    class Impl;

    GrDistanceFieldA8TextGeoProc(const GrShaderCaps& caps,
                                 const GrSurfaceProxyView* views,
                                 int numActiveViews,
                                 GrSamplerState params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                 float distanceAdjust,
#endif
                                 uint32_t flags,
                                 const SkMatrix& localMatrix);

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSamplers[i]; }

    TextureSampler   fTextureSamplers[kMaxTextures];
    SkISize          fAtlasDimensions;  // dimensions for all textures used with fTextureSamplers[].
    SkMatrix         fLocalMatrix;
    Attribute        fInPosition;
    Attribute        fInColor;
    Attribute        fInTextureCoords;
    uint32_t         fFlags;
#ifdef SK_GAMMA_APPLY_TO_A8
    float            fDistanceAdjust;
#endif

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

/**
 * The output color of this effect is a modulation of the input color and a sample from a
 * distance field texture (using a smoothed step function near 0.5).
 * It allows explicit specification of the filtering and wrap modes (GrSamplerState). The input
 * coords are a custom attribute. No gamma correct blending is applied. Used for paths only.
 */
class GrDistanceFieldPathGeoProc : public GrGeometryProcessor {
public:
    inline static constexpr int kMaxTextures = 4;

    /** The local matrix should be identity if local coords are not required by the GrPipeline. */
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, const GrShaderCaps& caps,
                                     const GrSurfaceProxyView* views, int numActiveViews,
                                     GrSamplerState params, const SkMatrix& localMatrix,
                                     uint32_t flags) {
        return arena->make([&](void* ptr) {
            return new (ptr) GrDistanceFieldPathGeoProc(caps, views, numActiveViews,
                                                        params, localMatrix, flags);
        });
    }

    ~GrDistanceFieldPathGeoProc() override {}

    const char* name() const override { return "DistanceFieldPath"; }

    void addNewViews(const GrSurfaceProxyView*, int numActiveViews, GrSamplerState);

    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

private:
    class Impl;

    GrDistanceFieldPathGeoProc(const GrShaderCaps& caps,
                               const GrSurfaceProxyView* views,
                               int numActiveViews,
                               GrSamplerState,
                               const SkMatrix& localMatrix,
                               uint32_t flags);

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSamplers[i]; }

    SkMatrix         fLocalMatrix;
    TextureSampler   fTextureSamplers[kMaxTextures];
    SkISize          fAtlasDimensions;  // dimensions for all textures used with fTextureSamplers[].
    Attribute        fInPosition;
    Attribute        fInColor;
    Attribute        fInTextureCoords;
    uint32_t         fFlags;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

/**
 * The output color of this effect is a modulation of the input color and samples from a
 * distance field texture (using a smoothed step function near 0.5), adjusted for LCD displays.
 * It allows explicit specification of the filtering and wrap modes (GrSamplerState). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldLCDTextGeoProc : public GrGeometryProcessor {
public:
    inline static constexpr int kMaxTextures = 4;

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

    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const GrShaderCaps& caps,
                                     const GrSurfaceProxyView* views,
                                     int numActiveViews,
                                     GrSamplerState params,
                                     DistanceAdjust distanceAdjust,
                                     uint32_t flags,
                                     const SkMatrix& localMatrixIfUsesLocalCoords) {
        return arena->make([&](void* ptr) {
            return new (ptr) GrDistanceFieldLCDTextGeoProc(caps, views, numActiveViews, params,
                                                           distanceAdjust, flags,
                                                           localMatrixIfUsesLocalCoords);
        });
    }

    ~GrDistanceFieldLCDTextGeoProc() override {}

    const char* name() const override { return "DistanceFieldLCDText"; }

    void addNewViews(const GrSurfaceProxyView*, int numActiveViews, GrSamplerState);

    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

private:
    class Impl;

    GrDistanceFieldLCDTextGeoProc(const GrShaderCaps& caps, const GrSurfaceProxyView* views,
                                  int numActiveViews, GrSamplerState params, DistanceAdjust wa,
                                  uint32_t flags, const SkMatrix& localMatrix);

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSamplers[i]; }

    TextureSampler   fTextureSamplers[kMaxTextures];
    SkISize          fAtlasDimensions;  // dimensions for all textures used with fTextureSamplers[].
    const SkMatrix   fLocalMatrix;
    DistanceAdjust   fDistanceAdjust;
    Attribute        fInPosition;
    Attribute        fInColor;
    Attribute        fInTextureCoords;
    uint32_t         fFlags;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

#endif // !defined(SK_DISABLE_SDF_TEXT)

#endif
