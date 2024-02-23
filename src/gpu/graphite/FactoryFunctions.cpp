/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"

#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
class PrecompileBlendModeBlender : public PrecompileBlender {
public:
    PrecompileBlendModeBlender(SkBlendMode blendMode) : fBlendMode(blendMode) {}

    std::optional<SkBlendMode> asBlendMode() const final { return fBlendMode; }

private:
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination == 0); // The blend mode blender only ever has one combination

        AddModeBlend(keyContext, builder, gatherer, fBlendMode);
    }


    SkBlendMode fBlendMode;
};

sk_sp<PrecompileBlender> PrecompileBlender::Mode(SkBlendMode blendMode) {
    return sk_make_sp<PrecompileBlendModeBlender>(blendMode);
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorShader : public PrecompileShader {
public:
    PrecompileColorShader() {}

    bool isConstant() const override { return true; }

private:
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination == 0); // The color shader only ever has one combination

        // The white PMColor is just a placeholder for the actual paint params color
        SolidColorShaderBlock::AddBlock(keyContext, builder, gatherer, SK_PMColor4fWHITE);
    }

};

sk_sp<PrecompileShader> PrecompileShaders::Color() {
    return sk_make_sp<PrecompileColorShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlendShader : public PrecompileShader {
public:
    PrecompileBlendShader(SkSpan<const sk_sp<PrecompileBlender>> runtimeBlendEffects,
                          SkSpan<const sk_sp<PrecompileShader>> dsts,
                          SkSpan<const sk_sp<PrecompileShader>> srcs,
                          bool needsPorterDuffBased,
                          bool needsSeparableMode)
            : fRuntimeBlendEffects(runtimeBlendEffects.begin(), runtimeBlendEffects.end())
            , fDstOptions(dsts.begin(), dsts.end())
            , fSrcOptions(srcs.begin(), srcs.end()) {

        fNumBlenderCombos = 0;
        for (const auto& rt : fRuntimeBlendEffects) {
            fNumBlenderCombos += rt->numCombinations();
        }
        if (needsPorterDuffBased) {
            ++fNumBlenderCombos;
        }
        if (needsSeparableMode) {
            ++fNumBlenderCombos;
        }

        SkASSERT(fNumBlenderCombos >= 1);

        fNumDstCombos = 0;
        for (const auto& d : fDstOptions) {
            fNumDstCombos += d->numCombinations();
        }

        fNumSrcCombos = 0;
        for (const auto& s : fSrcOptions) {
            fNumSrcCombos += s->numCombinations();
        }

        if (needsPorterDuffBased) {
            fPorterDuffIndex = 0;
            if (needsSeparableMode) {
                fSeparableModeIndex = 1;
                if (!fRuntimeBlendEffects.empty()) {
                    fBlenderIndex = 2;
                }
            } else if (!fRuntimeBlendEffects.empty()) {
                fBlenderIndex = 1;
            }
        } else if (needsSeparableMode) {
            fSeparableModeIndex = 0;
            if (!fRuntimeBlendEffects.empty()) {
                fBlenderIndex = 1;
            }
        } else {
            SkASSERT(!fRuntimeBlendEffects.empty());
            fBlenderIndex = 0;
        }
    }

private:
    int numChildCombinations() const override {
        return fNumBlenderCombos * fNumDstCombos * fNumSrcCombos;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        const int desiredDstCombination = desiredCombination % fNumDstCombos;
        int remainingCombinations = desiredCombination / fNumDstCombos;

        const int desiredSrcCombination = remainingCombinations % fNumSrcCombos;
        remainingCombinations /= fNumSrcCombos;

        int desiredBlendCombination = remainingCombinations;
        SkASSERT(desiredBlendCombination < fNumBlenderCombos);

        if (desiredBlendCombination == fPorterDuffIndex ||
            desiredBlendCombination == fSeparableModeIndex) {
            BlendShaderBlock::BeginBlock(keyContext, builder, gatherer);

        } else {
            const SkRuntimeEffect* blendEffect =
                    GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kBlend);

            RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                           { sk_ref_sp(blendEffect) });
            SkASSERT(desiredBlendCombination >= fBlenderIndex);
            desiredBlendCombination -= fBlenderIndex;
        }

        AddToKey<PrecompileShader>(keyContext, builder, gatherer, fSrcOptions,
                                   desiredSrcCombination);
        AddToKey<PrecompileShader>(keyContext, builder, gatherer, fDstOptions,
                                   desiredDstCombination);

        if (desiredBlendCombination == fPorterDuffIndex) {
            CoeffBlenderBlock::AddBlock(keyContext, builder, gatherer,
                                        { 0.0f, 0.0f, 0.0f, 0.0f }); // coeffs aren't used
        } else if (desiredBlendCombination == fSeparableModeIndex) {
            BlendModeBlenderBlock::AddBlock(keyContext, builder, gatherer,
                                            SkBlendMode::kOverlay); // the blendmode is unused
        } else {
            AddToKey<PrecompileBlender>(keyContext, builder, gatherer, fRuntimeBlendEffects,
                                        desiredBlendCombination);
        }

        builder->endBlock();  // BlendShaderBlock or RuntimeEffectBlock
    }

    std::vector<sk_sp<PrecompileBlender>> fRuntimeBlendEffects;
    std::vector<sk_sp<PrecompileShader>> fDstOptions;
    std::vector<sk_sp<PrecompileShader>> fSrcOptions;

    int fNumBlenderCombos;
    int fNumDstCombos;
    int fNumSrcCombos;

    int fPorterDuffIndex = -1;
    int fSeparableModeIndex = -1;
    int fBlenderIndex = -1;
};

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<const sk_sp<PrecompileBlender>> blenders,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {
    std::vector<sk_sp<PrecompileBlender>> tmp;
    tmp.reserve(blenders.size());

    bool needsPorterDuffBased = false;
    bool needsBlendModeBased = false;

    for (const auto& b : blenders) {
        if (!b) {
            needsPorterDuffBased = true; // fall back to kSrcOver
        } else if (b->asBlendMode().has_value()) {
            SkBlendMode bm = b->asBlendMode().value();

            SkSpan<const float> coeffs = skgpu::GetPorterDuffBlendConstants(bm);
            if (!coeffs.empty()) {
                needsPorterDuffBased = true;
            } else {
                needsBlendModeBased = true;
            }
        } else {
            tmp.push_back(b);
        }
    }

    if (!needsPorterDuffBased && !needsBlendModeBased && tmp.empty()) {
        needsPorterDuffBased = true; // fallback to kSrcOver
    }

    return sk_make_sp<PrecompileBlendShader>(SkSpan<const sk_sp<PrecompileBlender>>(tmp),
                                             dsts, srcs,
                                             needsPorterDuffBased, needsBlendModeBased);
}

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<SkBlendMode> blendModes,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {

    bool needsPorterDuffBased = false;
    bool needsBlendModeBased = false;

    for (SkBlendMode bm : blendModes) {
        SkSpan<const float> porterDuffConstants = skgpu::GetPorterDuffBlendConstants(bm);
        if (!porterDuffConstants.empty()) {
            needsPorterDuffBased = true;
        } else {
            needsBlendModeBased = true;
        }
    }

    if (!needsPorterDuffBased && !needsBlendModeBased) {
        needsPorterDuffBased = true; // fallback to kSrcOver
    }

    return sk_make_sp<PrecompileBlendShader>(SkSpan<const sk_sp<PrecompileBlender>>(),
                                             dsts, srcs,
                                             needsPorterDuffBased, needsBlendModeBased);
}

