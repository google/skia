/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RenderPassTask_DEFINED
#define skgpu_graphite_RenderPassTask_DEFINED

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Task.h"

#include <vector>

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
    static sk_sp<RenderPassTask> Make(std::vector<std::unique_ptr<DrawPass>> passes,
                                      const RenderPassDesc&,
                                      sk_sp<TextureProxy> target);

    ~RenderPassTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    RenderPassTask(std::vector<std::unique_ptr<DrawPass>> passes,
                   const RenderPassDesc&,
                   sk_sp<TextureProxy> target);

    std::vector<std::unique_ptr<DrawPass>> fDrawPasses;
    RenderPassDesc fRenderPassDesc;
    sk_sp<TextureProxy> fTarget;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_RenderPassTask_DEFINED
