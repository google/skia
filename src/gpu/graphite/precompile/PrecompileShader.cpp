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
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileImageShader.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShadersPriv.h"
#include "src/shaders/gradients/SkLinearGradient.h"

#if defined(SK_DEBUG)
#include "src/base/SkMathPriv.h"
#endif

namespace skgpu::graphite {

SK_MAKE_BITMASK_OPS(PrecompileShaders::ImageShaderFlags)
SK_MAKE_BITMASK_OPS(PrecompileShaders::YUVImageShaderFlags)

using PrecompileShaders::GradientShaderFlags;
using PrecompileShaders::ImageShaderFlags;
using PrecompileShaders::YUVImageShaderFlags;

PrecompileShader::~PrecompileShader() = default;

sk_sp<PrecompileShader> PrecompileShader::makeWithColorFilter(
        sk_sp<PrecompileColorFilter> cf) const {
    if (!cf) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::ColorFilter({{ sk_ref_sp(this) }}, {{ std::move(cf) }});
}

sk_sp<PrecompileShader> PrecompileShader::makeWithWorkingColorSpace(
        sk_sp<SkColorSpace> inputCS, sk_sp<SkColorSpace> outputCS) const {
    if (!inputCS && !outputCS) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::WorkingColorSpaceExplicit(
            {{ sk_ref_sp(this) }},
            {{ { std::move(inputCS), std::move(outputCS) } }});
}

//--------------------------------------------------------------------------------------------------
class PrecompileEmptyShader final : public PrecompileShader {
private:
    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination == 0); // The empty shader only ever has one combination
        keyContext.paintParamsKeyBuilder()->addBlock(BuiltInCodeSnippetID::kPriorOutput);
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination == 0); // The color shader only ever has one combination
        // The white PMColor is just a placeholder for the actual paint params color
        SolidColorShaderBlock::AddBlock(keyContext, SK_PMColor4fWHITE);
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
    PrecompileBlendShader(PrecompileBlenderList&& blenders,
                          SkSpan<const sk_sp<PrecompileShader>> dsts,
                          SkSpan<const sk_sp<PrecompileShader>> srcs)
            : fBlenderOptions(std::move(blenders))
            , fDstOptions(dsts.begin(), dsts.end())
            , fSrcOptions(srcs.begin(), srcs.end()) {
        fNumDstCombos = 0;
        for (const auto& d : fDstOptions) {
            fNumDstCombos += d->priv().numCombinations();
        }

        fNumSrcCombos = 0;
        for (const auto& s : fSrcOptions) {
            fNumSrcCombos += s->priv().numCombinations();
        }
    }

private:
    int numChildCombinations() const override {
        return fBlenderOptions.numCombinations() * fNumDstCombos * fNumSrcCombos;
    }

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        const int desiredDstCombination = desiredCombination % fNumDstCombos;
        int remainingCombinations = desiredCombination / fNumDstCombos;

        const int desiredSrcCombination = remainingCombinations % fNumSrcCombos;
        remainingCombinations /= fNumSrcCombos;

        int desiredBlendCombination = remainingCombinations;
        SkASSERT(desiredBlendCombination < fBlenderOptions.numCombinations());

        auto [blender, blenderCombination] = fBlenderOptions.selectOption(desiredBlendCombination);
        if (blender->priv().asBlendMode()) {
            // Coefficient and HSLC blends, and other fixed SkBlendMode blenders use the
            // BlendCompose block to organize the children.
            BlendComposeBlock::BeginBlock(keyContext);
        } else {
            // Runtime blenders are wrapped in the kBlend runtime shader, although functionally
            // it is identical to the BlendCompose snippet.
            const SkRuntimeEffect* blendEffect =
                    GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kBlend);

            RuntimeEffectBlock::BeginBlock(keyContext, { sk_ref_sp(blendEffect) });
        }

        AddToKey<PrecompileShader>(keyContext, fSrcOptions, desiredSrcCombination);
        AddToKey<PrecompileShader>(keyContext, fDstOptions, desiredDstCombination);

        if (blender->priv().asBlendMode()) {
            SkASSERT(blenderCombination == 0);
            AddBlendMode(keyContext, *blender->priv().asBlendMode());
        } else {
            blender->priv().addToKey(keyContext, blenderCombination);
        }

        keyContext.paintParamsKeyBuilder()->endBlock();  // BlendComposeBlock or RuntimeEffectBlock
    }

    PrecompileBlenderList fBlenderOptions;
    std::vector<sk_sp<PrecompileShader>> fDstOptions;
    std::vector<sk_sp<PrecompileShader>> fSrcOptions;

    int fNumDstCombos;
    int fNumSrcCombos;
};

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<const sk_sp<PrecompileBlender>> blenders,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {
    return sk_make_sp<PrecompileBlendShader>(PrecompileBlenderList(blenders), dsts, srcs);
}

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<const SkBlendMode> blendModes,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {
    return sk_make_sp<PrecompileBlendShader>(PrecompileBlenderList(blendModes), dsts, srcs);
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumShaderCombos);

        constexpr SkRect kIgnored { 0, 0, 256, 256 }; // ignored bc we're precompiling

        // TODO: update CoordClampShaderBlock so this is optional
        CoordClampShaderBlock::CoordClampData data(kIgnored);

        CoordClampShaderBlock::BeginBlock(keyContext, data);
            AddToKey<PrecompileShader>(keyContext, fShaders, desiredCombination);
        keyContext.paintParamsKeyBuilder()->endBlock();
    }

    std::vector<sk_sp<PrecompileShader>> fShaders;
    int fNumShaderCombos;
};

