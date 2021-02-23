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
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

class GrTextureEffect : public GrFragmentProcessor {
public:
    static constexpr float kDefaultBorder[4] = {0};

    /** Make from a filter. The sampler will be configured with clamp mode. */
    static std::unique_ptr<GrFragmentProcessor> Make(
            GrSurfaceProxyView,
            SkAlphaType,
            const SkMatrix& = SkMatrix::I(),
            GrSamplerState::Filter = GrSamplerState::Filter::kNearest,
            GrSamplerState::MipmapMode mipmapMode = GrSamplerState::MipmapMode::kNone);

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
     * avoid allowing linear filtering to read outside the texel window. However, if MIP
     * filtering is used and a shader invocation reads from a level other than the base
     * then it may read texel values that were computed from in part from base level texels
     * outside the window. More specifically, we treat the MIP map case exactly like the
     * linear case in terms of how the final texture coords are computed.
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

    /**
     * Like MakeSubset() but always uses kLinear filtering. MakeSubset() uses the subset rect
     * dimensions to determine the period of the wrap mode (for repeat and mirror). Once it computes
     * the wrapped texture coordinate inside subset rect it further clamps it to a 0.5 inset rect of
     * subset. When subset is an integer rectangle this clamping avoids the hw linear filtering from
     * reading texels just outside the subset rect. This factory allows a custom inset clamping
     * distance rather than 0.5, allowing those neighboring texels to influence the linear filtering
     * sample result. If there is a known restriction on the post-matrix texture coords it can be
     * specified using domain.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeCustomLinearFilterInset(
            GrSurfaceProxyView,
            SkAlphaType,
            const SkMatrix&,
            GrSamplerState::WrapMode wx,
            GrSamplerState::WrapMode wy,
            const SkRect& subset,
            const SkRect* domain,
            SkVector inset,
            const GrCaps& caps,
            const float border[4] = kDefaultBorder);

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const char* name() const override { return "TextureEffect"; }

    GrSamplerState samplerState() const { return fSamplerState; }

    GrTexture* texture() const { return fView.asTextureProxy()->peekTexture(); }

    const GrSurfaceProxyView& view() const { return fView; }

    class Impl : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs&) override;
        void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

        void setSamplerHandle(GrGLSLShaderBuilder::SamplerHandle handle) {
            fSamplerHandle = handle;
        }

    private:
        UniformHandle fSubsetUni;
        UniformHandle fClampUni;
        UniformHandle fNormUni;
        UniformHandle fBorderUni;
        GrGLSLShaderBuilder::SamplerHandle fSamplerHandle;
    };

private:
    struct Sampling;

    /**
     * Possible implementation of wrap mode in shader code. Some modes are specialized by
     * filter.
     */
    enum class ShaderMode : uint16_t {
        kNone,                   // Using HW mode
        kClamp,                  // Shader based clamp, no filter specialization
        kRepeat_Nearest_None,    // Simple repeat for nearest sampling, no mipmapping
        kRepeat_Linear_None,     // Filter the subset boundary for kRepeat mode, no mip mapping
        kRepeat_Linear_Mipmap,   // Logic for linear filtering and LOD selection with kRepeat mode.
        kRepeat_Nearest_Mipmap,  // Logic for nearest filtering and LOD selection with kRepeat mode.
        kMirrorRepeat,           // Mirror repeat (doesn't depend on filter))
        kClampToBorder_Nearest,  // Logic for hard transition to border color when not filtering.
        kClampToBorder_Filter,   // Logic for fading to border color when filtering.
    };
    static ShaderMode GetShaderMode(GrSamplerState::WrapMode,
                                    GrSamplerState::Filter,
                                    GrSamplerState::MipmapMode);
    static bool ShaderModeIsClampToBorder(ShaderMode);

    GrSurfaceProxyView fView;
    GrSamplerState fSamplerState;
    float fBorder[4];
    SkRect fSubset;
    SkRect fClamp;
    ShaderMode fShaderModes[2];
    // true if we are dealing with a fully lazy proxy which can't be normalized until runtime
    bool fLazyProxyNormalization;

    inline GrTextureEffect(GrSurfaceProxyView, SkAlphaType, const Sampling&, bool);

    explicit GrTextureEffect(const GrTextureEffect& src);

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    bool hasClampToBorderShaderMode() const {
        return ShaderModeIsClampToBorder(fShaderModes[0]) ||
               ShaderModeIsClampToBorder(fShaderModes[1]);
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};
#endif
