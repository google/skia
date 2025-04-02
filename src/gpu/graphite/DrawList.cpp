/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawList.h"

#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/Shape.h"

namespace skgpu::graphite {

const Transform& DrawList::deduplicateTransform(const Transform& localToDevice) {
    // TODO: This is a pretty simple deduplication strategy and doesn't take advantage of the stack
    // knowledge that Device has.
    if (fTransforms.empty() || fTransforms.back() != localToDevice) {
        fTransforms.push_back(localToDevice);
    }
    return fTransforms.back();
}

void DrawList::recordDraw(const Renderer* renderer,
                          const Transform& localToDevice,
                          const Geometry& geometry,
                          const Clip& clip,
                          DrawOrder ordering,
                          const PaintParams* paint,
                          const StrokeStyle* stroke) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    SkASSERT(!(renderer->depthStencilFlags() & DepthStencilFlags::kStencil) ||
             ordering.stencilIndex() != DrawOrder::kUnassigned);

    // TODO: Add validation that the renderer's expected shape type and stroke params match provided

    fDraws.emplace_back(renderer, this->deduplicateTransform(localToDevice),
                        geometry, clip, ordering, paint, stroke);

    // Accumulate renderer information for each draw added to this list
    fRenderStepCount += renderer->numRenderSteps();
    fRequiresMSAA |= renderer->requiresMSAA();
    fDepthStencilFlags |= renderer->depthStencilFlags();
    if (paint && paint->dstReadRequired()) {
        // For paints that read from the dst, update the bounds. It may later be determined that the
        // DstReadStrategy does not require them, but they are inexpensive to track.
        fDstReadBounds.join(clip.drawBounds());
    }

#if defined(SK_DEBUG)
    if (geometry.isCoverageMaskShape()) {
        fCoverageMaskShapeDrawCount++;
    }
#endif
}
} // namespace skgpu::graphite
