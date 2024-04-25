/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"

#include "include/private/base/SkTArray.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/FactoryFunctionsPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

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

// If all the options are null the span is considered empty
bool is_empty(SkSpan<const sk_sp<PrecompileColorFilter>> options) {
    if (options.empty()) {
        return true;
    }

    for (const auto& o : options) {
        if (o) {
            return false;
        }
    }

    return true;
}

} // anonymous namespace

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

sk_sp<PrecompileBlender> PrecompileBlenders::Arithmetic() {
    const SkRuntimeEffect* arithmeticEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kArithmetic);

    return MakePrecompileBlender(sk_ref_sp(arithmeticEffect));
}

//--------------------------------------------------------------------------------------------------
class PrecompileEmptyShader : public PrecompileShader {
public:
    PrecompileEmptyShader() {}

private:
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination == 0); // The empty shader only ever has one combination

        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
    }

};

sk_sp<PrecompileShader> PrecompileShaders::Empty() {
    return sk_make_sp<PrecompileEmptyShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompilePerlinNoiseShader : public PrecompileShader {
public:
    PrecompilePerlinNoiseShader() {}

private:
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination == 0); // The Perlin noise shader only ever has one combination

        // TODO: update PerlinNoiseShaderBlock so the NoiseData is optional
        static const PerlinNoiseShaderBlock::PerlinNoiseData kIgnoredNoiseData(
                PerlinNoiseShaderBlock::Type::kFractalNoise, { 0.0f, 0.0f }, 2, {1, 1});

        PerlinNoiseShaderBlock::AddBlock(keyContext, builder, gatherer, kIgnoredNoiseData);
    }

};

sk_sp<PrecompileShader> PrecompileShaders::MakeFractalNoise() {
    return sk_make_sp<PrecompilePerlinNoiseShader>();
}

