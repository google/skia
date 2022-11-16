/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParams.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkShader.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

PaintParams::PaintParams(const SkColor4f& color,
                         sk_sp<SkBlender> finalBlender,
                         sk_sp<SkShader> shader,
                         sk_sp<SkColorFilter> colorFilter,
                         sk_sp<SkBlender> primitiveBlender,
                         bool skipColorXform)
        : fColor(color)
        , fFinalBlender(std::move(finalBlender))
        , fShader(std::move(shader))
        , fColorFilter(std::move(colorFilter))
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fSkipColorXform(skipColorXform) {}

PaintParams::PaintParams(const SkPaint& paint,
                         sk_sp<SkBlender> primitiveBlender,
                         bool skipColorXform)
        : fColor(paint.getColor4f())
        , fFinalBlender(paint.refBlender())
        , fShader(paint.refShader())
        , fColorFilter(paint.refColorFilter())
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fSkipColorXform(skipColorXform) {}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

std::optional<SkBlendMode> PaintParams::asFinalBlendMode() const {
    return fFinalBlender ? as_BB(fFinalBlender)->asBlendMode()
                         : SkBlendMode::kSrcOver;
}

sk_sp<SkBlender> PaintParams::refFinalBlender() const { return fFinalBlender; }

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

sk_sp<SkColorFilter> PaintParams::refColorFilter() const { return fColorFilter; }

sk_sp<SkBlender> PaintParams::refPrimitiveBlender() const { return fPrimitiveBlender; }

void PaintParams::toKey(const KeyContext& keyContext,
                        PaintParamsKeyBuilder* builder,
                        PipelineDataGatherer* gatherer) const {

    // Begin the key with a solid color shader block to set the initial color to the paint's color.
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, fColor.makeOpaque().premul());
    builder->endBlock();

    if (fShader) {
        as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    }

    if (fPrimitiveBlender) {
        as_BB(fPrimitiveBlender)->addToKey(keyContext, builder, gatherer,
                                           /*primitiveColorBlender=*/true);
    }

    // Apply the paint's alpha value.
    auto alphaColorFilter = SkColorFilters::Blend({0, 0, 0, fColor[3]},
                                                  /*colorSpace*/nullptr,
                                                  SkBlendMode::kDstIn);
    if (alphaColorFilter) {
        as_CFB(alphaColorFilter)->addToKey(keyContext, builder, gatherer);
    }

    if (fColorFilter) {
        as_CFB(fColorFilter)->addToKey(keyContext, builder, gatherer);
    }

    if (fFinalBlender) {
        as_BB(fFinalBlender)->addToKey(keyContext, builder, gatherer,
                                       /*primitiveColorBlender=*/false);
    } else {
        BlendModeBlock::BeginBlock(keyContext, builder, gatherer, SkBlendMode::kSrcOver);
        builder->endBlock();
    }

    SkASSERT(builder->sizeInBytes() > 0);
}

} // namespace skgpu::graphite