//--------------------------------------------------------------------------------------------------
class PrecompileImageShader : public PrecompileShader {
public:
    PrecompileImageShader() {}

private:
    // hardware-tiled, shader-tiled and cubic sampling
    inline static constexpr int kNumIntrinsicCombinations = 3;

    int numIntrinsicCombinations() const override { return kNumIntrinsicCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < kNumIntrinsicCombinations);

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;

        // ImageShaderBlock will use hardware tiling when the subset covers the entire image, so we
        // create subset + image size combinations where subset == imgSize (for a shader that uses
        // hardware tiling) and subset < imgSize (for a shader that does shader-based tiling).
        static constexpr SkRect kSubset = SkRect::MakeWH(1.0f, 1.0f);
        static constexpr SkISize kHwTileableSize = SkISize::Make(1, 1);
        static constexpr SkISize kNonHwTileableSize = SkISize::Make(2, 2);

        ImageShaderBlock::ImageData imgData(desiredCombination == 2 ? kDefaultCubicSampling
                                                                    : kDefaultSampling,
                                            SkTileMode::kClamp, SkTileMode::kClamp,
                                            desiredCombination == 1 ? kHwTileableSize
                                                                    : kNonHwTileableSize,
                                            kSubset, ReadSwizzle::kRGBA);

        ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
    }
};

