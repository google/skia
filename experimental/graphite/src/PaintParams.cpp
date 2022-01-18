/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/PaintParams.h"

#include "include/core/SkShader.h"
#include "include/private/SkPaintParamsKey.h"
#include "src/core/SkKeyHelpers.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu {

PaintParams::PaintParams(const SkColor4f& color,
                         SkBlendMode blendMode,
                         sk_sp<SkShader> shader)
        : fColor(color)
        , fBlendMode(blendMode)
        , fShader(std::move(shader)) {}

PaintParams::PaintParams(const SkPaint& paint)
        : fColor(paint.getColor4f())
        , fBlendMode(paint.getBlendMode_or(SkBlendMode::kSrcOver))
        , fShader(paint.refShader()) {}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

void PaintParams::toKey(SkShaderCodeDictionary* dict,
                        SkBackend backend,
                        SkPaintParamsKey* key) const {

    if (fShader) {
        as_SB(fShader)->addToKey(dict, backend, key);
    } else {
        SolidColorShaderBlock::AddToKey(key);
    }

    // TODO: add blender support to PaintParams
    {
        BlendModeBlock::AddToKey(key, fBlendMode);
    }

    SkASSERT(key->sizeInBytes() > 0);
}

} // namespace skgpu
