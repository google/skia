/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"

#include "include/core/SkSamplingOptions.h"
#include "include/gpu/graphite/precompile/PrecompileBase.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/gpu/graphite/FactoryFunctionsPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"

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
class PrecompileBlendFilterImageFilter : public PrecompileImageFilter {
public:
    PrecompileBlendFilterImageFilter(sk_sp<PrecompileBlender> blender,
                                     SkSpan<sk_sp<PrecompileImageFilter>> inputs)
            : PrecompileImageFilter(std::move(inputs))
            , fBlender(std::move(blender)) {
    }

private:
    void onCreatePipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        PaintOptions paintOptions;

        sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
                PrecompileImageShaderFlags::kExcludeAlpha |
                PrecompileImageShaderFlags::kExcludeCubic);

        sk_sp<PrecompileShader> blendShader = PrecompileShaders::Blend(
                SkSpan<const sk_sp<PrecompileBlender>>(&fBlender, 1),
                { imageShader },
                { imageShader });

        paintOptions.setShaders({ std::move(blendShader) });

        paintOptions.priv().buildCombinations(keyContext,
                                              gatherer,
                                              DrawTypeFlags::kSimpleShape,
                                              /* withPrimitiveBlender= */ false,
                                              Coverage::kSingleChannel,
                                              processCombination);
    }

    sk_sp<PrecompileBlender> fBlender;
};

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Arithmetic(
        sk_sp<PrecompileImageFilter> background,
        sk_sp<PrecompileImageFilter> foreground) {
    return Blend(PrecompileBlenders::Arithmetic(), std::move(background), std::move(foreground));
}

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Blend(
        SkBlendMode bm,
        sk_sp<PrecompileImageFilter> background,
        sk_sp<PrecompileImageFilter> foreground) {
    return Blend(PrecompileBlenders::Mode(bm), std::move(background), std::move(foreground));
}

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Blend(
        sk_sp<PrecompileBlender> blender,
        sk_sp<PrecompileImageFilter> background,
        sk_sp<PrecompileImageFilter> foreground) {

    if (!blender) {
        blender = PrecompileBlenders::Mode(SkBlendMode::kSrcOver);
    }

    if (std::optional<SkBlendMode> bm = blender->priv().asBlendMode()) {
        if (bm == SkBlendMode::kSrc) {
            return foreground;
        } else if (bm == SkBlendMode::kDst) {
            return background;
        } else if (bm == SkBlendMode::kClear) {
            return nullptr; // TODO: actually return PrecompileImageFilters::Empty
        }
    }

    sk_sp<PrecompileImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    return sk_make_sp<PrecompileBlendFilterImageFilter>(std::move(blender), inputs);
}

namespace {

void create_blur_imagefilter_pipelines(
        const KeyContext& keyContext,
        PipelineDataGatherer* gatherer,
        const PaintOptionsPriv::ProcessCombination& processCombination) {

    PaintOptions blurPaintOptions;

    // For blur imagefilters we know we don't have alpha-only textures and don't need cubic
    // filtering.
    sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
            PrecompileImageShaderFlags::kExcludeAlpha | PrecompileImageShaderFlags::kExcludeCubic);

    static const SkBlendMode kBlurBlendModes[] = { SkBlendMode::kSrc };
    blurPaintOptions.setShaders({ PrecompileShadersPriv::Blur(imageShader) });
    blurPaintOptions.setBlendModes(kBlurBlendModes);

    blurPaintOptions.priv().buildCombinations(keyContext,
                                              gatherer,
                                              DrawTypeFlags::kSimpleShape,
                                              /* withPrimitiveBlender= */ false,
                                              Coverage::kSingleChannel,
                                              processCombination);
}

} // anonymous namespace