sk_sp<PrecompileShader> PrecompileShaders::Image() {
    return sk_make_sp<PrecompileImageShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileYUVImageShader : public PrecompileShader {
public:
    PrecompileYUVImageShader() {}

private:
    // non-cubic and cubic sampling
    inline static constexpr int kNumIntrinsicCombinations = 2;

    int numIntrinsicCombinations() const override { return kNumIntrinsicCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < kNumIntrinsicCombinations);

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;

        YUVImageShaderBlock::ImageData imgData(desiredCombination == 1 ? kDefaultCubicSampling
                                                                       : kDefaultSampling,
                                               SkTileMode::kClamp, SkTileMode::kClamp,
                                               SkISize::MakeEmpty(), SkRect::MakeEmpty());

        YUVImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
    }
};

sk_sp<PrecompileShader> PrecompileShaders::YUVImage() {
    return sk_make_sp<PrecompileYUVImageShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileGradientShader : public PrecompileShader {
public:
    PrecompileGradientShader(SkShaderBase::GradientType type) : fType(type) {}

private:
    /*
     * The gradients currently have two specializations based on the number of stops.
     */
    inline static constexpr int kNumStopVariants = 2;
    inline static constexpr int kStopVariants[kNumStopVariants] = { 4, 8 };

    int numIntrinsicCombinations() const override {
        return kNumStopVariants;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        const int intrinsicCombination = desiredCombination / this->numChildCombinations();
        SkDEBUGCODE(int childCombination = desiredCombination % this->numChildCombinations();)
        SkASSERT(intrinsicCombination < kNumStopVariants);
        SkASSERT(childCombination == 0);

        GradientShaderBlocks::GradientData gradData(fType, kStopVariants[intrinsicCombination]);

        constexpr SkAlphaType kAlphaType = kPremul_SkAlphaType;
        ColorSpaceTransformBlock::ColorSpaceTransformData csData(sk_srgb_singleton(), kAlphaType,
                                                                 sk_srgb_singleton(), kAlphaType);

        // TODO: we may need SkLocalMatrixShader-wrapped versions too
        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    GradientShaderBlocks::AddBlock(keyContext, builder, gatherer, gradData);
                },
                /* addOuterToKey= */  [&]() -> void {
                    ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, csData);
                });
    }

    SkShaderBase::GradientType fType;
};

sk_sp<PrecompileShader> PrecompileShaders::LinearGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kLinear);
}

sk_sp<PrecompileShader> PrecompileShaders::RadialGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kRadial);
}

sk_sp<PrecompileShader> PrecompileShaders::SweepGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kSweep);
}

sk_sp<PrecompileShader> PrecompileShaders::TwoPointConicalGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kConical);
}

//--------------------------------------------------------------------------------------------------
class PrecompileLocalMatrixShader : public PrecompileShader {
public:
    PrecompileLocalMatrixShader(sk_sp<PrecompileShader> wrapped) : fWrapped(std::move(wrapped)) {}

private:
    bool isALocalMatrixShader() const override { return true; }

    int numChildCombinations() const override {
        return fWrapped->numCombinations();
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < fWrapped->numCombinations());

        LocalMatrixShaderBlock::LMShaderData lmShaderData(SkMatrix::I());

        LocalMatrixShaderBlock::BeginBlock(keyContext, builder, gatherer, lmShaderData);

            fWrapped->priv().addToKey(keyContext, builder, gatherer, desiredCombination);

        builder->endBlock();
    }

    sk_sp<PrecompileShader> fWrapped;
};

