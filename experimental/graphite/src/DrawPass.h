/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawPass_DEFINED
#define skgpu_DrawPass_DEFINED

#include "include/core/SkRect.h"

#include <memory>

namespace skgpu {

class DrawList;
class DrawContext;

/**
 * DrawPass is analogous to a subpass, storing the drawing operations in the order they are stored
 * in the eventual command buffer, as well as the surface proxy the operations are intended for.
 * DrawPasses are grouped into a RenderPassTask for execution within a single render pass if the
 * subpasses are compatible with each other.
 *
 * Unlike DrawList, DrawPasses are immutable and represent as closely as possible what will be
 * stored in the command buffer while being flexible as to how the pass is incorporated. Depending
 * on the backend, it may even be able to write accumulated vertex and uniform data directly to
 * mapped GPU memory, although that is the extent of the CPU->GPU work they perform before they are
 * executed by a RenderPassTask.
 */
class DrawPass {
public:
    // TODO: Replace SDC with the SDC's surface proxy view
    static std::unique_ptr<DrawPass> Make(std::unique_ptr<DrawList>, DrawContext*);

    // Defined relative to the top-left corner of the surface the DrawPass renders to, and is
    // contained within its dimensions.
    const SkIRect& bounds() const { return fBounds; }

    bool requiresDstTexture() const { return false; }
    bool requiresDMSAA() const { return true; }

    size_t vertexBufferSize() const { return 0; }
    size_t uniformBufferSize() const { return 0; }

    // TODO: Real return types
    void samplers() const {}
    void programs() const {}

private:
    DrawPass() {}

    SkIRect fBounds;
    // TODO: actually implement this. Will own the results of sorting/culling/merging a DrawList,
    // however that is actually specified, and will have a surface proxy view.
};

} // namespace skgpu

#endif // skgpu_DrawPass_DEFINED
