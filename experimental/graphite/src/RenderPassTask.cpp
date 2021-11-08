/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/RenderPassTask.h"

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/DrawPass.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"

namespace skgpu {

sk_sp<RenderPassTask> RenderPassTask::Make(std::vector<std::unique_ptr<DrawPass>> passes) {
    return sk_sp<RenderPassTask>(new RenderPassTask(std::move(passes)));
}

RenderPassTask::RenderPassTask(std::vector<std::unique_ptr<DrawPass>> passes)
        : fDrawPasses(std::move(passes)) {}

RenderPassTask::~RenderPassTask() = default;

void RenderPassTask::addCommands(ResourceProvider* resourceProvider, CommandBuffer* commandBuffer) {
    // TBD: Expose the surfaces that will need to be attached within the renderpass?

    // TODO: for task execution, iterate the draw passes (can assume just 1 for sprint?) and
    // determine RenderPassDesc. Then start the render pass, then iterate passes again and
    // possibly(?) start each subpass, and call DrawPass::addCommands() on the command buffer
    // provided to the task. Then close the render pass and we should have pixels..

    // TODO: For now just generate a renderpass for each draw pass until we start using subpasses
    for (const auto& drawPass: fDrawPasses) {
        RenderPassDesc desc;

        auto target = drawPass->target();
        target->instantiate(resourceProvider);
        desc.fColorAttachment.fTexture = target->refTexture();
        if (!desc.fColorAttachment.fTexture) {
            SkDebugf("WARNING: given invalid texture proxy, will not draw!\n");
            SkDebugf("Dimensions are (%d, %d).\n", target->dimensions().width(),
                     target->dimensions().height());
            continue;
        }
        // TODO: need to get these from the drawPass somehow
        desc.fColorAttachment.fLoadOp = LoadOp::kLoad;
        desc.fColorAttachment.fStoreOp = StoreOp::kStore;

        commandBuffer->beginRenderPass(desc);

        drawPass->addCommands(commandBuffer);

        commandBuffer->endRenderPass();
    }
}

} // namespace skgpu
