/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RenderPassTask_DEFINED
#define skgpu_RenderPassTask_DEFINED

#include "experimental/graphite/src/Task.h"

#include <vector>

namespace skgpu {

class DrawPass;

/**
 * RenderPassTask handles preparing and recording DrawLists into a single render pass within a
 * command buffer. If the backend supports subpasses, and the DrawLists/surfaces are compatible, a
 * RenderPassTask can execute multiple DrawLists across different surfaces as subpasses nested
 * within a single render pass. If there is no such support, a RenderPassTask is one-to-one with a
 * "render pass" to specific surface.
 */
class RenderPassTask final : public Task {
public:
    static sk_sp<RenderPassTask> Make(std::vector<std::unique_ptr<DrawPass>> passes);

    ~RenderPassTask() override;

    void execute(CommandBuffer*) override {}

    // TBD: Expose the surfaces that will need to be attached within the renderpass?

    // TODO: for task execution, iterate the draw passes (can assume just 1 for sprint?) and
    // determine RenderPassDesc. Then start the render pass, then iterate passes again and
    // possibly(?) start each subpass, and call DrawPass::execute() on the command buffer provided
    // to the task. Then close the render pass and we should have pixels..

private:
    RenderPassTask(std::vector<std::unique_ptr<DrawPass>> passes);

    std::vector<std::unique_ptr<DrawPass>> fDrawPasses;
};

} // namespace skgpu

#endif // skgpu_RenderPassTask_DEFINED