sk_sp<PrecompileShader> PrecompileShaders::MakeTurbulence() {
    return sk_make_sp<PrecompilePerlinNoiseShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorShader : public PrecompileShader {
public:
    PrecompileColorShader() {}

    bool isConstant(int desiredCombination) const override {
        SkASSERT(desiredCombination == 0); // The color shader only ever has one combination
        return true;
    }

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

// The colorSpace is safe to ignore - it is just applied to the color and doesn't modify the
// generated program.
sk_sp<PrecompileShader> PrecompileShaders::Color(sk_sp<SkColorSpace>) {
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
class PrecompileCoordClampShader : public PrecompileShader {
public:
    PrecompileCoordClampShader(SkSpan<const sk_sp<PrecompileShader>> shaders)
            : fShaders(shaders.begin(), shaders.end()) {
        fNumShaderCombos = 0;
        for (const auto& s : fShaders) {
            fNumShaderCombos += s->numCombinations();
        }
    }

private:
    int numChildCombinations() const override {
        return fNumShaderCombos;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumShaderCombos);

        constexpr SkRect kIgnored { 0, 0, 256, 256 }; // ignored bc we're precompiling

        // TODO: update CoordClampShaderBlock so this is optional
        CoordClampShaderBlock::CoordClampData data(kIgnored);

        CoordClampShaderBlock::BeginBlock(keyContext, builder, gatherer, data);
            AddToKey<PrecompileShader>(keyContext, builder, gatherer, fShaders, desiredCombination);
        builder->endBlock();
    }

    std::vector<sk_sp<PrecompileShader>> fShaders;
    int fNumShaderCombos;
};

sk_sp<PrecompileShader> PrecompileShaders::CoordClamp(SkSpan<const sk_sp<PrecompileShader>> input) {
    return sk_make_sp<PrecompileCoordClampShader>(input);
}

//--------------------------------------------------------------------------------------------------
// TODO: Investigate the YUV-image use case
class PrecompileImageShader : public PrecompileShader {
public:
    PrecompileImageShader(SkEnumBitMask<PrecompileImageShaderFlags> flags) : fFlags(flags) {}

private:
    // The ImageShader has 3 potential sampling/tiling variants: hardware-tiled, shader-tiled and
    // cubic sampling (which always uses shader-tiling)
    inline static constexpr int kNumSamplingTilingCombos = 3;
    inline static constexpr int kCubicSampled = 2;
    inline static constexpr int kHWTiled      = 1;
    inline static constexpr int kShaderTiled  = 0;

    // There are also 2 potential alpha combinations: alpha-only and not-alpha-only
    inline static constexpr int kNumAlphaCombinations = 2;
    inline static constexpr int kAlphaOnly    = 1;
    inline static constexpr int kNonAlphaOnly = 0;

    int numIntrinsicCombinations() const override {
        int numSamplingTilingCombos =
                (fFlags & PrecompileImageShaderFlags::kExcludeCubic) ? 2 : kNumSamplingTilingCombos;

        if (fFlags & PrecompileImageShaderFlags::kExcludeAlpha) {
            // RawImageShaders don't blend alpha-only images w/ the paint color
            return numSamplingTilingCombos;
        }
        return numSamplingTilingCombos * kNumAlphaCombinations;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numIntrinsicCombinations());

        int desiredAlphaCombo, desiredSamplingTilingCombo;

        if (fFlags & PrecompileImageShaderFlags::kExcludeAlpha) {
            desiredAlphaCombo = kNonAlphaOnly;
            desiredSamplingTilingCombo = desiredCombination;
        } else {
            desiredAlphaCombo = desiredCombination % kNumAlphaCombinations;
            desiredSamplingTilingCombo = desiredCombination / kNumAlphaCombinations;
        }
        SkDEBUGCODE(int numSamplingTilingCombos =
            (fFlags & PrecompileImageShaderFlags::kExcludeCubic) ? 2 : kNumSamplingTilingCombos;)
        SkASSERT(desiredSamplingTilingCombo < numSamplingTilingCombos);

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;
        constexpr ReadSwizzle kIgnoredSwizzle = ReadSwizzle::kRGBA;

        // ImageShaderBlock will use hardware tiling when the subset covers the entire image, so we
        // create subset + image size combinations where subset == imgSize (for a shader that uses
        // hardware tiling) and subset < imgSize (for a shader that does shader-based tiling).
        static constexpr SkRect kSubset = SkRect::MakeWH(1.0f, 1.0f);
        static constexpr SkISize kHWTileableSize = SkISize::Make(1, 1);
        static constexpr SkISize kShaderTileableSize = SkISize::Make(2, 2);

        ImageShaderBlock::ImageData imgData(
                desiredSamplingTilingCombo == kCubicSampled ? kDefaultCubicSampling
                                                            : kDefaultSampling,
                SkTileMode::kClamp, SkTileMode::kClamp,
                desiredSamplingTilingCombo == kHWTiled ? kHWTileableSize : kShaderTileableSize,
                kSubset, kIgnoredSwizzle);

        if (desiredAlphaCombo == kAlphaOnly) {
            SkASSERT(!(fFlags & PrecompileImageShaderFlags::kExcludeAlpha));

            Blend(keyContext, builder, gatherer,
                  /* addBlendToKey= */ [&] () -> void {
                      AddKnownModeBlend(keyContext, builder, gatherer, SkBlendMode::kDstIn);
                  },
                  /* addSrcToKey= */ [&] () -> void {
                      ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
                  },
                  /* addDstToKey= */ [&]() -> void {
                      RGBPaintColorBlock::AddBlock(keyContext, builder, gatherer);
                  });
        } else {
            ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
        }
    }

    SkEnumBitMask<PrecompileImageShaderFlags> fFlags;
};

sk_sp<PrecompileShader> PrecompileShaders::Image() {
    return PrecompileShaders::LocalMatrix(
            { sk_make_sp<PrecompileImageShader>(PrecompileImageShaderFlags::kNone) });
}

sk_sp<PrecompileShader> PrecompileShaders::RawImage() {
    // Raw images do not perform color space conversion, but in Graphite, this is represented as
    // an identity color space xform, not as a distinct shader
    return PrecompileShaders::LocalMatrix(
            { sk_make_sp<PrecompileImageShader>(PrecompileImageShaderFlags::kExcludeAlpha) });
}

sk_sp<PrecompileShader> PrecompileShadersPriv::Image(
        SkEnumBitMask<PrecompileImageShaderFlags> flags) {
    return PrecompileShaders::LocalMatrix({ sk_make_sp<PrecompileImageShader>(flags) });
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
// The PictureShader ultimately turns into an SkImageShader optionally wrapped in a
// LocalMatrixShader. The PrecompileImageShader already captures that use case so just reuse it.
// Note that this means each precompile PictureShader will add 24 combinations:
//    2 (pictureshader LM) x 2 (imageShader LM) x 6 (imageShader variations)
sk_sp<PrecompileShader> PrecompileShaders::Picture() {
    // Note: We don't need to consider the PrecompileYUVImageShader since the image
    // being drawn was created internally by Skia (as non-YUV).
    return PrecompileShadersPriv::LocalMatrixBothVariants({ PrecompileShaders::Image() });
}

sk_sp<PrecompileShader> PrecompileShadersPriv::Picture(bool withLM) {
    sk_sp<PrecompileShader> s = PrecompileShaders::Image();
    if (withLM) {
        return PrecompileShaders::LocalMatrix({ std::move(s) });
    }
    return s;
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
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kLinear);
    return PrecompileShadersPriv::LocalMatrixBothVariants({ std::move(s) });
}

sk_sp<PrecompileShader> PrecompileShadersPriv::LinearGradient(bool withLM) {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kLinear);
    if (withLM) {
        return PrecompileShaders::LocalMatrix({ std::move(s) });
    }
    return s;
}

