/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureEffect_DEFINED
#define GrTextureEffect_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"

class GrTextureEffect : public GrFragmentProcessor {
public:
    static constexpr float kDefaultBorder[4] = {0};

    /** Make from a filter. The sampler will be configured with clamp mode. */
    static std::unique_ptr<GrFragmentProcessor> Make(
            GrSurfaceProxyView,
            SkAlphaType,
            const SkMatrix& = SkMatrix::I(),
            GrSamplerState::Filter = GrSamplerState::Filter::kNearest);

    /**
     * Make from a full GrSamplerState. Caps are required to determine support for kClampToBorder.
     * This will be emulated in the shader if there is no hardware support.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView, SkAlphaType,
                                                     const SkMatrix&, GrSamplerState,
                                                     const GrCaps& caps,
                                                     const float border[4] = kDefaultBorder);

    /**
     * Makes a texture effect that samples a subset of a texture. The wrap modes of the
     * GrSampleState are applied to the subset in the shader rather than using HW samplers.
     * The 'subset' parameter specifies the texels in the base level. The shader code will
     * avoid allowing bilerp filtering to read outside the texel window. However, if MIP
     * filtering is used and a shader invocation reads from a level other than the base
     * then it may read texel values that were computed from in part from base level texels
     * outside the window. More specifically, we treat the MIP map case exactly like the
     * bilerp case in terms of how the final texture coords are computed.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeSubset(GrSurfaceProxyView,
                                                           SkAlphaType,
                                                           const SkMatrix&,
                                                           GrSamplerState,
                                                           const SkRect& subset,
                                                           const GrCaps& caps,
                                                           const float border[4] = kDefaultBorder);

    /**
     * The same as above but also takes a 'domain' that specifies any known limit on the post-
     * matrix texture coords that will be used to sample the texture. Specifying this requires
     * knowledge of how this effect will be nested into a paint, the local coords used with the
     * draw, etc. It is only used to attempt to optimize away the shader subset calculations.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeSubset(GrSurfaceProxyView,
                                                           SkAlphaType,
                                                           const SkMatrix&,
                                                           GrSamplerState,
                                                           const SkRect& subset,
                                                           const SkRect& domain,
                                                           const GrCaps& caps,
                                                           const float border[4] = kDefaultBorder);

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const char* name() const override { return "TextureEffect"; }

private:
    enum class ShaderMode : uint16_t {
        kClamp         = static_cast<int>(GrSamplerState::WrapMode::kClamp),
        kRepeat        = static_cast<int>(GrSamplerState::WrapMode::kRepeat),
        kMirrorRepeat  = static_cast<int>(GrSamplerState::WrapMode::kMirrorRepeat),
        kClampToBorder = static_cast<int>(GrSamplerState::WrapMode::kClampToBorder),
        kNone,
    };

    struct Sampling;

    /**
     * Sometimes the implementation of a ShaderMode depends on which GrSamplerState::Filter is
     * used.
     */
    enum class FilterLogic {
        kNone,                  // The shader isn't specialized for the filter.
        kRepeatBilerp,          // Filter across the subset boundary for kRepeat mode
        kRepeatMipMap,          // Logic for LOD selection with kRepeat mode.
        kClampToBorderFilter,   // Logic for fading to border color when filtering.
        kClampToBorderNearest,  // Logic for hard transition to border color when not filtering.
    };
    static FilterLogic GetFilterLogic(ShaderMode mode, GrSamplerState::Filter filter);

    GrCoordTransform fCoordTransform;
    TextureSampler fSampler;
    float fBorder[4];
    SkRect fSubset;
    SkRect fClamp;
    ShaderMode fShaderModes[2];

    inline GrTextureEffect(GrSurfaceProxyView, SkAlphaType, const SkMatrix&, const Sampling&);

    explicit GrTextureEffect(const GrTextureEffect& src);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const TextureSampler& onTextureSampler(int) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};
#endif
