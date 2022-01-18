/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/PaintParams.h"

#include "include/core/SkShader.h"
#include "include/private/SkPaintParamsKey.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyHelpers.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu {

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

skstd::optional<SkBlendMode> PaintParams::asBlendMode() const {
    return fBlender ? as_BB(fBlender)->asBlendMode()
                    : SkBlendMode::kSrcOver;
}

sk_sp<SkBlender> PaintParams::refBlender() const { return fBlender; }

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

void PaintParams::toKey(SkShaderCodeDictionary* dict,
                        SkBackend backend,
                        SkPaintParamsKey* key) const {

    if (fShader) {
        as_SB(fShader)->addToKey(dict, backend, key);
    } else {
        SolidColorShaderBlock::AddToKey(key);
    }

    if (fBlender) {
        as_BB(fBlender)->addToKey(dict, backend, key);
    } else {
        BlendModeBlock::AddToKey(key, SkBlendMode::kSrcOver);
    }

    SkASSERT(key->sizeInBytes() > 0);
}

} // namespace skgpu