sk_sp<PrecompileShader> PrecompileShaders::RadialGradient() {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kRadial);
    return PrecompileShadersPriv::LocalMatrixBothVariants({ std::move(s) });
}

sk_sp<PrecompileShader> PrecompileShadersPriv::RadialGradient(bool withLM) {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kRadial);
    if (withLM) {
        return PrecompileShaders::LocalMatrix({ std::move(s) });
    }
    return s;
}

sk_sp<PrecompileShader> PrecompileShaders::SweepGradient() {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kSweep);
    return PrecompileShadersPriv::LocalMatrixBothVariants({ std::move(s) });
}

sk_sp<PrecompileShader> PrecompileShadersPriv::SweepGradient(bool withLM) {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kSweep);
    if (withLM) {
        return PrecompileShaders::LocalMatrix({ std::move(s) });
    }
    return s;
}

sk_sp<PrecompileShader> PrecompileShaders::TwoPointConicalGradient() {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kConical);
    return PrecompileShadersPriv::LocalMatrixBothVariants({ std::move(s) });
}

sk_sp<PrecompileShader> PrecompileShadersPriv::TwoPointConicalGradient(bool withLM) {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kConical);
    if (withLM) {
        return PrecompileShaders::LocalMatrix({ std::move(s) });
    }
    return s;
}

//--------------------------------------------------------------------------------------------------
// In the main Skia API the SkLocalMatrixShader is optimized away when the LM is the identity
// or omitted. The PrecompileLocalMatrixShader captures this by adding two intrinsic options.
// One with the LMShader wrapping the child and one without the LMShader.
class PrecompileLocalMatrixShader : public PrecompileShader {
public:
    enum class Flags {
        kNone,
        kIncludeWithOutVariant,
    };

    PrecompileLocalMatrixShader(SkSpan<const sk_sp<PrecompileShader>> wrapped,
                                Flags flags = Flags::kNone)
            : fWrapped(wrapped.begin(), wrapped.end())
            , fFlags(flags) {
        fNumWrappedCombos = 0;
        for (const auto& s : fWrapped) {
            fNumWrappedCombos += s->numCombinations();
        }
    }

    bool isConstant(int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        /*
         * Regardless of whether the LocalMatrixShader elides itself or not, we always want
         * the Constantness of the wrapped shader.
         */
        int desiredWrappedCombination = desiredCombination / kNumIntrinsicCombinations;
        SkASSERT(desiredWrappedCombination < fNumWrappedCombos);

        auto wrapped = PrecompileBase::SelectOption(SkSpan(fWrapped), desiredWrappedCombination);
        if (wrapped.first) {
            return wrapped.first->isConstant(wrapped.second);
        }

        return false;
    }

private:
    // The LocalMatrixShader has two potential variants: with and without the LocalMatrixShader
    inline static constexpr int kNumIntrinsicCombinations = 2;
    inline static constexpr int kWithLocalMatrix    = 1;
    inline static constexpr int kWithoutLocalMatrix = 0;

