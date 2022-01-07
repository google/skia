/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/PaintParams.h"

#include "include/core/SkShader.h"

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

} // namespace skgpu
