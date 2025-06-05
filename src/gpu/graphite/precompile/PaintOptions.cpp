/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PaintOptions.h"

#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"
#include "include/gpu/graphite/precompile/PrecompileMaskFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/precompile/PaintOption.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShadersPriv.h"

namespace skgpu::graphite {

PaintOptions::PaintOptions() = default;
PaintOptions::PaintOptions(const PaintOptions&) = default;
PaintOptions::~PaintOptions() = default;
PaintOptions& PaintOptions::operator=(const PaintOptions&) = default;

void PaintOptions::setShaders(SkSpan<const sk_sp<PrecompileShader>> shaders) {
    fShaderOptions.clear();
    fShaderOptions.push_back_n(shaders.size(), shaders.data());
}

void PaintOptions::setImageFilters(SkSpan<const sk_sp<PrecompileImageFilter>> imageFilters) {
    fImageFilterOptions.clear();
    fImageFilterOptions.push_back_n(imageFilters.size(), imageFilters.data());
}

void PaintOptions::setMaskFilters(SkSpan<const sk_sp<PrecompileMaskFilter>> maskFilters) {
    fMaskFilterOptions.clear();
    fMaskFilterOptions.push_back_n(maskFilters.size(), maskFilters.data());
}

void PaintOptions::setColorFilters(SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters) {
    fColorFilterOptions.clear();
    fColorFilterOptions.push_back_n(colorFilters.size(), colorFilters.data());
}

void PaintOptions::addColorFilter(sk_sp<PrecompileColorFilter> cf) {
    fColorFilterOptions.push_back(std::move(cf));
}

void PaintOptions::setBlendModes(SkSpan<const SkBlendMode> blendModes) {
    fBlendModeOptions.clear();
    fBlendModeOptions.push_back_n(blendModes.size(), blendModes.data());
}

void PaintOptions::setBlenders(SkSpan<const sk_sp<PrecompileBlender>> blenders) {
    for (const sk_sp<PrecompileBlender>& b: blenders) {
        if (b->priv().asBlendMode().has_value()) {
            fBlendModeOptions.push_back(b->priv().asBlendMode().value());
        } else {
            fBlenderOptions.push_back(b);
        }
    }
}

void PaintOptions::setClipShaders(SkSpan<const sk_sp<PrecompileShader>> clipShaders) {
    // In the normal API this modification happens in SkDevice::clipShader()
    fClipShaderOptions.reserve(2 * clipShaders.size());
    for (const sk_sp<PrecompileShader>& cs : clipShaders) {
        // All clipShaders get wrapped in a CTMShader ...
        sk_sp<PrecompileShader> withCTM = cs ? PrecompileShadersPriv::CTM({ cs }) : nullptr;
        // and, if it is a SkClipOp::kDifference clip, an additional ColorFilterShader
        sk_sp<PrecompileShader> inverted =
                withCTM ? withCTM->makeWithColorFilter(PrecompileColorFilters::Blend())
                        : nullptr;

        fClipShaderOptions.emplace_back(std::move(withCTM));
        fClipShaderOptions.emplace_back(std::move(inverted));
    }
}

int PaintOptions::numShaderCombinations() const {
    int numShaderCombinations = 0;
    for (const sk_sp<PrecompileShader>& s : fShaderOptions) {
        numShaderCombinations += s->numCombinations();
    }

    // If no shader option is specified we will add a solid color shader option
    return numShaderCombinations ? numShaderCombinations : 1;
}

int PaintOptions::numColorFilterCombinations() const {
    int numColorFilterCombinations = 0;
    for (const sk_sp<PrecompileColorFilter>& cf : fColorFilterOptions) {
        if (!cf) {
            ++numColorFilterCombinations;
        } else {
            numColorFilterCombinations += cf->numCombinations();
        }
    }

    // If no color filter options are specified we will use the unmodified result color
    return numColorFilterCombinations ? numColorFilterCombinations : 1;
}

int PaintOptions::numBlendCombinations() const {
    int numBlendCombos = fBlendModeOptions.size();
    for (const sk_sp<PrecompileBlender>& b: fBlenderOptions) {
        SkASSERT(!b->priv().asBlendMode().has_value());
        numBlendCombos += b->priv().numChildCombinations();
    }

    if (!numBlendCombos) {
        // If the user didn't specify a blender we will fall back to kSrcOver blending
        numBlendCombos = 1;
    }

    return numBlendCombos;
}

int PaintOptions::numClipShaderCombinations() const {
    int numClipShaderCombos = 0;
    for (const sk_sp<PrecompileShader>& cs: fClipShaderOptions) {
        if (cs) {
            numClipShaderCombos += cs->priv().numChildCombinations();
        } else {
            ++numClipShaderCombos;
        }
    }

    // If no clipShader options are specified we will just have the unclipped options
    return numClipShaderCombos ? numClipShaderCombos : 1;
}

int PaintOptions::numCombinations() const {
    return this->numShaderCombinations() *
           this->numColorFilterCombinations() *
           this->numBlendCombinations() *
           this->numClipShaderCombinations();
}

void PaintOptions::createKey(const KeyContext& keyContext,
                             PaintParamsKeyBuilder* keyBuilder,
                             PipelineDataGatherer* gatherer,
                             int desiredCombination,
                             bool addPrimitiveBlender,
                             Coverage coverage) const {
    SkDEBUGCODE(keyBuilder->checkReset();)
    SkASSERT(desiredCombination < this->numCombinations());

    const int numClipShaderCombos = this->numClipShaderCombinations();
    const int numBlendModeCombos = this->numBlendCombinations();
    const int numColorFilterCombinations = this->numColorFilterCombinations();

    const int desiredClipShaderCombination = desiredCombination % numClipShaderCombos;
    int remainingCombinations = desiredCombination / numClipShaderCombos;

    const int desiredBlendCombination = remainingCombinations % numBlendModeCombos;
    remainingCombinations /= numBlendModeCombos;

    const int desiredColorFilterCombination = remainingCombinations % numColorFilterCombinations;
    remainingCombinations /= numColorFilterCombinations;

    const int desiredShaderCombination = remainingCombinations;
    SkASSERT(desiredShaderCombination < this->numShaderCombinations());

    auto clipShader = PrecompileBase::SelectOption(SkSpan(fClipShaderOptions),
                                                   desiredClipShaderCombination);

    std::pair<sk_sp<PrecompileBlender>, int> finalBlender;
    if (desiredBlendCombination < fBlendModeOptions.size()) {
        finalBlender = { PrecompileBlenders::Mode(fBlendModeOptions[desiredBlendCombination]), 0 };
    } else {
        finalBlender = PrecompileBase::SelectOption(
                            SkSpan(fBlenderOptions),
                            desiredBlendCombination - fBlendModeOptions.size());
    }
    if (!finalBlender.first) {
        finalBlender = { PrecompileBlenders::Mode(SkBlendMode::kSrcOver), 0 };
    }

    PrecompileBlender* blender = finalBlender.first.get();
    std::optional<SkBlendMode> blendMode = blender ? blender->priv().asBlendMode()
                                                   : SkBlendMode::kSrcOver;
    PaintOption option(fPaintColorIsOpaque,
                       finalBlender,
                       PrecompileBase::SelectOption(SkSpan(fShaderOptions),
                                                    desiredShaderCombination),
                       PrecompileBase::SelectOption(SkSpan(fColorFilterOptions),
                                                    desiredColorFilterCombination),
                       addPrimitiveBlender,
                       clipShader,
                       /*dstReadRequired=*/!CanUseHardwareBlending(keyContext.caps(),
                                                                   blendMode,
                                                                   coverage),
                       fDither);

    option.toKey(keyContext, keyBuilder, gatherer);
}

namespace {

void create_image_drawing_pipelines(const KeyContext& keyContext,
                                    PipelineDataGatherer* gatherer,
                                    const PaintOptions& orig,
                                    const RenderPassDesc& renderPassDesc,
                                    const PaintOptionsPriv::ProcessCombination& processCombination) {
    PaintOptions imagePaintOptions;

    // For imagefilters we know we don't have alpha-only textures and don't need cubic filtering.
    sk_sp<PrecompileShader> imageShader = PrecompileShaders::Image(
            PrecompileShaders::ImageShaderFlags::kNoAlphaNoCubic);

    imagePaintOptions.setShaders({ imageShader });
    imagePaintOptions.setBlendModes(orig.getBlendModes());
    imagePaintOptions.setBlenders(orig.getBlenders());
    imagePaintOptions.setColorFilters(orig.getColorFilters());
    imagePaintOptions.priv().addColorFilter(nullptr);

    imagePaintOptions.priv().buildCombinations(keyContext,
                                               gatherer,
                                               DrawTypeFlags::kSimpleShape,
                                               /* withPrimitiveBlender= */ false,
                                               Coverage::kSingleChannel,
                                               renderPassDesc,
                                               processCombination);
}

} // anonymous namespace

void PaintOptions::buildCombinations(
        const KeyContext& keyContext,
        PipelineDataGatherer* gatherer,
        DrawTypeFlags drawTypes,
        bool withPrimitiveBlender,
        Coverage coverage,
        const RenderPassDesc& renderPassDesc,
        const ProcessCombination& processCombination) const {

    PaintParamsKeyBuilder builder(keyContext.dict());

    if (!fImageFilterOptions.empty() || !fMaskFilterOptions.empty()) {
        // TODO: split this out into a create_restore_draw_pipelines method
        PaintOptions tmp = *this;

        // When image filtering, the original blend mode is taken over by the restore paint
        tmp.setImageFilters({});
        tmp.setMaskFilters({});
        tmp.addBlendMode(SkBlendMode::kSrcOver);

        if (!fImageFilterOptions.empty()) {
            std::vector<sk_sp<PrecompileColorFilter>> newCFs(tmp.fColorFilterOptions.begin(),
                                                             tmp.fColorFilterOptions.end());
            if (newCFs.empty()) {
                // TODO: I (robertphillips) believe this is unnecessary and is just a result of the
                // base SkPaint generated in the PaintParamsKeyTest not correctly taking CFIFs into
                // account.
                newCFs.push_back(nullptr);
            }

            // As in SkCanvasPriv::ImageToColorFilter, we fuse CFIFs into the base draw's CFs.
            // TODO: in SkCanvasPriv::ImageToColorFilter this fusing of CFIFs and CFs is skipped
            // when there is a maskfilter. For now we over-generate.
            for (const sk_sp<PrecompileImageFilter>& o : fImageFilterOptions) {
                sk_sp<PrecompileColorFilter> imageFiltersCF = o ? o->asAColorFilter() : nullptr;
                if (imageFiltersCF) {
                    if (!tmp.fColorFilterOptions.empty()) {
                        for (const sk_sp<PrecompileColorFilter>& cf : tmp.fColorFilterOptions) {
                            // TODO: if a CFIF was fully handled here it should be removed from the
                            // later loop over fImageFilterOptions. For now we over-generate.
                            sk_sp<PrecompileColorFilter> newCF = imageFiltersCF->makeComposed(cf);
                            newCFs.push_back(std::move(newCF));
                        }
                    } else {
                        newCFs.push_back(imageFiltersCF);
                    }
                }
            }

            tmp.setColorFilters(newCFs);
        }

        tmp.buildCombinations(keyContext, gatherer, drawTypes, withPrimitiveBlender, coverage,
                              renderPassDesc, processCombination);

        create_image_drawing_pipelines(keyContext, gatherer, *this,
                                       renderPassDesc, processCombination);

        for (const sk_sp<PrecompileImageFilter>& o : fImageFilterOptions) {
            o->createPipelines(keyContext, gatherer, renderPassDesc, processCombination);
        }
        for (const sk_sp<PrecompileMaskFilter>& o : fMaskFilterOptions) {
            o->createPipelines(keyContext, gatherer, *this, renderPassDesc, processCombination);
        }
    } else {
        int numCombinations = this->numCombinations();
        for (int i = 0; i < numCombinations; ++i) {
            // Since the precompilation path's uniforms aren't used and don't change the key,
            // the exact layout doesn't matter
            gatherer->resetWithNewLayout(Layout::kMetal);

            this->createKey(keyContext, &builder, gatherer, i, withPrimitiveBlender, coverage);

            // The 'findOrCreate' calls lockAsKey on builder and then destroys the returned
            // PaintParamsKey. This serves to reset the builder.
            UniquePaintParamsID paintID = keyContext.dict()->findOrCreate(&builder);

            processCombination(paintID, drawTypes, withPrimitiveBlender, coverage, renderPassDesc);
        }
    }
}

void PaintOptionsPriv::addColorFilter(sk_sp<PrecompileColorFilter> cf) {
    fPaintOptions->addColorFilter(std::move(cf));
}

} // namespace skgpu::graphite