sk_sp<PrecompileShader> PrecompileShaders::CoordClamp(SkSpan<const sk_sp<PrecompileShader>> input) {
    return sk_make_sp<PrecompileCoordClampShader>(input);
}

//--------------------------------------------------------------------------------------------------
PrecompileImageShader::PrecompileImageShader(SkEnumBitMask<ImageShaderFlags> flags,
                                             SkSpan<const SkColorInfo> colorInfos,
                                             SkSpan<const SkTileMode> tileModes,
                                             bool raw)
    : fNumExtraSamplingTilingCombos((flags & ImageShaderFlags::kCubicSampling)
                                            ? kExtraNumSamplingTilingCombos
                                            : 1)  // Just kHWTiled
    , fColorInfos(!colorInfos.empty()
                    ? std::vector<SkColorInfo>(colorInfos.begin(), colorInfos.end())
                    : raw ? RawImageDefaultColorInfos()
                          : (flags & ImageShaderFlags::kIncludeAlphaOnly)
                                     ? DefaultColorInfos()
                                     : NonAlphaOnlyDefaultColorInfos())
    , fTileModes(std::vector<SkTileMode>(tileModes.begin(), tileModes.end()))
    , fUseDstColorInfo(!colorInfos.empty())
    , fRaw(raw) {}

void PrecompileImageShader::setImmutableSamplerInfo(const ImmutableSamplerInfo& samplerInfo) {
    fImmutableSamplerInfo = samplerInfo;
}

int PrecompileImageShader::numIntrinsicCombinations() const {
    // TODO(b/400682634) If color infos were provided by the client, and we're using the
    // destination color space to determine what color space transform shaders to use, we can
    // end up generating duplicate shaders, and the actual number of unique shaders generated
    // will be less than the number calculated here.
    return fColorInfos.size() * (fTileModes.size() + fNumExtraSamplingTilingCombos);
}

