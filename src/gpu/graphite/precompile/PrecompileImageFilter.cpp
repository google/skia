/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"

#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileImageFilterPriv.h"
#include "src/gpu/graphite/precompile/PrecompileImageFiltersPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShadersPriv.h"

namespace skgpu::graphite {

using PrecompileShaders::ImageShaderFlags;

//--------------------------------------------------------------------------------------------------
PrecompileImageFilter::PrecompileImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
        : PrecompileBase(Type::kImageFilter) {
    fInputs.reset(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i) {
        fInputs[i] = inputs[i];
    }
}

PrecompileImageFilter::~PrecompileImageFilter() = default;

sk_sp<PrecompileColorFilter> PrecompileImageFilter::asAColorFilter() const {
    sk_sp<PrecompileColorFilter> tmp = this->isColorFilterNode();
    if (!tmp) {
        return nullptr;
    }
    SkASSERT(this->countInputs() == 1);
    if (this->getInput(0)) {
        return nullptr;
    }
    // TODO: as in SkImageFilter::asAColorFilter, handle the special case of
    // affectsTransparentBlack. This is tricky for precompilation since we don't,
    // necessarily, have all the parameters of the ColorFilter in order to evaluate
    // filterColor4f(SkColors::kTransparent) - the normal API's implementation.
    return tmp;
}

void PrecompileImageFilter::createPipelines(
        const KeyContext& keyContext,
        PipelineDataGatherer* gatherer,
        const RenderPassDesc& renderPassDesc,
        const PaintOptions::ProcessCombination& processCombination) {
    // TODO: we will want to mark already visited nodes to prevent loops and track
    // already created Pipelines so we don't over-generate too much (e.g., if a DAG
    // has multiple blurs we don't want to keep trying to create all the blur pipelines).
    this->onCreatePipelines(keyContext, gatherer, renderPassDesc, processCombination);

    for (const sk_sp<PrecompileImageFilter>& input : fInputs) {
        if (input) {
            input->createPipelines(keyContext, gatherer, renderPassDesc, processCombination);
        }
    }
}

//--------------------------------------------------------------------------------------------------
namespace PrecompileImageFiltersPriv {

void CreateBlurImageFilterPipelines(
        const KeyContext& keyContext,
        PipelineDataGatherer* gatherer,
        const RenderPassDesc& renderPassDesc,
        const PaintOptionsPriv::ProcessCombination& processCombination) {

    PaintOptions blurPaintOptions;

    // For blur imagefilters we know we don't have alpha-only textures and don't need cubic
    // filtering.
    sk_sp<PrecompileShader> imageShader = PrecompileShaders::Image(
            ImageShaderFlags::kNoAlphaNoCubic);

    static const SkBlendMode kBlurBlendModes[] = { SkBlendMode::kSrc };
    blurPaintOptions.setShaders({ PrecompileShadersPriv::Blur(imageShader) });
    blurPaintOptions.setBlendModes(kBlurBlendModes);

    blurPaintOptions.priv().buildCombinations(keyContext,
                                              gatherer,
                                              DrawTypeFlags::kSimpleShape,
                                              /* withPrimitiveBlender= */ false,
                                              Coverage::kSingleChannel,
                                              renderPassDesc,
                                              processCombination);
}

} // namespace PrecompileImageFiltersPriv

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
            const RenderPassDesc& renderPassDesc,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        PaintOptions paintOptions;

        sk_sp<PrecompileShader> imageShader = PrecompileShaders::Image(
                ImageShaderFlags::kNoAlphaNoCubic);

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
                                              renderPassDesc,
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

//--------------------------------------------------------------------------------------------------
class PrecompileBlurImageFilter : public PrecompileImageFilter {
public:
    PrecompileBlurImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
            : PrecompileImageFilter(std::move(inputs)) {
    }

private:
    void onCreatePipelines(
            const KeyContext& keyContext,
            PipelineDataGatherer* gatherer,
            const RenderPassDesc& renderPassDesc,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        PrecompileImageFiltersPriv::CreateBlurImageFilterPipelines(keyContext, gatherer,
                                                                   renderPassDesc,
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
            const RenderPassDesc& renderPassDesc,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {
        PaintOptions paintOptions;

        sk_sp<PrecompileShader> imageShader = PrecompileShaders::Image(
                ImageShaderFlags::kNoAlphaNoCubic);

        static const SkBlendMode kBlendModes[] = { SkBlendMode::kDstOut };
        paintOptions.setShaders({ std::move(imageShader) });
        paintOptions.setColorFilters({ fColorFilter });
        paintOptions.setBlendModes(kBlendModes);

        paintOptions.priv().buildCombinations(keyContext,
                                              gatherer,
                                              DrawTypeFlags::kSimpleShape,
                                              /* withPrimitiveBlender= */ false,
                                              Coverage::kSingleChannel,
                                              renderPassDesc,
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
            const RenderPassDesc& renderPassDesc,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        PaintOptions displacement;

        // For displacement imagefilters we know we don't have alpha-only textures and don't need
        // cubic filtering.
        sk_sp<PrecompileShader> imageShader = PrecompileShaders::Image(
                ImageShaderFlags::kNoAlphaNoCubic);

        displacement.setShaders({ PrecompileShadersPriv::Displacement(imageShader, imageShader) });

        displacement.priv().buildCombinations(keyContext,
                                              gatherer,
                                              DrawTypeFlags::kSimpleShape,
                                              /* withPrimitiveBlender= */ false,
                                              Coverage::kSingleChannel,
                                              renderPassDesc,
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
            const RenderPassDesc& renderPassDesc,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        sk_sp<PrecompileShader> imageShader = PrecompileShaders::Image(
                ImageShaderFlags::kNoAlphaNoCubic);

        PaintOptions lighting;
        lighting.setShaders({ PrecompileShadersPriv::Lighting(std::move(imageShader)) });

        lighting.priv().buildCombinations(keyContext,
                                          gatherer,
                                          DrawTypeFlags::kSimpleShape,
                                          /* withPrimitiveBlender= */ false,
                                          Coverage::kSingleChannel,
                                          renderPassDesc,
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
            const RenderPassDesc& renderPassDesc,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        PaintOptions matrixConv;

        // For matrix convolution imagefilters we know we don't have alpha-only textures and don't
        // need cubic filtering.
        sk_sp<PrecompileShader> imageShader = PrecompileShaders::Image(
                ImageShaderFlags::kNoAlphaNoCubic);

        matrixConv.setShaders({ PrecompileShadersPriv::MatrixConvolution(imageShader) });

        matrixConv.priv().buildCombinations(keyContext,
                                            gatherer,
                                            DrawTypeFlags::kSimpleShape,
                                            /* withPrimitiveBlender= */ false,
                                            Coverage::kSingleChannel,
                                            renderPassDesc,
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
            const RenderPassDesc& renderPassDesc,
            const PaintOptionsPriv::ProcessCombination& processCombination) const override {

        // For morphology imagefilters we know we don't have alpha-only textures and don't need
        // cubic filtering.
        sk_sp<PrecompileShader> imageShader = PrecompileShaders::Image(
                ImageShaderFlags::kNoAlphaNoCubic);

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
                                            renderPassDesc,
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
                                            renderPassDesc,
                                            processCombination);
        }
    }
};

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Morphology(
        sk_sp<PrecompileImageFilter> input) {
    return sk_make_sp<PrecompileMorphologyImageFilter>(SkSpan(&input, 1));
}

} // namespace skgpu::graphite
