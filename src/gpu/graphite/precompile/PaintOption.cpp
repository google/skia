/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/precompile/PaintOption.h"

#include "include/core/SkBlender.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"

namespace skgpu::graphite {

void PaintOption::toKey(const KeyContext& keyContext,
                        PaintParamsKeyBuilder* keyBuilder,
                        PipelineDataGatherer* gatherer) const {
    this->handleDstRead(keyContext, keyBuilder, gatherer);

    std::optional<SkBlendMode> finalBlendMode = this->finalBlender()
                                                        ? this->finalBlender()->priv().asBlendMode()
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
                  PrimitiveColorBlock::AddBlock(keyContext, keyBuilder, gatherer);
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
    return fShader.first && !fShader.first->priv().isConstant(fShader.second);
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

} // namespace skgpu::graphite