void PrecompileImageShader::addToKey(const KeyContext& keyContext, int desiredCombination) const {
    SkASSERT(this->numChildCombinations() == 1);
    SkASSERT(desiredCombination < this->numIntrinsicCombinations());

    const int numSamplingTilingCombos = fTileModes.size() + fNumExtraSamplingTilingCombos;
    const int desiredSamplingTilingCombo = desiredCombination % numSamplingTilingCombos;
    desiredCombination /= numSamplingTilingCombos;

    const int desiredColorInfo = desiredCombination;
    SkASSERT(desiredColorInfo < static_cast<int>(fColorInfos.size()));

    static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
    // This is kLinear to work around b/417429187
    static constexpr SkSamplingOptions kDefaultSampling(SkFilterMode::kLinear);

    // ImageShaderBlock will use hardware tiling when the subset covers the entire image, so we
    // create subset + image size combinations where subset == imgSize (for a shader that uses
    // hardware tiling) and subset < imgSize (for a shader that does shader-based tiling).
    static constexpr SkRect kSubset = SkRect::MakeWH(1.0f, 1.0f);
    static constexpr SkISize kHWTileableSize = SkISize::Make(1, 1);
    static constexpr SkISize kShaderTileableSize = SkISize::Make(2, 2);

    const int numTileModes = fTileModes.size();
    const SkTileMode tileMode = (desiredSamplingTilingCombo < numTileModes)
                                        ? fTileModes[desiredSamplingTilingCombo]
                                        : SkTileMode::kClamp;
    const SkISize imgSize = (desiredSamplingTilingCombo >= numTileModes &&
                             desiredSamplingTilingCombo - numTileModes == kHWTiled)
                                    ? kHWTileableSize
                                    : kShaderTileableSize;
    const SkSamplingOptions sampling =
            (desiredSamplingTilingCombo >= numTileModes &&
             desiredSamplingTilingCombo - numTileModes == kCubicSampled)
                    ? kDefaultCubicSampling
                    : kDefaultSampling;

    const ImageShaderBlock::ImageData imgData(sampling, tileMode, tileMode, imgSize, kSubset,
                                              fImmutableSamplerInfo);

    const SkColorInfo& colorInfo = fColorInfos[desiredColorInfo];
    const bool alphaOnly = SkColorTypeIsAlphaOnly(colorInfo.colorType());

    const Caps* caps = keyContext.caps();
    Swizzle readSwizzle = caps->getReadSwizzle(
            colorInfo.colorType(),
            caps->getDefaultSampledTextureInfo(
                    colorInfo.colorType(), Mipmapped::kNo, Protected::kNo, Renderable::kNo));
    if (alphaOnly) {
        readSwizzle = Swizzle::Concat(readSwizzle, Swizzle("000a"));
    }

    ColorSpaceTransformBlock::ColorSpaceTransformData colorXformData(
            SwizzleClassToReadEnum(readSwizzle));

    if (!fRaw) {
        const SkColorSpace* dstColorSpace = sk_srgb_singleton();
        SkAlphaType dstAT = colorInfo.alphaType();
        if (fUseDstColorInfo) {
            dstColorSpace = keyContext.dstColorInfo().colorSpace();
            dstAT = keyContext.dstColorInfo().alphaType();
        }
        colorXformData.fSteps = SkColorSpaceXformSteps(
                colorInfo.colorSpace(), colorInfo.alphaType(),
                dstColorSpace, dstAT);

        if (alphaOnly) {
            Blend(keyContext,
                  /* addBlendToKey= */ [&] () -> void {
                      AddFixedBlendMode(keyContext, SkBlendMode::kDstIn);
                  },
                  /* addSrcToKey= */ [&] () -> void {
                      Compose(keyContext,
                              /* addInnerToKey= */ [&]() -> void {
                                  ImageShaderBlock::AddBlock(keyContext, imgData);
                              },
                              /* addOuterToKey= */ [&]() -> void {
                                  ColorSpaceTransformBlock::AddBlock(keyContext, colorXformData);
                              });
                  },
                  /* addDstToKey= */ [&]() -> void {
                      RGBPaintColorBlock::AddBlock(keyContext);
                  });
            return;
        }
    }

    Compose(keyContext,
            /* addInnerToKey= */ [&]() -> void {
                ImageShaderBlock::AddBlock(keyContext, imgData);
            },
            /* addOuterToKey= */ [&]() -> void {
                ColorSpaceTransformBlock::AddBlock(keyContext, colorXformData);
            });
}

sk_sp<PrecompileShader> PrecompileShaders::Image(ImageShaderFlags shaderFlags,
                                                 SkSpan<const SkColorInfo> colorInfos,
                                                 SkSpan<const SkTileMode> tileModes) {
    return PrecompileShaders::LocalMatrix(
            {{ sk_make_sp<PrecompileImageShader>(shaderFlags,
                                                colorInfos, tileModes,
                                                /* raw= */false) }});
}

sk_sp<PrecompileShader> PrecompileShaders::Image(SkSpan<const SkColorInfo> colorInfos,
                                                 SkSpan<const SkTileMode> tileModes) {
    return Image(ImageShaderFlags::kAll, colorInfos, tileModes);
}

sk_sp<PrecompileShader> PrecompileShaders::RawImage(ImageShaderFlags shaderFlags,
                                                    SkSpan<const SkColorInfo> colorInfos,
                                                    SkSpan<const SkTileMode> tileModes) {
    SkEnumBitMask<ImageShaderFlags> newFlags = ~ImageShaderFlags::kCubicSampling & shaderFlags;
    return PrecompileShaders::LocalMatrix(
            {{ sk_make_sp<PrecompileImageShader>(newFlags,
                                                colorInfos, tileModes,
                                                /* raw= */true) }});
}

//--------------------------------------------------------------------------------------------------
class PrecompileYUVImageShader : public PrecompileShader {
public:
    PrecompileYUVImageShader(SkEnumBitMask<YUVImageShaderFlags> shaderFlags,
                             SkSpan<const SkColorInfo> colorInfos)
            : fColorInfos(!colorInfos.empty()
                            ? std::vector<SkColorInfo>(colorInfos.begin(), colorInfos.end())
                            : PrecompileImageShader::NonAlphaOnlyDefaultColorInfos())
            , fUseDstColorSpace(!colorInfos.empty()) {
        this->setupTilingModes(shaderFlags);
    }

private:
    // There are 4 possible tiling modes:
    //    non-cubic shader tiling
    //    HW tiling w/o swizzle
    //    HW tiling w/ swizzle
    //    cubic shader tiling       -- can be omitted
    inline static constexpr int kMaxTilingModes     = 4;

