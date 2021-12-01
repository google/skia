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

sk_sp<RenderPassTask> RenderPassTask::Make(std::vector<std::unique_ptr<DrawPass>> passes,
                                           const RenderPassDesc& desc,
                                           sk_sp<TextureProxy> target) {
    // For now we have one DrawPass per RenderPassTask
    SkASSERT(passes.size() == 1);

    return sk_sp<RenderPassTask>(new RenderPassTask(std::move(passes), desc, target));
}

RenderPassTask::RenderPassTask(std::vector<std::unique_ptr<DrawPass>> passes,
                               const RenderPassDesc& desc,
                               sk_sp<TextureProxy> target)
        : fDrawPasses(std::move(passes))
        , fRenderPassDesc(desc)
        , fTarget(std::move(target)) {}

RenderPassTask::~RenderPassTask() = default;

void RenderPassTask::addCommands(ResourceProvider* resourceProvider, CommandBuffer* commandBuffer) {
    // TBD: Expose the surfaces that will need to be attached within the renderpass?

    // TODO: for task execution, start the render pass, then iterate passes and
    // possibly(?) start each subpass, and call DrawPass::addCommands() on the command buffer
    // provided to the task. Then close the render pass and we should have pixels..

    // Instantiate the target
    if (fTarget) {
        if (!fTarget->instantiate(resourceProvider)) {
            SkDebugf("WARNING: given invalid texture proxy. Will not create renderpass!\n");
            SkDebugf("Dimensions are (%d, %d).\n", fTarget->dimensions().width(),
                     fTarget->dimensions().height());
            return;
        }
    }
    // TODO: set up depth and stencil

    commandBuffer->beginRenderPass(fRenderPassDesc, fTarget->refTexture(), nullptr, nullptr);

    // Assuming one draw pass per renderpasstask for now
    SkASSERT(fDrawPasses.size() == 1);
    for (const auto& drawPass: fDrawPasses) {
        drawPass->addCommands(commandBuffer, resourceProvider);
    }

    commandBuffer->endRenderPass();
}

} // namespace skgpu
