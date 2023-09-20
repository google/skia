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
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileShader> PrecompileShader::makeWithLocalMatrix() {
    if (this->priv().isALocalMatrixShader()) {
        // SkShader::makeWithLocalMatrix collapses chains of localMatrix shaders so we need to
        // follow suit here
        return sk_ref_sp(this);
    }

    return PrecompileShaders::LocalMatrix(sk_ref_sp(this));
}

sk_sp<PrecompileShader> PrecompileShader::makeWithColorFilter(sk_sp<PrecompileColorFilter> cf) {
    if (!cf) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::ColorFilter(sk_ref_sp(this), std::move(cf));
}

//--------------------------------------------------------------------------------------------------
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
    bool bmBased = false;
    int numBlendCombos = 0;
    for (auto b: fBlenderOptions) {
        if (b->asBlendMode().has_value()) {
            bmBased = true;
        } else {
            numBlendCombos += b->numChildCombinations();
        }
    }

    if (bmBased || !numBlendCombos) {
        // If numBlendCombos is zero we will fallback to kSrcOver blending
        ++numBlendCombos;
    }

    return numBlendCombos;
}

int PaintOptions::numCombinations() const {
    // TODO: we need to handle ImageFilters separately
    return this->numShaderCombinations() *
           this->numMaskFilterCombinations() *
           this->numColorFilterCombinations() *
           this->numBlendModeCombinations();
}

DstReadRequirement get_dst_read_req(const Caps* caps,
                                    Coverage coverage,
                                    SkSpan<const sk_sp<PrecompileBlender>> options,
                                    int desiredOption) {
    for (const sk_sp<PrecompileBlender>& option : options) {
        if (desiredOption < option->numCombinations()) {
            return GetDstReadRequirement(caps, option->asBlendMode(), coverage);
        }
        desiredOption -= option->numCombinations();
    }
    return GetDstReadRequirement(caps, SkBlendMode::kSrcOver, coverage);
}

class PaintOption {
public:
    PaintOption(bool opaquePaintColor,
                const std::pair<sk_sp<PrecompileShader>, int>& shader,
                const std::pair<sk_sp<PrecompileColorFilter>, int>& colorFilter,
                bool hasPrimitiveBlender,
                bool dither)
        : fOpaquePaintColor(opaquePaintColor)
        , fShader(shader)
        , fColorFilter(colorFilter)
        , fHasPrimitiveBlender(hasPrimitiveBlender)
        , fDither(dither) {
    }