    inline static constexpr int kShaderTiled        = 0;
    inline static constexpr int kHWTiledNoSwizzle   = 1;
    inline static constexpr int kHWTiledWithSwizzle = 2;
    inline static constexpr int kCubicShaderTiled   = 3;

    void setupTilingModes(SkEnumBitMask<YUVImageShaderFlags> flags) {
        fNumTilingModes = 0;

        if (flags & YUVImageShaderFlags::kHardwareSamplingNoSwizzle) {
            fTilingModes[fNumTilingModes++] = kHWTiledNoSwizzle;
        }
        if (flags & YUVImageShaderFlags::kHardwareSampling) {
            fTilingModes[fNumTilingModes++] = kHWTiledWithSwizzle;
        }
        if (flags & YUVImageShaderFlags::kShaderBasedSampling) {
            fTilingModes[fNumTilingModes++] = kShaderTiled;
        }
        if (flags & YUVImageShaderFlags::kCubicSampling) {
            fTilingModes[fNumTilingModes++] = kCubicShaderTiled;
        }

        SkASSERT(fNumTilingModes == SkPopCount(flags.value()));
        SkASSERT(fNumTilingModes <= kMaxTilingModes);
    }

    int numIntrinsicCombinations() const override {
        return fNumTilingModes * fColorInfos.size();
    }

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numIntrinsicCombinations());

        int desiredTiling = desiredCombination % fNumTilingModes;
        desiredCombination /= fNumTilingModes;

        int desiredColorInfo = desiredCombination;
        SkASSERT(desiredColorInfo < static_cast<int>(fColorInfos.size()));

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;

        YUVImageShaderBlock::ImageData imgData(
                fTilingModes[desiredTiling] == kCubicShaderTiled
                                     ? kDefaultCubicSampling
                                     : kDefaultSampling,
                SkTileMode::kClamp,
                fTilingModes[desiredTiling] == kShaderTiled ? SkTileMode::kRepeat
                                                            : SkTileMode::kClamp,
                /* imgSize= */ { 1, 1 },
                /* subset= */ fTilingModes[desiredTiling] == kShaderTiled
                                     ? SkRect::MakeEmpty()
                                     : SkRect::MakeWH(1, 1));

        static constexpr SkV4 kRedChannel{ 1.f, 0.f, 0.f, 0.f };
        imgData.fChannelSelect[0] = kRedChannel;
        imgData.fChannelSelect[1] = kRedChannel;
        if (fTilingModes[desiredTiling] == kHWTiledNoSwizzle) {
            imgData.fChannelSelect[2] = kRedChannel;
        } else {
            // Having a non-red channel selector forces a swizzle
            imgData.fChannelSelect[2] = { 0.f, 1.f, 0.f, 0.f};
        }
        imgData.fChannelSelect[3] = kRedChannel;

        imgData.fYUVtoRGBMatrix.setAll(1, 0, 0, 0, 1, 0, 0, 0, 0);
        imgData.fYUVtoRGBTranslate = { 0, 0, 0 };

        const SkColorInfo& colorInfo = fColorInfos[desiredColorInfo];

        const Caps* caps = keyContext.caps();
        Swizzle readSwizzle = caps->getReadSwizzle(
                colorInfo.colorType(),
                caps->getDefaultSampledTextureInfo(
                        colorInfo.colorType(), Mipmapped::kNo, Protected::kNo, Renderable::kNo));
        ColorSpaceTransformBlock::ColorSpaceTransformData colorXformData(
                SwizzleClassToReadEnum(readSwizzle));

        const SkColorSpace* dstColorSpace = fUseDstColorSpace
                                            ? keyContext.dstColorInfo().colorSpace()
                                            : sk_srgb_singleton();
        colorXformData.fSteps = SkColorSpaceXformSteps(
                colorInfo.colorSpace(), colorInfo.alphaType(),
                dstColorSpace, colorInfo.alphaType());

        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    YUVImageShaderBlock::AddBlock(keyContext, imgData);
                },
                /* addOuterToKey= */ [&]() -> void {
                    ColorSpaceTransformBlock::AddBlock(keyContext, colorXformData);
                });
    }

    const std::vector<SkColorInfo> fColorInfos;

    // If true, use the destination color space from the KeyContext provided to addToKey.
    // This is true if and only if the client has provided a list of color infos. Otherwise, we
    // always use an sRGB destination per the default SkColorInfo lists defined in
    // PrecompileImageShader::DefaultColorInfos.
    const bool fUseDstColorSpace;
    int fNumTilingModes;
    int fTilingModes[kMaxTilingModes];
};