    bool isALocalMatrixShader() const override { return true; }

    int numIntrinsicCombinations() const override {
        if (fFlags != Flags::kIncludeWithOutVariant) {
            return 1;   // just kWithLocalMatrix
        }
        return kNumIntrinsicCombinations;
    }

    int numChildCombinations() const override { return fNumWrappedCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        int desiredLMCombination, desiredWrappedCombination;

        if (fFlags != Flags::kIncludeWithOutVariant) {
            desiredLMCombination = kWithLocalMatrix;
            desiredWrappedCombination = desiredCombination;
        } else {
            desiredLMCombination = desiredCombination % kNumIntrinsicCombinations;
            desiredWrappedCombination = desiredCombination / kNumIntrinsicCombinations;
        }
        SkASSERT(desiredWrappedCombination < fNumWrappedCombos);

        if (desiredLMCombination == kWithLocalMatrix) {
            LocalMatrixShaderBlock::LMShaderData kIgnoredLMShaderData(SkMatrix::I());

            LocalMatrixShaderBlock::BeginBlock(keyContext, builder, gatherer, kIgnoredLMShaderData);
        }

            AddToKey<PrecompileShader>(keyContext, builder, gatherer, fWrapped,
                                       desiredWrappedCombination);

        if (desiredLMCombination == kWithLocalMatrix) {
            builder->endBlock();
        }
    }

    std::vector<sk_sp<PrecompileShader>> fWrapped;
    int fNumWrappedCombos;
    Flags fFlags;
};

sk_sp<PrecompileShader> PrecompileShaders::LocalMatrix(
        SkSpan<const sk_sp<PrecompileShader>> wrapped) {
    return sk_make_sp<PrecompileLocalMatrixShader>(std::move(wrapped));
}

sk_sp<PrecompileShader> PrecompileShadersPriv::LocalMatrixBothVariants(
        SkSpan<const sk_sp<PrecompileShader>> wrapped) {
    return sk_make_sp<PrecompileLocalMatrixShader>(
            std::move(wrapped),
            PrecompileLocalMatrixShader::Flags::kIncludeWithOutVariant);
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorFilterShader : public PrecompileShader {
public:
    PrecompileColorFilterShader(SkSpan<const sk_sp<PrecompileShader>> shaders,
                                SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters)
            : fShaders(shaders.begin(), shaders.end())
            , fColorFilters(colorFilters.begin(), colorFilters.end()) {
        fNumShaderCombos = 0;
        for (const auto& s : fShaders) {
            fNumShaderCombos += s->numCombinations();
        }
        fNumColorFilterCombos = 0;
        for (const auto& cf : fColorFilters) {
            fNumColorFilterCombos += cf->numCombinations();
        }
    }

private:
    int numChildCombinations() const override { return fNumShaderCombos * fNumColorFilterCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        int desiredShaderCombination = desiredCombination % fNumShaderCombos;
        int desiredColorFilterCombination = desiredCombination / fNumShaderCombos;
        SkASSERT(desiredColorFilterCombination < fNumColorFilterCombos);

        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    AddToKey<PrecompileShader>(keyContext, builder, gatherer, fShaders,
                                               desiredShaderCombination);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddToKey<PrecompileColorFilter>(keyContext, builder, gatherer, fColorFilters,
                                                    desiredColorFilterCombination);
                });
    }

    std::vector<sk_sp<PrecompileShader>>      fShaders;
    std::vector<sk_sp<PrecompileColorFilter>> fColorFilters;
    int fNumShaderCombos;
    int fNumColorFilterCombos;
};

sk_sp<PrecompileShader> PrecompileShaders::ColorFilter(
        SkSpan<const sk_sp<PrecompileShader>> shaders,
        SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters) {
    return sk_make_sp<PrecompileColorFilterShader>(std::move(shaders), std::move(colorFilters));
}

