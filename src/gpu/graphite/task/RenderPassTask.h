/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_RenderPassTask_DEFINED
#define skgpu_graphite_task_RenderPassTask_DEFINED

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/task/Task.h"

namespace skgpu::graphite {

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
    using DrawPassList = skia_private::STArray<1, std::unique_ptr<DrawPass>>;

    static sk_sp<RenderPassTask> Make(DrawPassList,
                                      const RenderPassDesc&,
                                      sk_sp<TextureProxy> target);

    ~RenderPassTask() override;

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            const RuntimeEffectDictionary*) override;

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    RenderPassTask(DrawPassList, const RenderPassDesc&, sk_sp<TextureProxy> target);

    DrawPassList fDrawPasses;
    RenderPassDesc fRenderPassDesc;
    sk_sp<TextureProxy> fTarget;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_RenderPassTask_DEFINED