sk_sp<PrecompileShader> PrecompileShaders::YUVImage(YUVImageShaderFlags shaderFlags,
                                                    SkSpan<const SkColorInfo> colorInfos) {
    return PrecompileShaders::LocalMatrix(
            {{ sk_make_sp<PrecompileYUVImageShader>(shaderFlags, colorInfos) }});
}

//--------------------------------------------------------------------------------------------------
class PrecompilePerlinNoiseShader final : public PrecompileShader {
public:
    PrecompilePerlinNoiseShader() {}

private:
    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {

        SkASSERT(desiredCombination == 0); // The Perlin noise shader only ever has one combination

        // TODO: update PerlinNoiseShaderBlock so the NoiseData is optional
        static const PerlinNoiseShaderBlock::PerlinNoiseData kIgnoredNoiseData(
                PerlinNoiseShaderBlock::Type::kFractalNoise, { 0.0f, 0.0f }, 2, {1, 1});

        PerlinNoiseShaderBlock::AddBlock(keyContext, kIgnoredNoiseData);
    }

};

sk_sp<PrecompileShader> PrecompileShaders::MakeFractalNoise() {
    return sk_make_sp<PrecompilePerlinNoiseShader>();
}

sk_sp<PrecompileShader> PrecompileShaders::MakeTurbulence() {
    return sk_make_sp<PrecompilePerlinNoiseShader>();
}

namespace {

sk_sp<SkColorSpace> get_gradient_intermediate_cs(SkColorSpace* dstColorSpace,
                                                 SkGradientShader::Interpolation interpolation) {
    // Any gradient shader will do, as long as it has the correct interpolation settings.
    constexpr SkPoint pts[2] = {{0.f, 0.f}, {1.f, 0.f}};
    constexpr SkColor4f colors[2] = {SkColors::kBlack, SkColors::kWhite};
    constexpr float pos[2] = {0.f, 1.f};
    SkLinearGradient shader(pts, {colors, nullptr, pos, 2, SkTileMode::kClamp, interpolation});

    SkColor4fXformer xformedColors(&shader, dstColorSpace);
    return xformedColors.fIntermediateColorSpace;
}

}  // anonymous namespace

//--------------------------------------------------------------------------------------------------
class PrecompileGradientShader final : public PrecompileShader {
public:
    PrecompileGradientShader(SkShaderBase::GradientType type,
                             SkEnumBitMask<GradientShaderFlags> flags,
                             const SkGradientShader::Interpolation& interpolation)
            : fType(type)
            , fInterpolation(interpolation) {
        this->setupStopVariants(flags);
    }

private:
    /*
     * The gradients can have up to three specializations based on the number of stops.
     */
    inline static constexpr int kMaxStopVariants = 3;

    void setupStopVariants(SkEnumBitMask<GradientShaderFlags> flags) {
        fNumStopVariants = 0;

        if (flags & GradientShaderFlags::kSmall) {
            fStopVariants[fNumStopVariants++] = 4;
        }
        if (flags & GradientShaderFlags::kMedium) {
            fStopVariants[fNumStopVariants++] = 8;
        }
        if (flags & GradientShaderFlags::kLarge) {
            fStopVariants[fNumStopVariants++] =
                    GradientShaderBlocks::GradientData::kNumInternalStorageStops+1;
        }

        SkASSERT(fNumStopVariants == SkPopCount(flags.value()));
        SkASSERT(fNumStopVariants <= kMaxStopVariants);
    }

    int numIntrinsicCombinations() const override { return fNumStopVariants; }

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(this->numChildCombinations() == 1);
        SkASSERT(desiredCombination < fNumStopVariants);

        bool useStorageBuffer = keyContext.caps()->gradientBufferSupport();

        GradientShaderBlocks::GradientData gradData(fType,
                                                    fStopVariants[desiredCombination],
                                                    useStorageBuffer);

        // The logic for setting up color spaces here should match that in the "add_gradient_to_key"
        // functions from src/gpu/graphite/KeyHelpers.cpp.
        sk_sp<SkColorSpace> intermediateCS = get_gradient_intermediate_cs(
                keyContext.dstColorInfo().colorSpace(), fInterpolation);
        const SkColorSpace* dstCS = keyContext.dstColorInfo().colorSpace()
                                            ? keyContext.dstColorInfo().colorSpace()
                                            : sk_srgb_singleton();