class PrecompileBlurImageFilter : public PrecompileImageFilter {
public:
    PrecompileBlurImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
            : PrecompileImageFilter(std::move(inputs)) {
    }

private:
    void onCreatePipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        create_blur_imagefilter_pipelines(keyContext, gatherer, processCombination);
    }
};

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Blur(
            sk_sp<PrecompileImageFilter> input) {
    return sk_make_sp<PrecompileBlurImageFilter>(SkSpan(&input, 1));
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorFilterImageFilter : public PrecompileImageFilter {
public:
    PrecompileColorFilterImageFilter(sk_sp<PrecompileColorFilter> colorFilter,
                                     sk_sp<PrecompileImageFilter> input)
            : PrecompileImageFilter(SkSpan(&input, 1))
            , fColorFilter(std::move(colorFilter)) {
    }

private:
    sk_sp<PrecompileColorFilter> isColorFilterNode() const override {
        return fColorFilter;
    }

    void onCreatePipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {
        PaintOptions paintOptions;

        sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
                PrecompileImageShaderFlags::kExcludeAlpha |
                PrecompileImageShaderFlags::kExcludeCubic);

        static const SkBlendMode kBlendModes[] = { SkBlendMode::kDstOut };
        paintOptions.setShaders({ std::move(imageShader) });
        paintOptions.setColorFilters({ fColorFilter });
        paintOptions.setBlendModes(kBlendModes);

        paintOptions.priv().buildCombinations(keyContext,
                                              gatherer,
                                              DrawTypeFlags::kSimpleShape,
                                              /* withPrimitiveBlender= */ false,
                                              Coverage::kSingleChannel,
                                              processCombination);
    }

    sk_sp<PrecompileColorFilter> fColorFilter;
};

sk_sp<PrecompileImageFilter> PrecompileImageFilters::ColorFilter(
        sk_sp<PrecompileColorFilter> colorFilter,
        sk_sp<PrecompileImageFilter> input) {
    if (colorFilter && input) {
        sk_sp<PrecompileColorFilter> inputCF = input->isColorFilterNode();
        if (inputCF) {
            colorFilter = colorFilter->makeComposed(std::move(inputCF));
            input = sk_ref_sp(input->getInput(0));
        }
    }

    sk_sp<PrecompileImageFilter> filter = std::move(input);
    if (colorFilter) {
        filter = sk_make_sp<PrecompileColorFilterImageFilter>(std::move(colorFilter),
                                                              std::move(filter));
    }
    return filter;
}

//--------------------------------------------------------------------------------------------------
class PrecompileDisplacementMapImageFilter : public PrecompileImageFilter {
public:
    PrecompileDisplacementMapImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
            : PrecompileImageFilter(std::move(inputs)) {
    }

private:
    void onCreatePipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        PaintOptions displacement;

        // For displacement imagefilters we know we don't have alpha-only textures and don't need
        // cubic filtering.
        sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
                PrecompileImageShaderFlags::kExcludeAlpha |
                PrecompileImageShaderFlags::kExcludeCubic);

        displacement.setShaders({ PrecompileShadersPriv::Displacement(imageShader, imageShader) });

        displacement.priv().buildCombinations(keyContext,
                                              gatherer,
                                              DrawTypeFlags::kSimpleShape,
                                              /* withPrimitiveBlender= */ false,
                                              Coverage::kSingleChannel,
                                              processCombination);
    }
};

sk_sp<PrecompileImageFilter> PrecompileImageFilters::DisplacementMap(
            sk_sp<PrecompileImageFilter> input) {
    return sk_make_sp<PrecompileDisplacementMapImageFilter>(SkSpan(&input, 1));
}

//--------------------------------------------------------------------------------------------------
class PrecompileLightingImageFilter : public PrecompileImageFilter {
public:
    PrecompileLightingImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
            : PrecompileImageFilter(std::move(inputs)) {
    }

private:
    void onCreatePipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
                PrecompileImageShaderFlags::kExcludeAlpha |
                PrecompileImageShaderFlags::kExcludeCubic);

        PaintOptions lighting;
        lighting.setShaders({ PrecompileShadersPriv::Lighting(std::move(imageShader)) });

        lighting.priv().buildCombinations(keyContext,
                                          gatherer,
                                          DrawTypeFlags::kSimpleShape,
                                          /* withPrimitiveBlender= */ false,
                                          Coverage::kSingleChannel,
                                          processCombination);
    }
};

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Lighting(
            sk_sp<PrecompileImageFilter> input) {
    return sk_make_sp<PrecompileLightingImageFilter>(SkSpan(&input, 1));
}

//--------------------------------------------------------------------------------------------------
class PrecompileMatrixConvolutionImageFilter : public PrecompileImageFilter {
public:
    PrecompileMatrixConvolutionImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
            : PrecompileImageFilter(std::move(inputs)) {
    }

private:
    void onCreatePipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        PaintOptions matrixConv;

