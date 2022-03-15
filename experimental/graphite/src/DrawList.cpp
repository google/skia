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

void DrawList::recordDraw(const Renderer& renderer,
                          const Transform& localToDevice,
                          const Shape& shape,
                          const Clip& clip,
                          DrawOrder ordering,
                          const PaintParams* paint,
                          const StrokeParams* stroke) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!shape.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    SkASSERT(!(renderer.depthStencilFlags() & DepthStencilFlags::kStencil) ||
             ordering.stencilIndex() != DrawOrder::kUnassigned);

    // TODO: Add validation that the renderer's expected shape type and stroke params match provided

    fDraws.push_back({renderer, this->deduplicateTransform(localToDevice),
                      shape, clip, ordering, paint, stroke});
    fRenderStepCount += renderer.numRenderSteps();
}

} // namespace skgpu