        ColorSpaceTransformBlock::ColorSpaceTransformData csData(
                intermediateCS.get(), kPremul_SkAlphaType,
                dstCS, kPremul_SkAlphaType);

        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    GradientShaderBlocks::AddBlock(keyContext, gradData);
                },
                /* addOuterToKey= */  [&]() -> void {
                    ColorSpaceTransformBlock::AddBlock(keyContext, csData);
                });
    }

    const SkShaderBase::GradientType fType;
    const SkGradientShader::Interpolation fInterpolation;

    int fNumStopVariants = 0;
    int fStopVariants[kMaxStopVariants];
};

sk_sp<PrecompileShader> PrecompileShaders::LinearGradient(
        GradientShaderFlags flags,
        SkGradientShader::Interpolation interpolation) {
    sk_sp<PrecompileShader> s = sk_make_sp<PrecompileGradientShader>(
            SkShaderBase::GradientType::kLinear, flags, interpolation);
    return PrecompileShaders::LocalMatrix({{ std::move(s) }});
}

sk_sp<PrecompileShader> PrecompileShaders::RadialGradient(
        GradientShaderFlags flags,
        SkGradientShader::Interpolation interpolation) {
    sk_sp<PrecompileShader> s = sk_make_sp<PrecompileGradientShader>(
            SkShaderBase::GradientType::kRadial, flags, interpolation);
    return PrecompileShaders::LocalMatrix({{ std::move(s) }});
}

sk_sp<PrecompileShader> PrecompileShaders::SweepGradient(
        GradientShaderFlags flags,
        SkGradientShader::Interpolation interpolation) {
    sk_sp<PrecompileShader> s = sk_make_sp<PrecompileGradientShader>(
            SkShaderBase::GradientType::kSweep, flags, interpolation);
    return PrecompileShaders::LocalMatrix({{ std::move(s) }});
}

sk_sp<PrecompileShader> PrecompileShaders::TwoPointConicalGradient(
        GradientShaderFlags flags,
        SkGradientShader::Interpolation interpolation) {
    sk_sp<PrecompileShader> s = sk_make_sp<PrecompileGradientShader>(
            SkShaderBase::GradientType::kConical, flags, interpolation);
    return PrecompileShaders::LocalMatrix({{ std::move(s) }});
}

//--------------------------------------------------------------------------------------------------
// The PictureShader ultimately turns into an SkImageShader optionally wrapped in a
// LocalMatrixShader.
// Note that this means each precompile PictureShader will add 24 combinations:
//    2 (pictureshader LM) x 12 (imageShader variations)
sk_sp<PrecompileShader> PrecompileShaders::Picture() {
    // Note: We don't need to consider the PrecompileYUVImageShader since the image
    // being drawn was created internally by Skia (as non-YUV).
    return PrecompileShadersPriv::LocalMatrixBothVariants({{ PrecompileShaders::Image() }});
}

