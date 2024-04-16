/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/DitherUtils.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/FactoryFunctionsPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintOptionsPriv.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileShader> PrecompileShader::makeWithLocalMatrix() {
    if (this->priv().isALocalMatrixShader()) {
        // SkShader::makeWithLocalMatrix collapses chains of localMatrix shaders so we need to
        // follow suit here
        return sk_ref_sp(this);
    }

    return PrecompileShaders::LocalMatrix({ sk_ref_sp(this) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithColorFilter(sk_sp<PrecompileColorFilter> cf) {
    if (!cf) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::ColorFilter({ sk_ref_sp(this) }, { std::move(cf) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithWorkingColorSpace(sk_sp<SkColorSpace> cs) {
    if (!cs) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::WorkingColorSpace({ sk_ref_sp(this) }, { std::move(cs) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithCTM() {
    return PrecompileShadersPriv::CTM({ sk_ref_sp(this) });
}

sk_sp<PrecompileColorFilter> PrecompileColorFilter::makeComposed(
        sk_sp<PrecompileColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return PrecompileColorFilters::Compose({ sk_ref_sp(this) }, { std::move(inner) });
}

//--------------------------------------------------------------------------------------------------
void PaintOptions::setClipShaders(SkSpan<const sk_sp<PrecompileShader>> clipShaders) {
    // In the normal API this modification happens in SkDevice::clipShader()
    fClipShaderOptions.reserve(2 * clipShaders.size());
    for (const sk_sp<PrecompileShader>& cs : clipShaders) {
        // All clipShaders get wrapped in a CTMShader ...
        sk_sp<PrecompileShader> withCTM = cs ? cs->makeWithCTM() : nullptr;
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

int PaintOptions::numMaskFilterCombinations() const {
    int numMaskFilterCombinations = 0;
    for (const sk_sp<PrecompileMaskFilter>& mf : fMaskFilterOptions) {
        numMaskFilterCombinations += mf->numCombinations();
    }

    // If no mask filter options are specified we will use the geometry's coverage
    return numMaskFilterCombinations ? numMaskFilterCombinations : 1;
}

int PaintOptions::numColorFilterCombinations() const {
    int numColorFilterCombinations = 0;
    for (const sk_sp<PrecompileColorFilter>& cf : fColorFilterOptions) {
        numColorFilterCombinations += cf->numCombinations();
    }

    // If no color filter options are specified we will use the unmodified result color
    return numColorFilterCombinations ? numColorFilterCombinations : 1;
}

int PaintOptions::numBlendModeCombinations() const {
    int numBlendCombos = fBlendModeOptions.size();
    for (const sk_sp<PrecompileBlender>& b: fBlenderOptions) {
        SkASSERT(!b->asBlendMode().has_value());
        numBlendCombos += b->numChildCombinations();
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
            numClipShaderCombos += cs->numChildCombinations();
        } else {
            ++numClipShaderCombos;
        }
    }

    // If no clipShader options are specified we will just have the unclipped options
    return numClipShaderCombos ? numClipShaderCombos : 1;
}


int PaintOptions::numCombinations() const {
    // TODO: we need to handle ImageFilters separately
    return this->numShaderCombinations() *
           this->numMaskFilterCombinations() *
           this->numColorFilterCombinations() *
           this->numBlendModeCombinations() *
           this->numClipShaderCombinations();
}

DstReadRequirement get_dst_read_req(const Caps* caps,
                                    Coverage coverage,
                                    PrecompileBlender* blender) {
    if (blender) {
        return GetDstReadRequirement(caps, blender->asBlendMode(), coverage);
    }
    return GetDstReadRequirement(caps, SkBlendMode::kSrcOver, coverage);
}

class PaintOption {
public:
    PaintOption(bool opaquePaintColor,
                const std::pair<sk_sp<PrecompileBlender>, int>& finalBlender,
                const std::pair<sk_sp<PrecompileShader>, int>& shader,
                const std::pair<sk_sp<PrecompileColorFilter>, int>& colorFilter,
                bool hasPrimitiveBlender,
                const std::pair<sk_sp<PrecompileShader>, int>& clipShader,
                DstReadRequirement dstReadReq,
                bool dither)
        : fOpaquePaintColor(opaquePaintColor)
        , fFinalBlender(finalBlender)
        , fShader(shader)
        , fColorFilter(colorFilter)
        , fHasPrimitiveBlender(hasPrimitiveBlender)
        , fClipShader(clipShader)
        , fDstReadReq(dstReadReq)
        , fDither(dither) {
    }

    const PrecompileBlender* finalBlender() const { return fFinalBlender.first.get(); }

    void toKey(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;

private:
    void addPaintColorToKey(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handlePrimitiveColor(const KeyContext&,
                              PaintParamsKeyBuilder*,
                              PipelineDataGatherer*) const;
    void handlePaintAlpha(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleColorFilter(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleDithering(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleDstRead(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;

    bool shouldDither(SkColorType dstCT) const;

    bool fOpaquePaintColor;
    std::pair<sk_sp<PrecompileBlender>, int> fFinalBlender;
    std::pair<sk_sp<PrecompileShader>, int> fShader;
    std::pair<sk_sp<PrecompileColorFilter>, int> fColorFilter;
    bool fHasPrimitiveBlender;
    std::pair<sk_sp<PrecompileShader>, int> fClipShader;
    DstReadRequirement fDstReadReq;
    bool fDither;
};


void PaintOption::addPaintColorToKey(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer) const {
    if (fShader.first) {
        fShader.first->priv().addToKey(keyContext, builder, gatherer, fShader.second);
    } else {
        RGBPaintColorBlock::AddBlock(keyContext, builder, gatherer);
    }
}

void PaintOption::handlePrimitiveColor(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* keyBuilder,
                                       PipelineDataGatherer* gatherer) const {
    if (fHasPrimitiveBlender) {
        Blend(keyContext, keyBuilder, gatherer,
              /* addBlendToKey= */ [&] () -> void {
                  // TODO: Support runtime blenders for primitive blending in the precompile API.
                  // In the meantime, assume for now that we're using kSrcOver here.
                  AddToKey(keyContext, keyBuilder, gatherer,
                           SkBlender::Mode(SkBlendMode::kSrcOver).get());
              },
              /* addSrcToKey= */ [&]() -> void {
                  this->addPaintColorToKey(keyContext, keyBuilder, gatherer);
              },
              /* addDstToKey= */ [&]() -> void {
                  keyBuilder->addBlock(BuiltInCodeSnippetID::kPrimitiveColor);
              });
    } else {
        this->addPaintColorToKey(keyContext, keyBuilder, gatherer);
    }
}

void PaintOption::handlePaintAlpha(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* keyBuilder,
                                   PipelineDataGatherer* gatherer) const {

    if (!fShader.first && !fHasPrimitiveBlender) {
        // If there is no shader and no primitive blending the input to the colorFilter stage
        // is just the premultiplied paint color.
        SolidColorShaderBlock::AddBlock(keyContext, keyBuilder, gatherer, SK_PMColor4fWHITE);
        return;
    }

    if (!fOpaquePaintColor) {
        Blend(keyContext, keyBuilder, gatherer,
              /* addBlendToKey= */ [&] () -> void {
                  AddKnownModeBlend(keyContext, keyBuilder, gatherer, SkBlendMode::kSrcIn);
              },
              /* addSrcToKey= */ [&]() -> void {
                  this->handlePrimitiveColor(keyContext, keyBuilder, gatherer);
              },
              /* addDstToKey= */ [&]() -> void {
                  AlphaOnlyPaintColorBlock::AddBlock(keyContext, keyBuilder, gatherer);
              });
    } else {
        this->handlePrimitiveColor(keyContext, keyBuilder, gatherer);
    }
}

void PaintOption::handleColorFilter(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
                                    PipelineDataGatherer* gatherer) const {
    if (fColorFilter.first) {
        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    this->handlePaintAlpha(keyContext, builder, gatherer);
                },
                /* addOuterToKey= */ [&]() -> void {
                    fColorFilter.first->priv().addToKey(keyContext, builder, gatherer,
                                                        fColorFilter.second);
                });
    } else {
        this->handlePaintAlpha(keyContext, builder, gatherer);
    }
}

// This should be kept in sync w/ SkPaintPriv::ShouldDither and PaintParams::should_dither
bool PaintOption::shouldDither(SkColorType dstCT) const {
    // The paint dither flag can veto.
    if (!fDither) {
        return false;
    }

    if (dstCT == kUnknown_SkColorType) {
        return false;
    }

    // We always dither 565 or 4444 when requested.
    if (dstCT == kRGB_565_SkColorType || dstCT == kARGB_4444_SkColorType) {
        return true;
    }

    // Otherwise, dither is only needed for non-const paints.
    return fShader.first && !fShader.first->isConstant(fShader.second);
}

void PaintOption::handleDithering(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer) const {

#ifndef SK_IGNORE_GPU_DITHER
    SkColorType ct = keyContext.dstColorInfo().colorType();
    if (this->shouldDither(ct)) {
        Compose(keyContext, builder, gatherer,
                /* addInnerToKey= */ [&]() -> void {
                    this->handleColorFilter(keyContext, builder, gatherer);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddDitherBlock(keyContext, builder, gatherer, ct);
                });
    } else
#endif
    {
        this->handleColorFilter(keyContext, builder, gatherer);
    }
}

void PaintOption::handleDstRead(const KeyContext& keyContext,
                                PaintParamsKeyBuilder* builder,
                                PipelineDataGatherer* gatherer) const {
    if (fDstReadReq != DstReadRequirement::kNone) {
        Blend(keyContext, builder, gatherer,
                /* addBlendToKey= */ [&] () -> void {
                    if (fFinalBlender.first) {
                        fFinalBlender.first->priv().addToKey(keyContext, builder, gatherer,
                                                             fFinalBlender.second);
                    } else {
                        AddKnownModeBlend(keyContext, builder, gatherer, SkBlendMode::kSrcOver);
                    }
                },
                /* addSrcToKey= */ [&]() -> void {
                    this->handleDithering(keyContext, builder, gatherer);
                },
                /* addDstToKey= */ [&]() -> void {
                    AddDstReadBlock(keyContext, builder, gatherer, fDstReadReq);
                });
    } else {
        this->handleDithering(keyContext, builder, gatherer);
    }
}

void PaintOption::toKey(const KeyContext& keyContext,
                        PaintParamsKeyBuilder* keyBuilder,
                        PipelineDataGatherer* gatherer) const {
    this->handleDstRead(keyContext, keyBuilder, gatherer);

    std::optional<SkBlendMode> finalBlendMode = this->finalBlender()
                                                        ? this->finalBlender()->asBlendMode()
                                                        : SkBlendMode::kSrcOver;
    if (fDstReadReq != DstReadRequirement::kNone) {
        // In this case the blend will have been handled by shader-based blending with the dstRead.
        finalBlendMode = SkBlendMode::kSrc;
    }

    if (fClipShader.first) {
        ClipShaderBlock::BeginBlock(keyContext, keyBuilder, gatherer);
            fClipShader.first->priv().addToKey(keyContext, keyBuilder, gatherer,
                                               fClipShader.second);
        keyBuilder->endBlock();
    }

    // Set the hardware blend mode.
    SkASSERT(finalBlendMode);
    BuiltInCodeSnippetID fixedFuncBlendModeID = static_cast<BuiltInCodeSnippetID>(
            kFixedFunctionBlendModeIDOffset + static_cast<int>(*finalBlendMode));

    keyBuilder->addBlock(fixedFuncBlendModeID);
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
    const int numBlendModeCombos = this->numBlendModeCombinations();
    const int numColorFilterCombinations = this->numColorFilterCombinations();
    const int numMaskFilterCombinations = this->numMaskFilterCombinations();

    const int desiredClipShaderCombination = desiredCombination % numClipShaderCombos;
    int remainingCombinations = desiredCombination / numClipShaderCombos;

    const int desiredBlendCombination = remainingCombinations % numBlendModeCombos;
    remainingCombinations /= numBlendModeCombos;

    const int desiredColorFilterCombination = remainingCombinations % numColorFilterCombinations;
    remainingCombinations /= numColorFilterCombinations;

    [[maybe_unused]] const int desiredMaskFilterCombination =
                                             remainingCombinations % numMaskFilterCombinations;
    remainingCombinations /= numMaskFilterCombinations;

    const int desiredShaderCombination = remainingCombinations;
    SkASSERT(desiredShaderCombination < this->numShaderCombinations());

    // TODO: this probably needs to be passed in just like addPrimitiveBlender
    const bool kOpaquePaintColor = true;

    auto clipShader = PrecompileBase::SelectOption(SkSpan(fClipShaderOptions),
                                                   desiredClipShaderCombination);

    std::pair<sk_sp<PrecompileBlender>, int> finalBlender;
    if (desiredBlendCombination < fBlendModeOptions.size()) {
        finalBlender = { PrecompileBlender::Mode(fBlendModeOptions[desiredBlendCombination]), 0 };
    } else {
        finalBlender = PrecompileBase::SelectOption(
                            SkSpan(fBlenderOptions),
                            desiredBlendCombination - fBlendModeOptions.size());
    }
    if (!finalBlender.first) {
        finalBlender = { PrecompileBlender::Mode(SkBlendMode::kSrcOver), 0 };
    }
    DstReadRequirement dstReadReq = get_dst_read_req(keyContext.caps(), coverage,
                                                     finalBlender.first.get());

    PaintOption option(kOpaquePaintColor,
                       finalBlender,
                       PrecompileBase::SelectOption(SkSpan(fShaderOptions),
                                                    desiredShaderCombination),
                       PrecompileBase::SelectOption(SkSpan(fColorFilterOptions),
                                                    desiredColorFilterCombination),
                       addPrimitiveBlender,
                       clipShader,
                       dstReadReq,
                       fDither);

    option.toKey(keyContext, keyBuilder, gatherer);
}

namespace {

void create_blur_pipelines(const KeyContext& keyContext,
                           PipelineDataGatherer* gatherer,
                           const PaintOptions::ProcessCombination& processCombination) {
    PaintOptions blurPaintOptions, imagePaintOptions;

    // For blurs we know we don't have alpha-only textures and don't need cubic filtering.
    sk_sp<PrecompileShader> imageShader = PrecompileShadersPriv::Image(
            PrecompileImageShaderFlags::kExcludeAlpha | PrecompileImageShaderFlags::kExcludeCubic);

    SkBlendMode blurBlendModes[] = { SkBlendMode::kSrc };
    blurPaintOptions.setShaders({ PrecompileShadersPriv::Blur(imageShader) });
    blurPaintOptions.setBlendModes(blurBlendModes);

    SkBlendMode imageBlendModes[] = { SkBlendMode::kSrc, SkBlendMode::kDstOut };
    imagePaintOptions.setShaders({ imageShader });
    imagePaintOptions.setBlendModes(imageBlendModes);

    blurPaintOptions.priv().buildCombinations(keyContext,
                                              gatherer,
                                              DrawTypeFlags::kSimpleShape,
                                              /* withPrimitiveBlender= */ false,
                                              Coverage::kSingleChannel,
                                              processCombination);
    imagePaintOptions.priv().buildCombinations(keyContext,
                                               gatherer,
                                               DrawTypeFlags::kSimpleShape,
                                               /* withPrimitiveBlender= */ false,
                                               Coverage::kSingleChannel,
                                               processCombination);
}

} // anonymous namespace

void PaintOptions::buildCombinations(
        const KeyContext& keyContext,
        PipelineDataGatherer* gatherer,
        DrawTypeFlags drawTypes,
        bool withPrimitiveBlender,
        Coverage coverage,
        const ProcessCombination& processCombination) const {

    PaintParamsKeyBuilder builder(keyContext.dict());

    if (fImageFilterOptions != PrecompileImageFilters::kNone) {
        PaintOptions tmp = *this;

        // When image filtering the original blend mode is taken over by the restore paint
        tmp.setImageFilters(PrecompileImageFilters::kNone);
        SkBlendMode newDrawBlendMode[] = { SkBlendMode::kSrcOver };
        tmp.setBlendModes(newDrawBlendMode);

        tmp.buildCombinations(keyContext, gatherer, drawTypes, withPrimitiveBlender, coverage,
                              processCombination);

        if (fImageFilterOptions & PrecompileImageFilters::kBlur) {
            create_blur_pipelines(keyContext, gatherer, processCombination);
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

            processCombination(paintID, drawTypes, withPrimitiveBlender, coverage);
        }
    }
}

} // namespace skgpu::graphite
