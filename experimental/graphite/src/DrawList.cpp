/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawList.h"

#include "experimental/graphite/src/Renderer.h"
#include "src/gpu/BufferWriter.h"

namespace skgpu {

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

    const Renderer& renderer = Renderer::StencilAndFillPath(shape.fillType());
    fDraws.push_back({renderer, this->deduplicateTransform(localToDevice),
                      shape, clip, ordering, paint, nullptr});
    fRenderStepCount += renderer.numRenderSteps();
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