    void toKey(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;

private:
    void addPaintColorToKey(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handlePrimitiveColor(const KeyContext&,
                              PaintParamsKeyBuilder*,
                              PipelineDataGatherer*) const;
    void handlePaintAlpha(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleColorFilter(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleDithering(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;

    bool shouldDither(SkColorType dstCT) const;

    bool fOpaquePaintColor;
    std::pair<sk_sp<PrecompileShader>, int> fShader;
    std::pair<sk_sp<PrecompileColorFilter>, int> fColorFilter;
    bool fHasPrimitiveBlender;
    bool fDither;
};


void PaintOption::addPaintColorToKey(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer) const {
    if (fShader.first) {
        fShader.first->priv().addToKey(keyContext, fShader.second, builder);
    } else {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
        builder->endBlock();
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
                  PrimitiveColorBlock::BeginBlock(keyContext, keyBuilder, gatherer);
                  keyBuilder->endBlock();
              });
    } else {
        this->addPaintColorToKey(keyContext, keyBuilder, gatherer);
    }
}

void PaintOption::handlePaintAlpha(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* keyBuilder,
                                   PipelineDataGatherer* gatherer) const {
    if (!fOpaquePaintColor) {
        Blend(keyContext, keyBuilder, gatherer,
              /* addBlendToKey= */ [&] () -> void {
                  auto coeffs = skgpu::GetPorterDuffBlendConstants(SkBlendMode::kSrcIn);
                  SkASSERT(!coeffs.empty());
                  CoeffBlenderBlock::BeginBlock(keyContext, keyBuilder, gatherer, coeffs);
                  keyBuilder->endBlock();
              },
              /* addSrcToKey= */ [&]() -> void {
                  this->handlePrimitiveColor(keyContext, keyBuilder, gatherer);
              },
              /* addDstToKey= */ [&]() -> void {
                  SolidColorShaderBlock::BeginBlock(keyContext, keyBuilder, gatherer,
                                                    {0, 0, 0, 0.5f});
                  keyBuilder->endBlock();
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
                    fColorFilter.first->priv().addToKey(keyContext, fColorFilter.second, builder);
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
    return fShader.first && !fShader.first->isConstant();
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
                    DitherShaderBlock::DitherData data(skgpu::DitherRangeForConfig(ct));

                    DitherShaderBlock::BeginBlock(keyContext, builder, gatherer, &data);
                    builder->endBlock();
                });
    } else
#endif
    {
        this->handleColorFilter(keyContext, builder, gatherer);
    }
}

void PaintOption::toKey(const KeyContext& keyContext,
                        PaintParamsKeyBuilder* keyBuilder,
                        PipelineDataGatherer* gatherer) const {
    this->handleDithering(keyContext, keyBuilder, gatherer);
}

void PaintOptions::createKey(const KeyContext& keyContext,
                             int desiredCombination,
                             PaintParamsKeyBuilder* keyBuilder,
                             bool addPrimitiveBlender,
                             Coverage coverage) const {
    SkDEBUGCODE(keyBuilder->checkReset();)
    SkASSERT(desiredCombination < this->numCombinations());

    const int numBlendModeCombos = this->numBlendModeCombinations();
    const int numColorFilterCombinations = this->numColorFilterCombinations();
    const int numMaskFilterCombinations = this->numMaskFilterCombinations();

    const int desiredBlendCombination = desiredCombination % numBlendModeCombos;
    int remainingCombinations = desiredCombination / numBlendModeCombos;

    const int desiredColorFilterCombination = remainingCombinations % numColorFilterCombinations;
    remainingCombinations /= numColorFilterCombinations;

    const int desiredMaskFilterCombination = remainingCombinations % numMaskFilterCombinations;
    remainingCombinations /= numMaskFilterCombinations;

    const int desiredShaderCombination = remainingCombinations;
    SkASSERT(desiredShaderCombination < this->numShaderCombinations());

    // TODO: eliminate this block for the Paint's color when it isn't needed
    SolidColorShaderBlock::BeginBlock(keyContext, keyBuilder, /* gatherer= */ nullptr,
                                      {1, 0, 0, 1});
    keyBuilder->endBlock();

    DstReadRequirement dstReadReq = get_dst_read_req(
            keyContext.caps(), coverage, fBlenderOptions, desiredBlendCombination);
    bool needsDstSample = dstReadReq == DstReadRequirement::kTextureCopy ||
                          dstReadReq == DstReadRequirement::kTextureSample;
    if (needsDstSample) {
        DstReadSampleBlock::BeginBlock(keyContext,
                                       keyBuilder,
                                       /* gatherer= */ nullptr,
                                       /* dstTexture= */ nullptr,
                                       /* dstOffset= */ {0, 0});
        keyBuilder->endBlock();

    } else if (dstReadReq == DstReadRequirement::kFramebufferFetch) {
        DstReadFetchBlock::BeginBlock(keyContext, keyBuilder, /* gatherer= */ nullptr);
        keyBuilder->endBlock();
    }

    // TODO: this probably needs to be passed in just like addPrimitiveBlender
    const bool kOpaquePaintColor = true;

    PaintOption option(kOpaquePaintColor,
                       PrecompileBase::SelectOption(fShaderOptions, desiredShaderCombination),
                       PrecompileBase::SelectOption(fColorFilterOptions,
                                                    desiredColorFilterCombination),
                       addPrimitiveBlender,
                       fDither);

    option.toKey(keyContext, keyBuilder, /* gatherer= */ nullptr);

    PrecompileBase::AddToKey(keyContext, keyBuilder, fMaskFilterOptions,
                             desiredMaskFilterCombination);

    auto [blender, _] = PrecompileBase::SelectOption(fBlenderOptions, desiredBlendCombination);

    std::optional<SkBlendMode> finalBlendMode = blender ? blender->asBlendMode()
                                                        : SkBlendMode::kSrcOver;
    if (dstReadReq != DstReadRequirement::kNone) {
        BlendShaderBlock::BeginBlock(keyContext, keyBuilder, /* gatherer= */ nullptr);
        // src -- prior output
        PriorOutputBlock::BeginBlock(keyContext, keyBuilder, /* gatherer= */ nullptr);
        keyBuilder->endBlock();
        // dst -- surface color
        DstColorBlock::BeginBlock(keyContext, keyBuilder, /* gatherer= */ nullptr);
        keyBuilder->endBlock();
        // blender -- shader based blending
        PrecompileBase::AddToKey(keyContext, keyBuilder, fBlenderOptions, desiredBlendCombination);
        keyBuilder->endBlock();  // BlendShaderBlock

        finalBlendMode = SkBlendMode::kSrc;
    }

    SkASSERT(finalBlendMode);
    BuiltInCodeSnippetID fixedFuncBlendModeID = static_cast<BuiltInCodeSnippetID>(
            kFixedFunctionBlendModeIDOffset + static_cast<int>(*finalBlendMode));
    keyBuilder->beginBlock(fixedFuncBlendModeID);
    keyBuilder->endBlock();
}

void PaintOptions::buildCombinations(
        const KeyContext& keyContext,
        bool addPrimitiveBlender,
        Coverage coverage,
        const std::function<void(UniquePaintParamsID)>& processCombination) const {

    PaintParamsKeyBuilder builder(keyContext.dict());

    int numCombinations = this->numCombinations();
    for (int i = 0; i < numCombinations; ++i) {
        this->createKey(keyContext, i, &builder, addPrimitiveBlender, coverage);

        // The 'findOrCreate' calls lockAsKey on builder and then destroys the returned
        // PaintParamsKey. This serves to reset the builder.
        UniquePaintParamsID paintID = keyContext.dict()->findOrCreate(&builder);

        processCombination(paintID);
    }
}

} // namespace skgpu::graphite