        // For matrix convolution imagefilters we know we don't have alpha-only textures and don't
        // need cubic filtering.
        sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
                PrecompileImageShaderFlags::kExcludeAlpha |
                PrecompileImageShaderFlags::kExcludeCubic);

        matrixConv.setShaders({ PrecompileShadersPriv::MatrixConvolution(imageShader) });

        matrixConv.priv().buildCombinations(keyContext,
                                            gatherer,
                                            DrawTypeFlags::kSimpleShape,
                                            /* withPrimitiveBlender= */ false,
                                            Coverage::kSingleChannel,
                                            processCombination);
    }
};

sk_sp<PrecompileImageFilter> PrecompileImageFilters::MatrixConvolution(
            sk_sp<PrecompileImageFilter> input) {
    return sk_make_sp<PrecompileMatrixConvolutionImageFilter>(SkSpan(&input, 1));
}

//--------------------------------------------------------------------------------------------------
class PrecompileMorphologyImageFilter : public PrecompileImageFilter {
public:
    PrecompileMorphologyImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
            : PrecompileImageFilter(std::move(inputs)) {
    }

private:
    void onCreatePipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        // For morphology imagefilters we know we don't have alpha-only textures and don't need
        // cubic filtering.
        sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
            PrecompileImageShaderFlags::kExcludeAlpha | PrecompileImageShaderFlags::kExcludeCubic);

        {
            PaintOptions sparse;

            static const SkBlendMode kBlendModes[] = { SkBlendMode::kSrc };
            sparse.setShaders({ PrecompileShadersPriv::SparseMorphology(imageShader) });
            sparse.setBlendModes(kBlendModes);

            sparse.priv().buildCombinations(keyContext,
                                            gatherer,
                                            DrawTypeFlags::kSimpleShape,
                                            /* withPrimitiveBlender= */ false,
                                            Coverage::kSingleChannel,
                                            processCombination);
        }

        {
            PaintOptions linear;

            static const SkBlendMode kBlendModes[] = { SkBlendMode::kSrcOver };
            linear.setShaders({ PrecompileShadersPriv::LinearMorphology(std::move(imageShader)) });
            linear.setBlendModes(kBlendModes);

            linear.priv().buildCombinations(keyContext,
                                            gatherer,
                                            DrawTypeFlags::kSimpleShape,
                                            /* withPrimitiveBlender= */ false,
                                            Coverage::kSingleChannel,
                                            processCombination);
        }
    }
};

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Morphology(
        sk_sp<PrecompileImageFilter> input) {
    return sk_make_sp<PrecompileMorphologyImageFilter>(SkSpan(&input, 1));
}

//--------------------------------------------------------------------------------------------------
// TODO(b/342413572): the analytic blurmasks are triggered off of the simple DrawType thus
// over-generate when a simple draw doesn't have a blur mask.
class PrecompileBlurMaskFilter : public PrecompileMaskFilter {
public:
    PrecompileBlurMaskFilter() {}

private:
    void createPipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {
        create_blur_imagefilter_pipelines(keyContext, gatherer, processCombination);
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
sk_sp<PrecompileColorFilter> PrecompileColorFilters::HighContrast() {
    const SkRuntimeEffect* highContrastEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kHighContrast);

    sk_sp<PrecompileColorFilter> cf = MakePrecompileColorFilter(sk_ref_sp(highContrastEffect));
    if (!cf) {
        return nullptr;
    }
    return PrecompileColorFiltersPriv::WithWorkingFormat({ std::move(cf) });
}

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileColorFilter> PrecompileColorFilters::Luma() {
    const SkRuntimeEffect* lumaEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kLuma);

    return MakePrecompileColorFilter(sk_ref_sp(lumaEffect));
}

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileColorFilter> PrecompileColorFilters::Overdraw() {
    const SkRuntimeEffect* overdrawEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kOverdraw);

    return MakePrecompileColorFilter(sk_ref_sp(overdrawEffect));
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
            numOptions *= childOption.base()->priv().numCombinations();
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

        const int numChildCombos = childOption.base() ? childOption.base()->priv().numCombinations()
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