sk_sp<PrecompileShader> PrecompileShaders::LocalMatrix(sk_sp<PrecompileShader> wrapped) {
    return sk_make_sp<PrecompileLocalMatrixShader>(std::move(wrapped));
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorFilterShader : public PrecompileShader {
public:
    PrecompileColorFilterShader(sk_sp<PrecompileShader> shader, sk_sp<PrecompileColorFilter> cf)
            : fShader(std::move(shader))
            , fColorFilter(std::move(cf)) {}

private:
    int numChildCombinations() const override {
        const int numShaderCombos = fShader->numCombinations();
        const int numColorFilterCombos = fColorFilter->numCombinations();

        return numShaderCombos * numColorFilterCombos;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination < this->numCombinations());

        const int numShaderCombos = fShader->numCombinations();
        SkDEBUGCODE(int numColorFilterCombos = fColorFilter->numCombinations();)

        int desiredShaderCombination = desiredCombination % numShaderCombos;
        int desiredColorFilterCombination = desiredCombination / numShaderCombos;
        SkASSERT(desiredColorFilterCombination < numColorFilterCombos);

        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    fShader->priv().addToKey(keyContext, builder, gatherer,
                                             desiredShaderCombination);
                },
                /* addOuterToKey= */ [&]() -> void {
                    fColorFilter->priv().addToKey(keyContext, builder, gatherer,
                                                  desiredColorFilterCombination);
                });
    }

    sk_sp<PrecompileShader> fShader;
    sk_sp<PrecompileColorFilter> fColorFilter;
};

sk_sp<PrecompileShader> PrecompileShaders::ColorFilter(sk_sp<PrecompileShader> shader,
                                                       sk_sp<PrecompileColorFilter> cf) {
    return sk_make_sp<PrecompileColorFilterShader>(std::move(shader), std::move(cf));
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlurMaskFilter : public PrecompileMaskFilter {
public:
    PrecompileBlurMaskFilter() {}

private:
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination == 0);

        // TODO: need to add a BlurMaskFilter Block. This is somewhat blocked on figuring out
        // what we're going to do with the Blur system.
    }
};

sk_sp<PrecompileMaskFilter> PrecompileMaskFilters::Blur() {
    return sk_make_sp<PrecompileBlurMaskFilter>();
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
class PrecompileBlendModeColorFilter : public PrecompileColorFilter {
public:
    PrecompileBlendModeColorFilter() {}

private:
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination == 0);

        // Here, kSrcOver and the white color are just a stand-ins for some later blend mode
        // and color.
        AddBlendModeColorFilter(keyContext, builder, gatherer,
                                SkBlendMode::kSrcOver, SK_PMColor4fWHITE);
    }
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Blend() {
    return sk_make_sp<PrecompileBlendModeColorFilter>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorSpaceXformColorFilter : public PrecompileColorFilter {
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination == 0);

        constexpr SkAlphaType kAlphaType = kPremul_SkAlphaType;
        ColorSpaceTransformBlock::ColorSpaceTransformData csData(sk_srgb_singleton(), kAlphaType,
                                                                 sk_srgb_singleton(), kAlphaType);

        ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, csData);
    }
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::LinearToSRGBGamma() {
    return sk_make_sp<PrecompileColorSpaceXformColorFilter>();
}

sk_sp<PrecompileColorFilter> PrecompileColorFilters::SRGBToLinearGamma() {
    return sk_make_sp<PrecompileColorSpaceXformColorFilter>();
}

