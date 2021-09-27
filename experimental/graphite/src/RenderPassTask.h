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
    // TODO: 'prior' isn't actually used yet but is here to represent the dependency between a
    // series of RenderPassTasks.
    static sk_sp<RenderPassTask> Make(sk_sp<Task> prior,
                                      std::vector<std::unique_ptr<DrawPass>> passes);

    ~RenderPassTask() override;

    // TBD: Expose the surfaces that will need to be attached within the renderpass?

private:
    RenderPassTask(std::vector<std::unique_ptr<DrawPass>> passes);

    std::vector<std::unique_ptr<DrawPass>> fDrawPasses;
};

} // namespace skgpu

#endif // skgpu_RenderPassTask_DEFINED
