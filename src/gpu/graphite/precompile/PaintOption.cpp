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
#include "src/core/SkColorSpacePriv.h"
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
    // Root Node 0 is the source color, which is the output of all effects post dithering
    this->handleDithering(keyContext, keyBuilder, gatherer);

    // Root Node 1 is the final blender
    std::optional<SkBlendMode> finalBlendMode =
            this->finalBlender() ? this->finalBlender()->priv().asBlendMode()
                                 : SkBlendMode::kSrcOver;
    if (finalBlendMode) {
        if (!fDstReadRequired) {
            AddFixedBlendMode(keyContext, keyBuilder, gatherer, *finalBlendMode);
        } else {
            AddBlendMode(keyContext, keyBuilder, gatherer, *finalBlendMode);
        }
    } else {
        SkASSERT(this->finalBlender());
        fFinalBlender.first->priv().addToKey(keyContext, keyBuilder, gatherer,
                                             fFinalBlender.second);
    }

    // Optional Root Node 2 is the clip
    // TODO(b/372221436): Also include analytic clippiing in this node.
    if (fClipShader.first) {
        fClipShader.first->priv().addToKey(keyContext, keyBuilder, gatherer,
                                           fClipShader.second);
    }
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
                  AddPrimitiveColor(keyContext, keyBuilder, gatherer, sk_srgb_singleton());
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
                  AddFixedBlendMode(keyContext, keyBuilder, gatherer, SkBlendMode::kSrcIn);
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

} // namespace skgpu::graphite