sk_sp<PrecompileShader> PrecompileShadersPriv::Picture(bool withLM) {
    sk_sp<PrecompileShader> s = PrecompileShaders::Image();
    if (withLM) {
        return PrecompileShaders::LocalMatrix({{ std::move(s) }});
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
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

            LocalMatrixShaderBlock::BeginBlock(keyContext, matrix);
        }

        AddToKey<PrecompileShader>(keyContext, fWrapped, desiredWrappedCombination);

        if (desiredLMCombination == kWithLocalMatrix) {
            keyContext.paintParamsKeyBuilder()->endBlock();
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

    return PrecompileShaders::LocalMatrix({{ sk_ref_sp(this) }}, isPerspective);
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        int desiredShaderCombination = desiredCombination % fNumShaderCombos;
        int desiredColorFilterCombination = desiredCombination / fNumShaderCombos;
        SkASSERT(desiredColorFilterCombination < fNumColorFilterCombos);

        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    AddToKey<PrecompileShader>(keyContext, fShaders, desiredShaderCombination);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddToKey<PrecompileColorFilter>(keyContext, fColorFilters,
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
                                      SkSpan<const std::pair<sk_sp<SkColorSpace>,
                                                             sk_sp<SkColorSpace>>> colorSpaces)
            : fShaders(shaders.begin(), shaders.end())
            , fColorSpaces(colorSpaces.begin(), colorSpaces.end()) {
        if (colorSpaces.empty()) {
            fColorSpaces.push_back({nullptr, nullptr}); // encode identity
        }
        this->updateNumShaderCombos();
    }

    PrecompileWorkingColorSpaceShader(SkSpan<const sk_sp<PrecompileShader>> shaders,
                                      SkSpan<const sk_sp<SkColorSpace>> inputSpaces,
                                      SkSpan<const sk_sp<SkColorSpace>> outputSpaces)
            : fShaders(shaders.begin(), shaders.end()) {
        static const sk_sp<SkColorSpace> kNullCS;
        SkSpan<const sk_sp<SkColorSpace>> nullSpan{&kNullCS, 1};
        if (inputSpaces.empty())  { inputSpaces  = nullSpan; }
        if (outputSpaces.empty()) { outputSpaces = nullSpan; }

        fColorSpaces.reserve(inputSpaces.size() * outputSpaces.size());
        for (const sk_sp<SkColorSpace>& iCS : inputSpaces) {
            for (const sk_sp<SkColorSpace>& oCS : outputSpaces) {
                fColorSpaces.push_back({iCS, oCS});
            }
        }

        this->updateNumShaderCombos();
    }

private:
    int numChildCombinations() const override { return fNumShaderCombos * fColorSpaces.size(); }

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        int desiredShaderCombination = desiredCombination % fNumShaderCombos;
        int desiredColorSpaceCombination = desiredCombination / fNumShaderCombos;
        SkASSERT(desiredColorSpaceCombination < (int) fColorSpaces.size());

        // Check for an identity working colorspace (that is detected up front with
        // makeWithColorSpace, but due to return type mismatches, can't be handled as easily with
        // the WorkingColorSpace() factory).
        if (!fColorSpaces[desiredColorSpaceCombination].first &&
            !fColorSpaces[desiredColorSpaceCombination].second) {
            // So just add the desired shader direction
            AddToKey<PrecompileShader>(keyContext, fShaders, desiredShaderCombination);
            return;
        }

        const SkColorInfo& dstInfo = keyContext.dstColorInfo();
        const SkAlphaType dstAT = dstInfo.alphaType();
        sk_sp<SkColorSpace> dstCS = dstInfo.refColorSpace();
        if (!dstCS) {
            dstCS = SkColorSpace::MakeSRGB();
        }

        sk_sp<SkColorSpace> inputCS = fColorSpaces[desiredColorSpaceCombination].first;
        if (!inputCS) {
            inputCS = dstCS;
        }
        sk_sp<SkColorSpace> outputCS = fColorSpaces[desiredColorSpaceCombination].second;
        if (!outputCS) {
            outputCS = inputCS;
        }

        // SkWorkingColorSpaceShader's workInUnpremul is not exposed yet in the public API so
        // precompile can assume that it'll always use dstAT.
        const SkAlphaType workingAT = dstAT;
        KeyContext workingContext =
                keyContext.withColorInfo({dstInfo.colorType(), workingAT, inputCS});

        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    AddToKey<PrecompileShader>(workingContext, fShaders, desiredShaderCombination);
                },
                /* addOuterToKey= */ [&]() -> void {
                    ColorSpaceTransformBlock::ColorSpaceTransformData data(
                            outputCS.get(), workingAT, dstCS.get(), dstAT);
                    ColorSpaceTransformBlock::AddBlock(keyContext, data);
                });
    }

    void updateNumShaderCombos() {
        fNumShaderCombos = 0;
        for (const auto& s : fShaders) {
            fNumShaderCombos += s->priv().numCombinations();
        }
    }

    std::vector<sk_sp<PrecompileShader>> fShaders;
    std::vector<std::pair</*input =*/sk_sp<SkColorSpace>,
                          /*output=*/sk_sp<SkColorSpace>>> fColorSpaces;
    int fNumShaderCombos;
};

sk_sp<PrecompileShader> PrecompileShaders::WorkingColorSpace(
        SkSpan<const sk_sp<PrecompileShader>> shaders,
        SkSpan<const sk_sp<SkColorSpace>> inputSpaces,
        SkSpan<const sk_sp<SkColorSpace>> outputSpaces) {
    return sk_make_sp<PrecompileWorkingColorSpaceShader>(std::move(shaders),
                                                         std::move(inputSpaces),
                                                         std::move(outputSpaces));
}

