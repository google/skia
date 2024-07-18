/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileShader.h"

#include "include/core/SkColorSpace.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShadersPriv.h"

namespace skgpu::graphite {

PrecompileShader::~PrecompileShader() = default;

sk_sp<PrecompileShader> PrecompileShader::makeWithColorFilter(
        sk_sp<PrecompileColorFilter> cf) const {
    if (!cf) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::ColorFilter({ sk_ref_sp(this) }, { std::move(cf) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithWorkingColorSpace(sk_sp<SkColorSpace> cs) const {
    if (!cs) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::WorkingColorSpace({ sk_ref_sp(this) }, { std::move(cs) });
}

//--------------------------------------------------------------------------------------------------
class PrecompileEmptyShader final : public PrecompileShader {
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
class PrecompileColorShader final : public PrecompileShader {
private:
    bool isConstant(int desiredCombination) const override {
        SkASSERT(desiredCombination == 0); // The color shader only ever has one combination
        return true;
    }

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
class PrecompileBlendShader final : public PrecompileShader {
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
            fNumBlenderCombos += rt->priv().numCombinations();
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
            fNumDstCombos += d->priv().numCombinations();
        }

        fNumSrcCombos = 0;
        for (const auto& s : fSrcOptions) {
            fNumSrcCombos += s->priv().numCombinations();
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
        } else if (b->priv().asBlendMode().has_value()) {
            SkBlendMode bm = b->priv().asBlendMode().value();

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
        SkSpan<const SkBlendMode> blendModes,
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
class PrecompileCoordClampShader final : public PrecompileShader {
public:
    PrecompileCoordClampShader(SkSpan<const sk_sp<PrecompileShader>> shaders)
            : fShaders(shaders.begin(), shaders.end()) {
        fNumShaderCombos = 0;
        for (const auto& s : fShaders) {
            fNumShaderCombos += s->priv().numCombinations();
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
class PrecompileImageShader final : public PrecompileShader {
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
                kSubset);
        ColorSpaceTransformBlock::ColorSpaceTransformData colorXformData(kIgnoredSwizzle);

        if (desiredAlphaCombo == kAlphaOnly) {
            SkASSERT(!(fFlags & PrecompileImageShaderFlags::kExcludeAlpha));

            Blend(keyContext, builder, gatherer,
                  /* addBlendToKey= */ [&] () -> void {
                      AddKnownModeBlend(keyContext, builder, gatherer, SkBlendMode::kDstIn);
                  },
                  /* addSrcToKey= */ [&] () -> void {
                      Compose(keyContext, builder, gatherer,
                              /* addInnerToKey= */ [&]() -> void {
                                  ImageShaderBlock::AddBlock(keyContext, builder, gatherer,
                                                             imgData);
                              },
                              /* addOuterToKey= */ [&]() -> void {
                                  ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer,
                                                                     colorXformData);
                              });
                  },
                  /* addDstToKey= */ [&]() -> void {
                      RGBPaintColorBlock::AddBlock(keyContext, builder, gatherer);
                  });
        } else {
            Compose(keyContext, builder, gatherer,
                    /* addInnerToKey= */ [&]() -> void {
                        ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
                    },
                    /* addOuterToKey= */ [&]() -> void {
                        ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer,
                                                           colorXformData);
                    });
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

sk_sp<PrecompileShader> PrecompileShadersPriv::RawImage(
        SkEnumBitMask<PrecompileImageShaderFlags> flags) {
    return PrecompileShaders::LocalMatrix(
            { sk_make_sp<PrecompileImageShader>(flags |
                                                PrecompileImageShaderFlags::kExcludeAlpha) });
}

//--------------------------------------------------------------------------------------------------
class PrecompileYUVImageShader : public PrecompileShader {
public:
    PrecompileYUVImageShader() {}

private:
    // There are 8 intrinsic YUV shaders:
    //  4 tiling modes
    //    HW tiling w/o swizzle
    //    HW tiling w/ swizzle
    //    cubic shader tiling
    //    non-cubic shader tiling
    //  crossed with two postambles:
    //    just premul
    //    full-blown colorSpace transform
    inline static constexpr int kNumTilingModes     = 4;
    inline static constexpr int kHWTiledNoSwizzle   = 3;
    inline static constexpr int kHWTiledWithSwizzle = 2;
    inline static constexpr int kCubicShaderTiled   = 1;
    inline static constexpr int kShaderTiled        = 0;

    inline static constexpr int kNumPostambles       = 2;
    inline static constexpr int kWithColorSpaceXform = 1;
    inline static constexpr int kJustPremul          = 0;

    inline static constexpr int kNumIntrinsicCombinations = kNumTilingModes * kNumPostambles;

    int numIntrinsicCombinations() const override { return kNumIntrinsicCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < kNumIntrinsicCombinations);

        int desiredPostamble = desiredCombination % kNumPostambles;
        int desiredTiling = desiredCombination / kNumPostambles;
        SkASSERT(desiredTiling < kNumTilingModes);

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;

        YUVImageShaderBlock::ImageData imgData(desiredTiling == kCubicShaderTiled
                                                                       ? kDefaultCubicSampling
                                                                       : kDefaultSampling,
                                               SkTileMode::kClamp, SkTileMode::kClamp,
                                               /* imgSize= */ { 1, 1 },
                                               /* subset= */ desiredTiling == kShaderTiled
                                                                     ? SkRect::MakeEmpty()
                                                                     : SkRect::MakeWH(1, 1));

        static constexpr SkV4 kRedChannel{ 1.f, 0.f, 0.f, 0.f };
        imgData.fChannelSelect[0] = kRedChannel;
        imgData.fChannelSelect[1] = kRedChannel;
        if (desiredTiling == kHWTiledNoSwizzle) {
            imgData.fChannelSelect[2] = kRedChannel;
        } else {
            // Having a non-red channel selector forces a swizzle
            imgData.fChannelSelect[2] = { 0.f, 1.f, 0.f, 0.f};
        }
        imgData.fChannelSelect[3] = kRedChannel;

        imgData.fYUVtoRGBMatrix.setAll(1, 0, 0, 0, 1, 0, 0, 0, 0);
        imgData.fYUVtoRGBTranslate = { 0, 0, 0 };

        ColorSpaceTransformBlock::ColorSpaceTransformData colorXformData(
                                      skgpu::graphite::ReadSwizzle::kRGBA);

        if (desiredPostamble == kJustPremul) {
            Compose(keyContext, builder, gatherer,
                    /* addInnerToKey= */ [&]() -> void {
                        YUVImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
                    },
                    /* addOuterToKey= */ [&]() -> void {
                        builder->addBlock(BuiltInCodeSnippetID::kPremulAlphaColorFilter);
                    });
        } else {
            Compose(keyContext, builder, gatherer,
                    /* addInnerToKey= */ [&]() -> void {
                        YUVImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
                    },
                    /* addOuterToKey= */ [&]() -> void {
                        ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer,
                                                           colorXformData);
                    });
        }

    }
};

sk_sp<PrecompileShader> PrecompileShaders::YUVImage() {
    return PrecompileShaders::LocalMatrix({ sk_make_sp<PrecompileYUVImageShader>() });
}

//--------------------------------------------------------------------------------------------------
class PrecompilePerlinNoiseShader final : public PrecompileShader {
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
class PrecompileGradientShader final : public PrecompileShader {
public:
    PrecompileGradientShader(SkShaderBase::GradientType type) : fType(type) {}

private:
    /*
     * The gradients currently have three specializations based on the number of stops.
     */
    inline static constexpr int kNumStopVariants = 3;
    inline static constexpr int kStopVariants[kNumStopVariants] =
            { 4, 8, GradientShaderBlocks::GradientData::kNumInternalStorageStops+1 };

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

        bool useStorageBuffer = keyContext.caps()->gradientBufferSupport();

        GradientShaderBlocks::GradientData gradData(fType,
                                                    kStopVariants[intrinsicCombination],
                                                    useStorageBuffer);

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
    return PrecompileShaders::LocalMatrix({ std::move(s) });
}

sk_sp<PrecompileShader> PrecompileShaders::RadialGradient() {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kRadial);
    return PrecompileShaders::LocalMatrix({ std::move(s) });
}

sk_sp<PrecompileShader> PrecompileShaders::SweepGradient() {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kSweep);
    return PrecompileShaders::LocalMatrix({ std::move(s) });
}

sk_sp<PrecompileShader> PrecompileShaders::TwoPointConicalGradient() {
    sk_sp<PrecompileShader> s =
            sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kConical);
    return PrecompileShaders::LocalMatrix({ std::move(s) });
}

//--------------------------------------------------------------------------------------------------
// The PictureShader ultimately turns into an SkImageShader optionally wrapped in a
// LocalMatrixShader.
// Note that this means each precompile PictureShader will add 12 combinations:
//    2 (pictureshader LM) x 6 (imageShader variations)
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
// In the main Skia API the SkLocalMatrixShader is optimized away when the LM is the identity
// or omitted. The PrecompileLocalMatrixShader captures this by adding two intrinsic options.
// One with the LMShader wrapping the child and one without the LMShader.
class PrecompileLocalMatrixShader final : public PrecompileShader {
public:
    enum class Flags {
        kNone                  = 0b00,
        kIsPerspective         = 0b01,
        kIncludeWithOutVariant = 0b10,
    };

    PrecompileLocalMatrixShader(SkSpan<const sk_sp<PrecompileShader>> wrapped,
                                SkEnumBitMask<Flags> flags = Flags::kNone)
            : fWrapped(wrapped.begin(), wrapped.end())
            , fFlags(flags) {
        fNumWrappedCombos = 0;
        for (const auto& s : fWrapped) {
            fNumWrappedCombos += s->priv().numCombinations();
        }
    }

    bool isConstant(int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        /*
         * Regardless of whether the LocalMatrixShader elides itself or not, we always want
         * the Constant-ness of the wrapped shader.
         */
        int desiredWrappedCombination = desiredCombination / kNumIntrinsicCombinations;
        SkASSERT(desiredWrappedCombination < fNumWrappedCombos);

        std::pair<sk_sp<PrecompileShader>, int> wrapped =
                PrecompileBase::SelectOption(SkSpan(fWrapped), desiredWrappedCombination);
        if (wrapped.first) {
            return wrapped.first->priv().isConstant(wrapped.second);
        }

        return false;
    }

    SkSpan<const sk_sp<PrecompileShader>> getWrapped() const {
        return fWrapped;
    }

    SkEnumBitMask<Flags> getFlags() const { return fFlags; }

private:
    // The LocalMatrixShader has two potential variants: with and without the LocalMatrixShader
    // In the "with" variant, the kIsPerspective flag will determine if the shader performs
    // the perspective division or not.
    inline static constexpr int kNumIntrinsicCombinations = 2;
    inline static constexpr int kWithLocalMatrix    = 1;
    inline static constexpr int kWithoutLocalMatrix = 0;

    bool isALocalMatrixShader() const override { return true; }

    int numIntrinsicCombinations() const override {
        if (!(fFlags & Flags::kIncludeWithOutVariant)) {
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

        if (!(fFlags & Flags::kIncludeWithOutVariant)) {
            desiredLMCombination = kWithLocalMatrix;
            desiredWrappedCombination = desiredCombination;
        } else {
            desiredLMCombination = desiredCombination % kNumIntrinsicCombinations;
            desiredWrappedCombination = desiredCombination / kNumIntrinsicCombinations;
        }
        SkASSERT(desiredWrappedCombination < fNumWrappedCombos);

        if (desiredLMCombination == kWithLocalMatrix) {
            SkMatrix matrix = SkMatrix::I();
            if (fFlags & Flags::kIsPerspective) {
                matrix.setPerspX(0.1f);
            }
            LocalMatrixShaderBlock::LMShaderData lmShaderData(matrix);

            LocalMatrixShaderBlock::BeginBlock(keyContext, builder, gatherer, matrix);
        }

        AddToKey<PrecompileShader>(keyContext, builder, gatherer, fWrapped,
                                   desiredWrappedCombination);

        if (desiredLMCombination == kWithLocalMatrix) {
            builder->endBlock();
        }
    }

    std::vector<sk_sp<PrecompileShader>> fWrapped;
    int fNumWrappedCombos;
    SkEnumBitMask<Flags> fFlags;
};

sk_sp<PrecompileShader> PrecompileShaders::LocalMatrix(
        SkSpan<const sk_sp<PrecompileShader>> wrapped,
        bool isPerspective) {
    return sk_make_sp<PrecompileLocalMatrixShader>(
            std::move(wrapped),
            isPerspective ? PrecompileLocalMatrixShader::Flags::kIsPerspective
                          : PrecompileLocalMatrixShader::Flags::kNone);
}

sk_sp<PrecompileShader> PrecompileShadersPriv::LocalMatrixBothVariants(
        SkSpan<const sk_sp<PrecompileShader>> wrapped) {
    return sk_make_sp<PrecompileLocalMatrixShader>(
            std::move(wrapped),
            PrecompileLocalMatrixShader::Flags::kIncludeWithOutVariant);
}

sk_sp<PrecompileShader> PrecompileShader::makeWithLocalMatrix(bool isPerspective) const {
    if (this->priv().isALocalMatrixShader()) {
        // SkShader::makeWithLocalMatrix collapses chains of localMatrix shaders so we need to
        // follow suit here, folding in any new perspective flag if needed.
        auto thisAsLMShader = static_cast<const PrecompileLocalMatrixShader*>(this);
        if (isPerspective && !(thisAsLMShader->getFlags() &
                PrecompileLocalMatrixShader::Flags::kIsPerspective)) {
            return sk_make_sp<PrecompileLocalMatrixShader>(
                thisAsLMShader->getWrapped(),
                thisAsLMShader->getFlags() | PrecompileLocalMatrixShader::Flags::kIsPerspective);
        }

        return sk_ref_sp(this);
    }

    return PrecompileShaders::LocalMatrix({ sk_ref_sp(this) }, isPerspective);
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorFilterShader final : public PrecompileShader {
public:
    PrecompileColorFilterShader(SkSpan<const sk_sp<PrecompileShader>> shaders,
                                SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters)
            : fShaders(shaders.begin(), shaders.end())
            , fColorFilters(colorFilters.begin(), colorFilters.end()) {
        fNumShaderCombos = 0;
        for (const auto& s : fShaders) {
            fNumShaderCombos += s->priv().numCombinations();
        }
        fNumColorFilterCombos = 0;
        for (const auto& cf : fColorFilters) {
            fNumColorFilterCombos += cf->priv().numCombinations();
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
class PrecompileWorkingColorSpaceShader final : public PrecompileShader {
public:
    PrecompileWorkingColorSpaceShader(SkSpan<const sk_sp<PrecompileShader>> shaders,
                                      SkSpan<const sk_sp<SkColorSpace>> colorSpaces)
            : fShaders(shaders.begin(), shaders.end())
            , fColorSpaces(colorSpaces.begin(), colorSpaces.end()) {
        fNumShaderCombos = 0;
        for (const auto& s : fShaders) {
            fNumShaderCombos += s->priv().numCombinations();
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
class PrecompileCTMShader final : public PrecompileShader {
public:
    PrecompileCTMShader(SkSpan<const sk_sp<PrecompileShader>> wrapped)
            : fWrapped(wrapped.begin(), wrapped.end()) {
        fNumWrappedCombos = 0;
        for (const auto& s : fWrapped) {
            fNumWrappedCombos += s->priv().numCombinations();
        }
    }

    bool isConstant(int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumWrappedCombos);

        std::pair<sk_sp<PrecompileShader>, int> wrapped =
                PrecompileBase::SelectOption(SkSpan(fWrapped), desiredCombination);
        if (wrapped.first) {
            return wrapped.first->priv().isConstant(wrapped.second);
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
class PrecompileBlurShader final : public PrecompileShader {
public:
    PrecompileBlurShader(sk_sp<PrecompileShader> wrapped)
            : fWrapped(std::move(wrapped)) {
        fNumWrappedCombos = fWrapped->priv().numCombinations();
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
class PrecompileMatrixConvolutionShader final : public PrecompileShader {
public:
    PrecompileMatrixConvolutionShader(sk_sp<PrecompileShader> wrapped)
            : fWrapped(std::move(wrapped)) {
        fNumWrappedCombos = fWrapped->priv().numCombinations();

        // When the matrix convolution ImageFilter uses a texture we know it will only ever
        // be SkFilterMode::kNearest and SkTileMode::kClamp.
        // TODO: add a PrecompileImageShaderFlags to further limit the raw image shader
        // combinations. Right now we're getting two combinations for the raw shader
        // (sk_image_shader and sk_hw_image_shader).
        fRawImageShader =
                PrecompileShadersPriv::RawImage(PrecompileImageShaderFlags::kExcludeCubic);
        fNumRawImageShaderCombos = fRawImageShader->priv().numCombinations();
    }

private:
    int numIntrinsicCombinations() const override {
        // The uniform version only has one option but the two texture-based versions will
        // have as many combinations as the raw image shader.
        return 1 + 2 * fNumRawImageShaderCombos;
    }

    int numChildCombinations() const override { return fNumWrappedCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        int desiredTextureCombination = 0;

        const int desiredWrappedCombination = desiredCombination % fNumWrappedCombos;
        int remainingCombinations = desiredCombination / fNumWrappedCombos;

        SkKnownRuntimeEffects::StableKey stableKey = SkKnownRuntimeEffects::StableKey::kInvalid;
        if (remainingCombinations == 0) {
            stableKey = SkKnownRuntimeEffects::StableKey::kMatrixConvUniforms;
        } else {
            static constexpr SkKnownRuntimeEffects::StableKey kTextureBasedStableKeys[] = {
                    SkKnownRuntimeEffects::StableKey::kMatrixConvTexSm,
                    SkKnownRuntimeEffects::StableKey::kMatrixConvTexLg,
            };

            --remainingCombinations;
            stableKey = kTextureBasedStableKeys[remainingCombinations % 2];
            desiredTextureCombination = remainingCombinations / 2;
            SkASSERT(desiredTextureCombination < fNumRawImageShaderCombos);
        }

        const SkRuntimeEffect* fEffect = GetKnownRuntimeEffect(stableKey);

        KeyContextWithScope childContext(keyContext, KeyContext::Scope::kRuntimeEffect);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, { sk_ref_sp(fEffect) });
            fWrapped->priv().addToKey(childContext, builder, gatherer, desiredWrappedCombination);
            if (stableKey != SkKnownRuntimeEffects::StableKey::kMatrixConvUniforms) {
                fRawImageShader->priv().addToKey(childContext, builder, gatherer,
                                                 desiredTextureCombination);
            }
        builder->endBlock();
    }

    sk_sp<PrecompileShader> fWrapped;
    int fNumWrappedCombos;
    sk_sp<PrecompileShader> fRawImageShader;
    int fNumRawImageShaderCombos;
};

sk_sp<PrecompileShader> PrecompileShadersPriv::MatrixConvolution(
        sk_sp<skgpu::graphite::PrecompileShader> wrapped) {
    return sk_make_sp<PrecompileMatrixConvolutionShader>(std::move(wrapped));
}

//--------------------------------------------------------------------------------------------------
class PrecompileMorphologyShader final : public PrecompileShader {
public:
    PrecompileMorphologyShader(sk_sp<PrecompileShader> wrapped,
                               SkKnownRuntimeEffects::StableKey stableKey)
            : fWrapped(std::move(wrapped))
            , fStableKey(stableKey) {
        fNumWrappedCombos = fWrapped->priv().numCombinations();
        SkASSERT(stableKey == SkKnownRuntimeEffects::StableKey::kLinearMorphology ||
                 stableKey == SkKnownRuntimeEffects::StableKey::kSparseMorphology);
    }

private:
    int numChildCombinations() const override { return fNumWrappedCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumWrappedCombos);

        const SkRuntimeEffect* effect = GetKnownRuntimeEffect(fStableKey);

        KeyContextWithScope childContext(keyContext, KeyContext::Scope::kRuntimeEffect);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, { sk_ref_sp(effect) });
            fWrapped->priv().addToKey(childContext, builder, gatherer, desiredCombination);
        builder->endBlock();
    }

    sk_sp<PrecompileShader> fWrapped;
    int fNumWrappedCombos;
    SkKnownRuntimeEffects::StableKey fStableKey;
};

sk_sp<PrecompileShader> PrecompileShadersPriv::LinearMorphology(sk_sp<PrecompileShader> wrapped) {
    return sk_make_sp<PrecompileMorphologyShader>(
            std::move(wrapped),
            SkKnownRuntimeEffects::StableKey::kLinearMorphology);
}

sk_sp<PrecompileShader> PrecompileShadersPriv::SparseMorphology(sk_sp<PrecompileShader> wrapped) {
    return sk_make_sp<PrecompileMorphologyShader>(
            std::move(wrapped),
            SkKnownRuntimeEffects::StableKey::kSparseMorphology);
}

//--------------------------------------------------------------------------------------------------
class PrecompileDisplacementShader final : public PrecompileShader {
public:
    PrecompileDisplacementShader(sk_sp<PrecompileShader> displacement,
                                 sk_sp<PrecompileShader> color)
            : fDisplacement(std::move(displacement))
            , fColor(std::move(color)) {
        fNumDisplacementCombos = fDisplacement->priv().numCombinations();
        fNumColorCombos = fColor->priv().numCombinations();
    }

private:
    int numChildCombinations() const override { return fNumDisplacementCombos * fNumColorCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numChildCombinations());

        const int desiredDisplacementCombination = desiredCombination % fNumDisplacementCombos;
        const int desiredColorCombination = desiredCombination / fNumDisplacementCombos;
        SkASSERT(desiredColorCombination < fNumColorCombos);

        const SkRuntimeEffect* fEffect =
                GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kDisplacement);

        KeyContextWithScope childContext(keyContext, KeyContext::Scope::kRuntimeEffect);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, { sk_ref_sp(fEffect) });
            fDisplacement->priv().addToKey(childContext, builder, gatherer,
                                           desiredDisplacementCombination);
            fColor->priv().addToKey(childContext, builder, gatherer,
                                    desiredColorCombination);
        builder->endBlock();
    }

    sk_sp<PrecompileShader> fDisplacement;
    int fNumDisplacementCombos;
    sk_sp<PrecompileShader> fColor;
    int fNumColorCombos;
};

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileShader> PrecompileShadersPriv::Displacement(sk_sp<PrecompileShader> displacement,
                                                            sk_sp<PrecompileShader> color) {
    return sk_make_sp<PrecompileDisplacementShader>(std::move(displacement), std::move(color));
}

//--------------------------------------------------------------------------------------------------
class PrecompileLightingShader final : public PrecompileShader {
public:
    PrecompileLightingShader(sk_sp<PrecompileShader> wrapped)
            : fWrapped(std::move(wrapped)) {
        fNumWrappedCombos = fWrapped->priv().numCombinations();
    }

private:
    int numChildCombinations() const override { return fNumWrappedCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumWrappedCombos);

        const SkRuntimeEffect* normalEffect =
                GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kNormal);
        const SkRuntimeEffect* lightingEffect =
                GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kLighting);

        KeyContextWithScope childContext(keyContext, KeyContext::Scope::kRuntimeEffect);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                       { sk_ref_sp(lightingEffect) });
            RuntimeEffectBlock::BeginBlock(childContext, builder, gatherer,
                                           { sk_ref_sp(normalEffect) });
                fWrapped->priv().addToKey(childContext, builder, gatherer, desiredCombination);
            builder->endBlock();
        builder->endBlock();
    }

    sk_sp<PrecompileShader> fWrapped;
    int fNumWrappedCombos;
};

sk_sp<PrecompileShader> PrecompileShadersPriv::Lighting(sk_sp<PrecompileShader> wrapped) {
    return sk_make_sp<PrecompileLightingShader>(std::move(wrapped));
}

//--------------------------------------------------------------------------------------------------

} // namespace skgpu::graphite