sk_sp<PrecompileColorFilter> PrecompileColorFilters::ColorSpaceXform() {
    return sk_make_sp<PrecompileColorSpaceXformColorFilter>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileComposeColorFilter : public PrecompileColorFilter {
public:
    PrecompileComposeColorFilter(SkSpan<const sk_sp<PrecompileColorFilter>> outerOptions,
                                 SkSpan<const sk_sp<PrecompileColorFilter>> innerOptions)
            : fOuterOptions(outerOptions.begin(), outerOptions.end())
            , fInnerOptions(innerOptions.begin(), innerOptions.end()) {

        fNumOuterCombos = 0;
        for (const auto& outerOption : fOuterOptions) {
            fNumOuterCombos += outerOption ? outerOption->numCombinations() : 1;
        }

        fNumInnerCombos = 0;
        for (const auto& innerOption : fInnerOptions) {
            fNumInnerCombos += innerOption ? innerOption->numCombinations() : 1;
        }
    }

private:
    int numChildCombinations() const override { return fNumOuterCombos * fNumInnerCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        const int desiredOuterCombination = desiredCombination % fNumOuterCombos;
        int remainingCombinations = desiredCombination / fNumOuterCombos;

        const int desiredInnerCombination = remainingCombinations % fNumInnerCombos;
        remainingCombinations /= fNumInnerCombos;

        SkASSERT(!remainingCombinations);

        sk_sp<PrecompileColorFilter> inner, outer;
        int innerChildOptions, outerChildOptions;

        std::tie(outer, outerChildOptions) = SelectOption<PrecompileColorFilter>(
                fOuterOptions, desiredOuterCombination);
        std::tie(inner, innerChildOptions) = SelectOption<PrecompileColorFilter>(
                fInnerOptions, desiredInnerCombination);

        if (!inner && !outer) {
            // A "passthrough" color filter returns the input color as-is.
            builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
        } else if (!inner) {
            outer->priv().addToKey(keyContext, builder, gatherer, outerChildOptions);
        } else if (!outer) {
            inner->priv().addToKey(keyContext, builder, gatherer, innerChildOptions);
        } else {
            Compose(keyContext, builder, gatherer,
                    /* addInnerToKey= */ [&]() -> void {
                        inner->priv().addToKey(keyContext, builder, gatherer, innerChildOptions);
                    },
                    /* addOuterToKey= */ [&]() -> void {
                        outer->priv().addToKey(keyContext, builder, gatherer, outerChildOptions);
                    });
        }
    }

    std::vector<sk_sp<PrecompileColorFilter>> fOuterOptions;
    std::vector<sk_sp<PrecompileColorFilter>> fInnerOptions;

    int fNumOuterCombos;
    int fNumInnerCombos;
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Compose(
        SkSpan<const sk_sp<PrecompileColorFilter>> outerOptions,
        SkSpan<const sk_sp<PrecompileColorFilter>> innerOptions) {
    if (outerOptions.empty() && innerOptions.empty()) {
        return nullptr;
    }

    return sk_make_sp<PrecompileComposeColorFilter>(outerOptions, innerOptions);
}

//--------------------------------------------------------------------------------------------------
class PrecompileGaussianColorFilter : public PrecompileColorFilter {
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination == 0);

        builder->addBlock(BuiltInCodeSnippetID::kGaussianColorFilter);
    }
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Gaussian() {
    return sk_make_sp<PrecompileGaussianColorFilter>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileMatrixColorFilter : public PrecompileColorFilter {
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination == 0);

        static constexpr float kIdentity[20] = { 1, 0, 0, 0, 0,
                                                 0, 1, 0, 0, 0,
                                                 0, 0, 1, 0, 0,
                                                 0, 0, 0, 1, 0 };

        MatrixColorFilterBlock::MatrixColorFilterData matrixCFData(kIdentity, /* inHSLA= */ false);

        MatrixColorFilterBlock::AddBlock(keyContext, builder, gatherer, matrixCFData);
    }
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Matrix() {
    return sk_make_sp<PrecompileMatrixColorFilter>();
}

sk_sp<PrecompileColorFilter> PrecompileColorFilters::HSLAMatrix() {
    return sk_make_sp<PrecompileMatrixColorFilter>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileTableColorFilter : public PrecompileColorFilter {
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination == 0);

        TableColorFilterBlock::TableColorFilterData data(/* proxy= */ nullptr);

        TableColorFilterBlock::AddBlock(keyContext, builder, gatherer, data);
    }
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Table() {
    return sk_make_sp<PrecompileTableColorFilter>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileColorFilter> PrecompileColorFilters::Lighting() {
    return PrecompileColorFilters::Matrix();
}

//--------------------------------------------------------------------------------------------------
class PrecompileWithWorkingFormatColorFilter : public PrecompileColorFilter {
public:
    PrecompileWithWorkingFormatColorFilter(SkSpan<const sk_sp<PrecompileColorFilter>> childOptions)
            : fChildOptions(childOptions.begin(), childOptions.end()) {

        fNumChildCombos = 0;
        for (const auto& childOption : fChildOptions) {
            fNumChildCombos += childOption->numCombinations();
        }
    }

private:
    int numChildCombinations() const override { return fNumChildCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumChildCombos);

        constexpr SkAlphaType kAlphaType = kPremul_SkAlphaType;
        ColorSpaceTransformBlock::ColorSpaceTransformData csData(sk_srgb_singleton(), kAlphaType,
                                                                 sk_srgb_singleton(), kAlphaType);

        // Use two nested compose blocks to chain (dst->working), child, and (working->dst) together
        // while appearing as one block to the parent node.
        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    // Inner compose
                    Compose(keyContext, builder, gatherer,
                            /* addInnerToKey= */ [&]() -> void {
                                // Innermost (inner of inner compose)
                                ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer,
                                                                   csData);
                            },
                            /* addOuterToKey= */ [&]() -> void {
                                // Middle (outer of inner compose)
                                AddToKey<PrecompileColorFilter>(keyContext, builder, gatherer,
                                                                fChildOptions, desiredCombination);
                            });
                },
                /* addOuterToKey= */ [&]() -> void {
                    // Outermost (outer of outer compose)
                    ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, csData);
                });
    }

    std::vector<sk_sp<PrecompileColorFilter>> fChildOptions;

    int fNumChildCombos;
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::WithWorkingFormat(
        SkSpan<const sk_sp<PrecompileColorFilter>> childOptions) {
    return sk_make_sp<PrecompileWithWorkingFormatColorFilter>(childOptions);
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
// TODO: need to figure out how we're going to decompose ImageFilters
sk_sp<PrecompileImageFilter> PrecompileImageFilters::Blur() {
    return nullptr; // sk_make_sp<PrecompileImageFilter>();
}

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Image() {
    return nullptr; // sk_make_sp<PrecompileImageFilter>();
}

//--------------------------------------------------------------------------------------------------
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileShader> s) : fChild(std::move(s)) {}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileColorFilter> cf)
        : fChild(std::move(cf)) {
}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileBlender> b) : fChild(std::move(b)) {}

