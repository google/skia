/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/PaintParams.h"

#include "include/core/SkShader.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

PaintParams::PaintParams(const SkColor4f& color,
                         sk_sp<SkBlender> blender,
                         sk_sp<SkShader> shader)
        : fColor(color)
        , fBlender(std::move(blender))
        , fShader(std::move(shader)) {}

PaintParams::PaintParams(const SkPaint& paint)
        : fColor(paint.getColor4f())
        , fBlender(paint.refBlender())
        , fShader(paint.refShader()) {}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

std::optional<SkBlendMode> PaintParams::asBlendMode() const {
    return fBlender ? as_BB(fBlender)->asBlendMode()
                    : SkBlendMode::kSrcOver;
}

sk_sp<SkBlender> PaintParams::refBlender() const { return fBlender; }

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

void PaintParams::toKey(const SkKeyContext& keyContext,
                        SkPaintParamsKeyBuilder* builder,
                        SkPipelineDataGatherer* gatherer) const {

    if (fShader) {
        as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    } else {
        SolidColorShaderBlock::AddToKey(keyContext, builder, gatherer, fColor.premul());
    }

    if (fBlender) {
        as_BB(fBlender)->addToKey(keyContext, builder, gatherer);
    } else {
        BlendModeBlock::AddToKey(keyContext, builder, gatherer, SkBlendMode::kSrcOver);
    }

    SkASSERT(builder->sizeInBytes() > 0);
}

} // namespace skgpu::graphite