//--------------------------------------------------------------------------------------------------
class PrecompileWorkingColorSpaceShader : public PrecompileShader {
public:
    PrecompileWorkingColorSpaceShader(SkSpan<const sk_sp<PrecompileShader>> shaders,
                                      SkSpan<const sk_sp<SkColorSpace>> colorSpaces)
            : fShaders(shaders.begin(), shaders.end())
            , fColorSpaces(colorSpaces.begin(), colorSpaces.end()) {
        fNumShaderCombos = 0;
        for (const auto& s : fShaders) {
            fNumShaderCombos += s->numCombinations();
        }
    }

private:
    int numChildCombinations() const override { return fNumShaderCombos * fColorSpaces.size(); }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        int desiredShaderCombination = desiredCombination % fNumShaderCombos;
        int desiredColorSpaceCombination = desiredCombination / fNumShaderCombos;
        SkASSERT(desiredColorSpaceCombination < (int) fColorSpaces.size());

        const SkColorInfo& dstInfo = keyContext.dstColorInfo();
        const SkAlphaType dstAT = dstInfo.alphaType();
        sk_sp<SkColorSpace> dstCS = dstInfo.refColorSpace();
        if (!dstCS) {
            dstCS = SkColorSpace::MakeSRGB();
        }

        sk_sp<SkColorSpace> workingCS = fColorSpaces[desiredColorSpaceCombination];
        SkColorInfo workingInfo(dstInfo.colorType(), dstAT, workingCS);
        KeyContextWithColorInfo workingContext(keyContext, workingInfo);

        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    AddToKey<PrecompileShader>(keyContext, builder, gatherer, fShaders,
                                               desiredShaderCombination);
                },
                /* addOuterToKey= */ [&]() -> void {
                    ColorSpaceTransformBlock::ColorSpaceTransformData data(
                            workingCS.get(), dstAT, dstCS.get(), dstAT);
                    ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, data);
                });
    }

    std::vector<sk_sp<PrecompileShader>> fShaders;
    std::vector<sk_sp<SkColorSpace>>     fColorSpaces;
    int fNumShaderCombos;
};

sk_sp<PrecompileShader> PrecompileShaders::WorkingColorSpace(
        SkSpan<const sk_sp<PrecompileShader>> shaders,
        SkSpan<const sk_sp<SkColorSpace>> colorSpaces) {
    return sk_make_sp<PrecompileWorkingColorSpaceShader>(std::move(shaders),
                                                         std::move(colorSpaces));
}

//--------------------------------------------------------------------------------------------------
// In Graphite this acts as a non-elidable LocalMatrixShader
class PrecompileCTMShader : public PrecompileShader {
public:
    PrecompileCTMShader(SkSpan<const sk_sp<PrecompileShader>> wrapped)
            : fWrapped(wrapped.begin(), wrapped.end()) {
        fNumWrappedCombos = 0;
        for (const auto& s : fWrapped) {
            fNumWrappedCombos += s->numCombinations();
        }
    }

    bool isConstant(int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumWrappedCombos);

        auto wrapped = PrecompileBase::SelectOption(SkSpan(fWrapped), desiredCombination);
        if (wrapped.first) {
            return wrapped.first->isConstant(wrapped.second);
        }

        return false;
    }

private:
    int numChildCombinations() const override { return fNumWrappedCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumWrappedCombos);

        LocalMatrixShaderBlock::LMShaderData kIgnoredLMShaderData(SkMatrix::I());

        LocalMatrixShaderBlock::BeginBlock(keyContext, builder, gatherer, kIgnoredLMShaderData);

            AddToKey<PrecompileShader>(keyContext, builder, gatherer, fWrapped, desiredCombination);

        builder->endBlock();
    }

    std::vector<sk_sp<PrecompileShader>> fWrapped;
    int fNumWrappedCombos;
};

