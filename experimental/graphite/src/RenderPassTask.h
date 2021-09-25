/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RenderPassTask_DEFINED
#define skgpu_RenderPassTask_DEFINED

#include "experimental/graphite/src/Task.h"

namespace skgpu {

class DrawList;

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
    // TODO: RenderPassTask also needs to know the surface the commads are sent to (might not need
    // to be explicit if there's a render pass list that collects {draw lists + surface}).
    static sk_sp<RenderPassTask> Make(sk_sp<Task> prior, std::unique_ptr<DrawList> cmds);

    ~RenderPassTask() override;

private:
    RenderPassTask(std::unique_ptr<DrawList> cmds);

    // TODO: Seems very likely that the RenderPassTask will store an optimized/immutable
    // representation derived from a DrawList and not directly a DrawList.
    std::unique_ptr<DrawList> fCmds;
};

} // namespace skgpu

#endif // skgpu_RenderPassTask_DEFINED
