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
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileImageFilterPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShadersPriv.h"

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

namespace PrecompileImageFiltersPriv {

void CreateBlurImageFilterPipelines(
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

} // namespace PrecompileImageFiltersPriv

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

        PrecompileImageFiltersPriv::CreateBlurImageFilterPipelines(keyContext, gatherer,
                                                                   processCombination);
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
        sk_sp<PrecompileColorFilter> inputCF = input->priv().isColorFilterNode();
        if (inputCF) {
            colorFilter = colorFilter->makeComposed(std::move(inputCF));
            input = sk_ref_sp(input->priv().getInput(0));
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
