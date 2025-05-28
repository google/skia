/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "src/core/SkColorData.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/effects/colorfilters/SkWorkingFormatColorFilter.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileColorFiltersPriv.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
PrecompileColorFilter::~PrecompileColorFilter() = default;

sk_sp<PrecompileColorFilter> PrecompileColorFilter::makeComposed(
        sk_sp<PrecompileColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return PrecompileColorFilters::Compose({ sk_ref_sp(this) }, { std::move(inner) });
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
namespace {

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
class PrecompileComposeColorFilter : public PrecompileColorFilter {
public:
    PrecompileComposeColorFilter(SkSpan<const sk_sp<PrecompileColorFilter>> outerOptions,
                                 SkSpan<const sk_sp<PrecompileColorFilter>> innerOptions)
            : fOuterOptions(outerOptions.begin(), outerOptions.end())
            , fInnerOptions(innerOptions.begin(), innerOptions.end()) {

        fNumOuterCombos = 0;
        for (const auto& outerOption : fOuterOptions) {
            fNumOuterCombos += outerOption ? outerOption->priv().numCombinations() : 1;
        }

        fNumInnerCombos = 0;
        for (const auto& innerOption : fInnerOptions) {
            fNumInnerCombos += innerOption ? innerOption->priv().numCombinations() : 1;
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
class PrecompileBlendModeColorFilter : public PrecompileColorFilter {
public:
    PrecompileBlendModeColorFilter(SkSpan<const SkBlendMode> blendModes)
            : fBlendOptions(blendModes) {}

private:
    int numIntrinsicCombinations() const override {
        return fBlendOptions.numCombinations();
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        auto [blender, option ] = fBlendOptions.selectOption(desiredCombination);
        SkASSERT(option == 0 && blender->priv().asBlendMode().has_value());

        SkBlendMode representativeBlendMode = *blender->priv().asBlendMode();

        // Here the color is just a stand-in for a later value.
        AddBlendModeColorFilter(keyContext, builder, gatherer,
                                representativeBlendMode, SK_PMColor4fWHITE);
    }

    // NOTE: The BlendMode color filter can only be created with SkBlendModes, not arbitrary
    // SkBlenders, so this list will only contain consolidated blend functions or fixed blend mode
    // options.
    PrecompileBlenderList fBlendOptions;
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Blend() {
    static constexpr SkBlendMode kAllBlendOptions[15] = {
        SkBlendMode::kSrcOver, // Trigger porter-duff blends
        SkBlendMode::kHue,     // Trigger HSLC blends
        // All remaining fixed blend modes:
        SkBlendMode::kPlus,
        SkBlendMode::kModulate,
        SkBlendMode::kScreen,
        SkBlendMode::kOverlay,
        SkBlendMode::kDarken,
        SkBlendMode::kLighten,
        SkBlendMode::kColorDodge,
        SkBlendMode::kColorBurn,
        SkBlendMode::kHardLight,
        SkBlendMode::kSoftLight,
        SkBlendMode::kDifference,
        SkBlendMode::kExclusion,
        SkBlendMode::kMultiply
    };
    return Blend(kAllBlendOptions);
}

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Blend(SkSpan<const SkBlendMode> blendModes) {
    return sk_make_sp<PrecompileBlendModeColorFilter>(blendModes);
}

//--------------------------------------------------------------------------------------------------
class PrecompileMatrixColorFilter : public PrecompileColorFilter {
public:
    PrecompileMatrixColorFilter(bool inHSLA) : fInHSLA(inHSLA) {}

private:
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination == 0);

        static constexpr float kIdentity[20] = { 1, 0, 0, 0, 0,
                                                 0, 1, 0, 0, 0,
                                                 0, 0, 1, 0, 0,
                                                 0, 0, 0, 1, 0 };

        MatrixColorFilterBlock::MatrixColorFilterData matrixCFData(
                kIdentity, fInHSLA, /* clamp= */ true);

        MatrixColorFilterBlock::AddBlock(keyContext, builder, gatherer, matrixCFData);
    }

    bool fInHSLA;
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Matrix() {
    return sk_make_sp<PrecompileMatrixColorFilter>(/*inHSLA=*/false);
}

sk_sp<PrecompileColorFilter> PrecompileColorFilters::HSLAMatrix() {
    return sk_make_sp<PrecompileMatrixColorFilter>(/*inHSLA=*/true);
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorSpaceXformColorFilter : public PrecompileColorFilter {
public:
    PrecompileColorSpaceXformColorFilter(SkSpan<const sk_sp<SkColorSpace>> src,
                                         SkSpan<const sk_sp<SkColorSpace>> dst)
            : fSrc(src.begin(), src.end())
            , fDst(dst.begin(), dst.end())
            , fNumCombinations(src.size() * dst.size()) {}

private:
    int numIntrinsicCombinations() const override { return fNumCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        const int srcCombination = desiredCombination % fSrc.size();
        const int dstCombination = desiredCombination / fSrc.size();
        SkASSERT(dstCombination < static_cast<int>(fDst.size()));

        // The alpha type is unused for determining which color space transform block to use.
        constexpr SkAlphaType kAlphaType = kPremul_SkAlphaType;

        ColorSpaceTransformBlock::ColorSpaceTransformData csData =
                ColorSpaceTransformBlock::ColorSpaceTransformData(
                        fSrc[srcCombination].get(), kAlphaType,
                        fDst[dstCombination].get(), kAlphaType);

        ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, csData);
    }

    std::vector<sk_sp<SkColorSpace>> fSrc;
    std::vector<sk_sp<SkColorSpace>> fDst;

    const int fNumCombinations;
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::LinearToSRGBGamma() {
    return PrecompileColorFiltersPriv::ColorSpaceXform({ SkColorSpace::MakeSRGBLinear() },
                                                       { SkColorSpace::MakeSRGB() });
}

sk_sp<PrecompileColorFilter> PrecompileColorFilters::SRGBToLinearGamma() {
    return PrecompileColorFiltersPriv::ColorSpaceXform({ SkColorSpace::MakeSRGB() },
                                                       { SkColorSpace::MakeSRGBLinear() });
}

sk_sp<PrecompileColorFilter> PrecompileColorFiltersPriv::ColorSpaceXform(
        SkSpan<const sk_sp<SkColorSpace>> src, SkSpan<const sk_sp<SkColorSpace>> dst) {
    return sk_make_sp<PrecompileColorSpaceXformColorFilter>(src, dst);
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

    skia_private::TArray<sk_sp<PrecompileBase>> dsts, srcs;
    dsts.reserve(dstOptions.size());
    for (const sk_sp<PrecompileColorFilter>& d : dstOptions) {
        dsts.push_back(d);
    }

    srcs.reserve(srcOptions.size());
    for (const sk_sp<PrecompileColorFilter>& s : srcOptions) {
        srcs.push_back(s);
    }

    return PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(lerpEffect),
                                                               { dsts, srcs });
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
sk_sp<PrecompileColorFilter> PrecompileColorFilters::HighContrast() {
    const SkRuntimeEffect* highContrastEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kHighContrast);

    sk_sp<PrecompileColorFilter> cf =
            PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(highContrastEffect));
    if (!cf) {
        return nullptr;
    }

    // These color space working format arguments should match those from
    // src/effects/SkHighContrastFilter.cpp.
    const skcms_TransferFunction kTF = SkNamedTransferFn::kLinear;
    const SkAlphaType kUnpremul = kUnpremul_SkAlphaType;
    return PrecompileColorFiltersPriv::WithWorkingFormat(
            {std::move(cf)}, &kTF, /* gamut= */ nullptr, &kUnpremul);
}

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileColorFilter> PrecompileColorFilters::Luma() {
    const SkRuntimeEffect* lumaEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kLuma);

    return PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(lumaEffect));
}

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileColorFilter> PrecompileColorFilters::Overdraw() {
    const SkRuntimeEffect* overdrawEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kOverdraw);

    return PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(overdrawEffect));
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
class PrecompileWithWorkingFormatColorFilter : public PrecompileColorFilter {
public:
    PrecompileWithWorkingFormatColorFilter(SkSpan<const sk_sp<PrecompileColorFilter>> childOptions,
                                           const skcms_TransferFunction* tf,
                                           const skcms_Matrix3x3* gamut,
                                           const SkAlphaType* at)
            : fChildOptions(childOptions.begin(), childOptions.end())
            , fWorkingFormatCalculator(tf, gamut, at) {
        fNumChildCombos = 0;
        for (const auto& childOption : fChildOptions) {
            fNumChildCombos += childOption->priv().numCombinations();
        }
    }

private:
    int numChildCombinations() const override { return fNumChildCombos; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumChildCombos);

        SkAlphaType unusedWorkingAT;
        const sk_sp<SkColorSpace> dstCS = keyContext.dstColorInfo().colorSpace()
                                                  ? keyContext.dstColorInfo().refColorSpace()
                                                  : SkColorSpace::MakeSRGB();
        const sk_sp<SkColorSpace> workingCS =
                fWorkingFormatCalculator.workingFormat(dstCS, &unusedWorkingAT);

        // The alpha type is unused for determining which color space transform block to use.
        constexpr SkAlphaType kAlphaType = kPremul_SkAlphaType;

        // Use two nested compose blocks to chain (dst->working), child, and (working->dst) together
        // while appearing as one block to the parent node.
        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    // Inner compose
                    Compose(keyContext, builder, gatherer,
                            /* addInnerToKey= */ [&]() -> void {
                                // Innermost (inner of inner compose)
                                ColorSpaceTransformBlock::ColorSpaceTransformData data1(
                                        dstCS.get(), kAlphaType, workingCS.get(), kAlphaType);
                                ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer,
                                                                   data1);
                            },
                            /* addOuterToKey= */ [&]() -> void {
                                // Middle (outer of inner compose)
                                AddToKey<PrecompileColorFilter>(keyContext, builder, gatherer,
                                                                fChildOptions, desiredCombination);
                            });
                },
                /* addOuterToKey= */ [&]() -> void {
                    // Outermost (outer of outer compose)
                    ColorSpaceTransformBlock::ColorSpaceTransformData data2(
                            workingCS.get(), kAlphaType, dstCS.get(), kAlphaType);
                    ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, data2);
                });
    }

    std::vector<sk_sp<PrecompileColorFilter>> fChildOptions;

    int fNumChildCombos;

    SkWorkingFormatCalculator fWorkingFormatCalculator;
};

sk_sp<PrecompileColorFilter> PrecompileColorFiltersPriv::WithWorkingFormat(
        SkSpan<const sk_sp<PrecompileColorFilter>> childOptions,
        const skcms_TransferFunction* tf,
        const skcms_Matrix3x3* gamut,
        const SkAlphaType* at) {
    return sk_make_sp<PrecompileWithWorkingFormatColorFilter>(childOptions, tf, gamut, at);
}

} // namespace skgpu::graphite
