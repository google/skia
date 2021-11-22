/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawList.h"

#include "experimental/graphite/src/Renderer.h"
#include "include/core/SkShader.h"
#include "src/gpu/BufferWriter.h"

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

const Transform& DrawList::deduplicateTransform(const Transform& localToDevice) {
    // TODO: This is a pretty simple deduplication strategy and doesn't take advantage of the stack
    // knowledge that Device has.
    if (fTransforms.empty() || fTransforms.back() != localToDevice) {
        fTransforms.push_back(localToDevice);
    }
    return fTransforms.back();
}

void DrawList::stencilAndFillPath(const Transform& localToDevice,
                                  const Shape& shape,
                                  const Clip& clip,
                                  DrawOrder ordering,
                                  const PaintParams* paint) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!shape.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    fDraws.push_back({Renderer::StencilAndFillPath(),
                      this->deduplicateTransform(localToDevice),
                      shape, clip, ordering, paint, nullptr});
    fRenderStepCount += Renderer::StencilAndFillPath().numRenderSteps();
}

void DrawList::fillConvexPath(const Transform& localToDevice,
                              const Shape& shape,
                              const Clip& clip,
                              DrawOrder ordering,
                              const PaintParams* paint) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!shape.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    // TODO actually record this, but for now just drop the draw since the Renderer
    // isn't implemented yet
    // fDraws.push_back({Renderer::FillConvexPath(),
    //                   this->deduplicateTransform(localToDevice),
    //                   shape, clip, ordering, paint, nullptr});
    // fRenderStepCount += Renderer::FillConvexPath().numRenderSteps();
}

void DrawList::strokePath(const Transform& localToDevice,
                          const Shape& shape,
                          const StrokeParams& stroke,
                          const Clip& clip,
                          DrawOrder ordering,
                          const PaintParams* paint) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!shape.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    // TODO actually record this, but for now just drop the draw since the Renderer
    // isn't implemented yet
    // fDraws.push_back({Renderer::StrokePath(),
    //                   this->deduplicateTransform(localToDevice),
    //                   shape, clip, ordering, paint, stroke});
    // fRenderStepCount += Renderer::StrokePath().numRenderSteps();
}

} // namespace skgpu