namespace {

#ifdef SK_DEBUG

bool precompilebase_is_valid_as_child(const PrecompileBase *child) {
    if (!child) {
        return true;
    }

    switch (child->type()) {
        case PrecompileBase::Type::kShader:
        case PrecompileBase::Type::kColorFilter:
        case PrecompileBase::Type::kBlender:
            return true;
        default:
            return false;
    }
}

#endif // SK_DEBUG

} // anonymous namespace

PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileBase> child)
        : fChild(std::move(child)) {
    SkASSERT(precompilebase_is_valid_as_child(fChild.get()));
}

std::optional<SkRuntimeEffect::ChildType> PrecompileChildPtr::type() const {
    if (fChild) {
        switch (fChild->type()) {
            case PrecompileBase::Type::kShader:
                return SkRuntimeEffect::ChildType::kShader;
            case PrecompileBase::Type::kColorFilter:
                return SkRuntimeEffect::ChildType::kColorFilter;
            case PrecompileBase::Type::kBlender:
                return SkRuntimeEffect::ChildType::kBlender;
            default:
                break;
        }
    }
    return std::nullopt;
}

PrecompileShader* PrecompileChildPtr::shader() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kShader)
           ? static_cast<PrecompileShader*>(fChild.get())
           : nullptr;
}

PrecompileColorFilter* PrecompileChildPtr::colorFilter() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kColorFilter)
           ? static_cast<PrecompileColorFilter*>(fChild.get())
           : nullptr;
}

PrecompileBlender* PrecompileChildPtr::blender() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kBlender)
           ? static_cast<PrecompileBlender*>(fChild.get())
           : nullptr;
}

