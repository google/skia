/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParams.h"

#include "include/core/SkShader.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkUniform.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

PaintParams::PaintParams(const SkColor4f& color,
                         sk_sp<SkBlender> blender,
                         sk_sp<SkShader> shader,
                         sk_sp<SkColorFilter> colorFilter)
        : fColor(color)
        , fBlender(std::move(blender))
        , fShader(std::move(shader))
        , fColorFilter(std::move(colorFilter)) {}

PaintParams::PaintParams(const SkPaint& paint)
        : fColor(paint.getColor4f())
        , fBlender(paint.refBlender())
        , fShader(paint.refShader())
        , fColorFilter(paint.refColorFilter()) {}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

std::optional<SkBlendMode> PaintParams::asBlendMode() const {
    return fBlender ? as_BB(fBlender)->asBlendMode()
                    : SkBlendMode::kSrcOver;
}

sk_sp<SkBlender> PaintParams::refBlender() const { return fBlender; }

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

sk_sp<SkColorFilter> PaintParams::refColorFilter() const { return fColorFilter; }

void PaintParams::toKey(const SkKeyContext& keyContext,
                        SkPaintParamsKeyBuilder* builder,
                        SkPipelineDataGatherer* gatherer) const {

    if (fShader) {
        as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    } else {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, fColor.premul());
        builder->endBlock();
    }

    if (fColorFilter) {
        as_CFB(fColorFilter)->addToKey(keyContext, builder, gatherer);
    }

    if (fBlender) {
        as_BB(fBlender)->addToKey(keyContext, builder, gatherer);
    } else {
        BlendModeBlock::BeginBlock(keyContext, builder, gatherer, SkBlendMode::kSrcOver);
        builder->endBlock();
    }

    if (gatherer) {
        if (gatherer->needsLocalCoords()) {
#ifdef SK_DEBUG
            static constexpr SkUniform kDev2LocalUniform[] = {{ "dev2Local", SkSLType::kFloat4x4 }};
            UniformExpectationsValidator uev(gatherer,
                                             SkSpan<const SkUniform>(kDev2LocalUniform, 1));
#endif

            gatherer->write(keyContext.dev2Local());
        }
    }

    SkASSERT(builder->sizeInBytes() > 0);
}

} // namespace skgpu::graphite