sk_sp<PrecompileShader> PrecompileShadersPriv::CTM(SkSpan<const sk_sp<PrecompileShader>> wrapped) {
    return sk_make_sp<PrecompileCTMShader>(std::move(wrapped));
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlurShader : public PrecompileShader {
public:
    PrecompileBlurShader(sk_sp<PrecompileShader> wrapped)
            : fWrapped(std::move(wrapped)) {
        fNumWrappedCombos = fWrapped->numCombinations();
    }

private:
    // 6 known 1D blur effects + 6 known 2D blur effects
    inline static constexpr int kNumIntrinsicCombinations = 12;

    int numIntrinsicCombinations() const override { return kNumIntrinsicCombinations; }

    int numChildCombinations() const override { return fNumWrappedCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        using namespace SkKnownRuntimeEffects;

        int desiredBlurCombination = desiredCombination % kNumIntrinsicCombinations;
        int desiredWrappedCombination = desiredCombination / kNumIntrinsicCombinations;
        SkASSERT(desiredWrappedCombination < fNumWrappedCombos);

        static const StableKey kIDs[kNumIntrinsicCombinations] = {
                StableKey::k1DBlur4,  StableKey::k1DBlur8,  StableKey::k1DBlur12,
                StableKey::k1DBlur16, StableKey::k1DBlur20, StableKey::k1DBlur28,

                StableKey::k2DBlur4,  StableKey::k2DBlur8,  StableKey::k2DBlur12,
                StableKey::k2DBlur16, StableKey::k2DBlur20, StableKey::k2DBlur28,
        };

        const SkRuntimeEffect* fEffect = GetKnownRuntimeEffect(kIDs[desiredBlurCombination]);

        KeyContextWithScope childContext(keyContext, KeyContext::Scope::kRuntimeEffect);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, { sk_ref_sp(fEffect) });
            fWrapped->priv().addToKey(childContext, builder, gatherer, desiredWrappedCombination);
        builder->endBlock();
    }

    sk_sp<PrecompileShader> fWrapped;
    int fNumWrappedCombos;
};

sk_sp<PrecompileShader> PrecompileShadersPriv::Blur(sk_sp<PrecompileShader> wrapped) {
    return sk_make_sp<PrecompileBlurShader>(std::move(wrapped));
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

sk_sp<PrecompileColorFilter> PrecompileColorFiltersPriv::ColorSpaceXform() {
    return sk_make_sp<PrecompileColorSpaceXformColorFilter>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileColorFilter> PrecompileColorFilters::Luma() {
    const SkRuntimeEffect* lumaEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kLuma);

    return MakePrecompileColorFilter(sk_ref_sp(lumaEffect));
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
    if (is_empty(outerOptions) && is_empty(innerOptions)) {
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

sk_sp<PrecompileColorFilter> PrecompileColorFiltersPriv::Gaussian() {
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
sk_sp<PrecompileColorFilter> PrecompileColorFilters::Lerp(
        SkSpan<const sk_sp<PrecompileColorFilter>> dstOptions,
        SkSpan<const sk_sp<PrecompileColorFilter>> srcOptions) {

    if (dstOptions.empty() && srcOptions.empty()) {
        return nullptr;
    }

    const SkRuntimeEffect* lerpEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kLerp);

    // Since the RuntimeEffect Precompile objects behave differently we have to manually create
    // all the combinations here (b/332690425).
    skia_private::TArray<std::array<const PrecompileChildPtr, 2>> combos;
    combos.reserve(dstOptions.size() * srcOptions.size());
    for (const sk_sp<PrecompileColorFilter>& d : dstOptions) {
        for (const sk_sp<PrecompileColorFilter>& s : srcOptions) {
            combos.push_back({ d, s });
        }
    }
    skia_private::TArray<SkSpan<const PrecompileChildPtr>> comboSpans;
    comboSpans.reserve(combos.size());
    for (const std::array<const PrecompileChildPtr, 2>& combo : combos) {
        comboSpans.push_back({ combo });
    }

    return MakePrecompileColorFilter(sk_ref_sp(lerpEffect), comboSpans);
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

sk_sp<PrecompileColorFilter> PrecompileColorFiltersPriv::WithWorkingFormat(
        SkSpan<const sk_sp<PrecompileColorFilter>> childOptions) {
    return sk_make_sp<PrecompileWithWorkingFormatColorFilter>(childOptions);
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileShader> s) : fChild(std::move(s)) {}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileColorFilter> cf)
        : fChild(std::move(cf)) {
}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileBlender> b) : fChild(std::move(b)) {}

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