//--------------------------------------------------------------------------------------------------
namespace {

int num_options_in_set(const std::vector<PrecompileChildPtr>& optionSet) {
    int numOptions = 1;
    for (const PrecompileChildPtr& childOption : optionSet) {
        // A missing child will fall back to a passthrough object
        if (childOption.base()) {
            numOptions *= childOption.base()->numCombinations();
        }
    }

    return numOptions;
}

// This is the precompile correlate to KeyHelper.cpp's add_children_to_key
void add_children_to_key(const KeyContext& keyContext,
                         PaintParamsKeyBuilder* builder,
                         PipelineDataGatherer* gatherer,
                         int desiredCombination,
                         const std::vector<PrecompileChildPtr>& optionSet,
                         SkSpan<const SkRuntimeEffect::Child> childInfo) {
    using ChildType = SkRuntimeEffect::ChildType;

    SkASSERT(optionSet.size() == childInfo.size());

    KeyContextWithScope childContext(keyContext, KeyContext::Scope::kRuntimeEffect);

    int remainingCombinations = desiredCombination;

    for (size_t index = 0; index < optionSet.size(); ++index) {
        const PrecompileChildPtr& childOption = optionSet[index];

        const int numChildCombos = childOption.base() ? childOption.base()->numCombinations()
                                                      : 1;
        const int curCombo = remainingCombinations % numChildCombos;
        remainingCombinations /= numChildCombos;

        std::optional<ChildType> type = childOption.type();
        if (type == ChildType::kShader) {
            childOption.shader()->priv().addToKey(childContext, builder, gatherer, curCombo);
        } else if (type == ChildType::kColorFilter) {
            childOption.colorFilter()->priv().addToKey(childContext, builder, gatherer, curCombo);
        } else if (type == ChildType::kBlender) {
            childOption.blender()->priv().addToKey(childContext, builder, gatherer, curCombo);
        } else {
            SkASSERT(curCombo == 0);

            // We don't have a child effect. Substitute in a no-op effect.
            switch (childInfo[index].type) {
                case ChildType::kShader:
                    // A missing shader returns transparent black
                    SolidColorShaderBlock::AddBlock(childContext, builder, gatherer,
                                                    SK_PMColor4fTRANSPARENT);
                    break;

                case ChildType::kColorFilter:
                    // A "passthrough" shader returns the input color as-is.
                    builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
                    break;

                case ChildType::kBlender:
                    // A "passthrough" blender performs `blend_src_over(src, dest)`.
                    AddKnownModeBlend(childContext, builder, gatherer, SkBlendMode::kSrcOver);
                    break;
            }
        }
    }
}

} // anonymous namespace

template<typename T>
class PrecompileRTEffect : public T {
public:
    PrecompileRTEffect(sk_sp<SkRuntimeEffect> effect,
                       SkSpan<const PrecompileChildOptions> childOptions)
            : fEffect(std::move(effect)) {
        fChildOptions.reserve(childOptions.size());
        for (PrecompileChildOptions c : childOptions) {
            fChildOptions.push_back({ c.begin(), c.end() });
        }
    }

private:
    int numChildCombinations() const override {
        int numOptions = 0;
        for (const std::vector<PrecompileChildPtr>& optionSet : fChildOptions) {
            numOptions += num_options_in_set(optionSet);
        }

        return numOptions ? numOptions : 1;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination < this->numCombinations());

        SkSpan<const SkRuntimeEffect::Child> childInfo = fEffect->children();

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, { fEffect });

        for (const std::vector<PrecompileChildPtr>& optionSet : fChildOptions) {
            int numOptionsInSet = num_options_in_set(optionSet);

            if (desiredCombination < numOptionsInSet) {
                add_children_to_key(keyContext, builder, gatherer, desiredCombination, optionSet,
                                    childInfo);
                break;
            }

            desiredCombination -= numOptionsInSet;
        }

        builder->endBlock();
    }

    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::vector<PrecompileChildPtr>> fChildOptions;
};

sk_sp<PrecompileShader> MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowShader_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileShader>>(std::move(effect), childOptions);
}

sk_sp<PrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowColorFilter_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileColorFilter>>(std::move(effect), childOptions);
}

sk_sp<PrecompileBlender> MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowBlender_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileBlender>>(std::move(effect), childOptions);
}

} // namespace skgpu::graphite

//--------------------------------------------------------------------------------------------------