sk_sp<PrecompileShader> PrecompileShaders::WorkingColorSpaceExplicit(
        SkSpan<const sk_sp<PrecompileShader>> shaders,
        SkSpan<const std::pair</*input =*/sk_sp<SkColorSpace>,
                               /*output=*/sk_sp<SkColorSpace>>> inputAndOutputSpaces) {
    return sk_make_sp<PrecompileWorkingColorSpaceShader>(std::move(shaders),
                                                         std::move(inputAndOutputSpaces));
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumWrappedCombos);

        LocalMatrixShaderBlock::LMShaderData kIgnoredLMShaderData(SkMatrix::I());

        LocalMatrixShaderBlock::BeginBlock(keyContext, kIgnoredLMShaderData);

        AddToKey<PrecompileShader>(keyContext, fWrapped, desiredCombination);

        keyContext.paintParamsKeyBuilder()->endBlock();
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
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

        const SkRuntimeEffect* effect = GetKnownRuntimeEffect(kIDs[desiredBlurCombination]);
        SkASSERT(effect->children().size() == 1);

        RuntimeEffectBlock::BeginBlock(keyContext, { sk_ref_sp(effect) });
            fWrapped->priv().addToKey(keyContext.forRuntimeEffect(effect, /*child=*/0),
                                      desiredWrappedCombination);
        keyContext.paintParamsKeyBuilder()->endBlock();
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
        fRawImageShader = PrecompileShaders::RawImage();
        fNumRawImageShaderCombos = fRawImageShader->priv().numCombinations();
    }

private:
    int numIntrinsicCombinations() const override {
        // The uniform version only has one option but the two texture-based versions will
        // have as many combinations as the raw image shader.
        return 1 + 2 * fNumRawImageShaderCombos;
    }

    int numChildCombinations() const override { return fNumWrappedCombos; }

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {

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

        const SkRuntimeEffect* effect = GetKnownRuntimeEffect(stableKey);

        RuntimeEffectBlock::BeginBlock(keyContext, { sk_ref_sp(effect) });
            fWrapped->priv().addToKey(keyContext.forRuntimeEffect(effect, /*child=*/0),
                                      desiredWrappedCombination);
            if (stableKey != SkKnownRuntimeEffects::StableKey::kMatrixConvUniforms) {
                SkASSERT(effect->children().size() == 2);
                fRawImageShader->priv().addToKey(keyContext.forRuntimeEffect(effect, /*child=*/1),
                                                 desiredTextureCombination);
            } else {
                SkASSERT(effect->children().size() == 1);
            }
        keyContext.paintParamsKeyBuilder()->endBlock();
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumWrappedCombos);

        const SkRuntimeEffect* effect = GetKnownRuntimeEffect(fStableKey);
        SkASSERT(effect->children().size() == 1);

        RuntimeEffectBlock::BeginBlock(keyContext, { sk_ref_sp(effect) });
            fWrapped->priv().addToKey(keyContext.forRuntimeEffect(effect, /*child=*/0),
                                      desiredCombination);
        keyContext.paintParamsKeyBuilder()->endBlock();
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numChildCombinations());

        const int desiredDisplacementCombination = desiredCombination % fNumDisplacementCombos;
        const int desiredColorCombination = desiredCombination / fNumDisplacementCombos;
        SkASSERT(desiredColorCombination < fNumColorCombos);

        const SkRuntimeEffect* effect =
                GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kDisplacement);
        SkASSERT(effect->children().size() == 2);

        RuntimeEffectBlock::BeginBlock(keyContext, { sk_ref_sp(effect) });
            fDisplacement->priv().addToKey(keyContext.forRuntimeEffect(effect, /*child=*/0),
                                           desiredDisplacementCombination);
            fColor->priv().addToKey(keyContext.forRuntimeEffect(effect, /*child=*/1),
                                    desiredColorCombination);
        keyContext.paintParamsKeyBuilder()->endBlock();
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

    void addToKey(const KeyContext& keyContext, int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumWrappedCombos);

        const SkRuntimeEffect* normalEffect =
                GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kNormal);
        const SkRuntimeEffect* lightingEffect =
                GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kLighting);
        SkASSERT(normalEffect->children().size() == 1 &&
                 lightingEffect->children().size() == 1);

        KeyContext lightingContext = keyContext.forRuntimeEffect(lightingEffect, /*child=*/0);
        KeyContext normalContext = lightingContext.forRuntimeEffect(normalEffect, /*child=*/0);

        RuntimeEffectBlock::BeginBlock(keyContext, { sk_ref_sp(lightingEffect) });
            RuntimeEffectBlock::BeginBlock(lightingContext, { sk_ref_sp(normalEffect) });
                fWrapped->priv().addToKey(normalContext, desiredCombination);
            keyContext.paintParamsKeyBuilder()->endBlock();
        keyContext.paintParamsKeyBuilder()->endBlock();
    }

    sk_sp<PrecompileShader> fWrapped;
    int fNumWrappedCombos;
};

sk_sp<PrecompileShader> PrecompileShadersPriv::Lighting(sk_sp<PrecompileShader> wrapped) {
    return sk_make_sp<PrecompileLightingShader>(std::move(wrapped));
}

//--------------------------------------------------------------------------------------------------

} // namespace skgpu::graphite
