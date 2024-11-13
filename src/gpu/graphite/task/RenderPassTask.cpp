/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/RenderPassTask.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

sk_sp<RenderPassTask> RenderPassTask::Make(DrawPassList passes,
                                           const RenderPassDesc& desc,
                                           sk_sp<TextureProxy> target,
                                           sk_sp<TextureProxy> dstCopy,
                                           SkIRect dstCopyBounds) {
    // For now we have one DrawPass per RenderPassTask
    SkASSERT(passes.size() == 1);
    // We should only have dst copy bounds if we have a dst copy texture, and the texture should be
    // big enough to cover the copy bounds that will be sampled.
    SkASSERT(dstCopyBounds.isEmpty() == !dstCopy);
    SkASSERT(!dstCopy || (dstCopy->dimensions().width() >= dstCopyBounds.width() &&
                          dstCopy->dimensions().height() >= dstCopyBounds.height()));
    if (!target) {
        return nullptr;
    }

    if (desc.fColorAttachment.fTextureInfo.isValid()) {
        // The color attachment's samples count must ether match the render pass's samples count
        // or be 1 (when multisampled render to single sampled is used).
        SkASSERT(desc.fSampleCount == desc.fColorAttachment.fTextureInfo.numSamples() ||
                 1 == desc.fColorAttachment.fTextureInfo.numSamples());
    }

    if (desc.fDepthStencilAttachment.fTextureInfo.isValid()) {
        SkASSERT(desc.fSampleCount == desc.fDepthStencilAttachment.fTextureInfo.numSamples());
    }

    return sk_sp<RenderPassTask>(new RenderPassTask(std::move(passes),
                                                    desc,
                                                    std::move(target),
                                                    std::move(dstCopy),
                                                    dstCopyBounds));
}

RenderPassTask::RenderPassTask(DrawPassList passes,
                               const RenderPassDesc& desc,
                               sk_sp<TextureProxy> target,
                               sk_sp<TextureProxy> dstCopy,
                               SkIRect dstCopyBounds)
        : fDrawPasses(std::move(passes))
        , fRenderPassDesc(desc)
        , fTarget(std::move(target))
        , fDstCopy(std::move(dstCopy))
        , fDstCopyBounds(dstCopyBounds) {}

RenderPassTask::~RenderPassTask() = default;

Task::Status RenderPassTask::prepareResources(ResourceProvider* resourceProvider,
                                              ScratchResourceManager* scratchManager,
                                              const RuntimeEffectDictionary* runtimeDict) {
    SkASSERT(fTarget);
    if (!TextureProxy::InstantiateIfNotLazy(scratchManager, fTarget.get())) {
        SKGPU_LOG_W("Failed to instantiate RenderPassTask target. Will not create renderpass!");
        SKGPU_LOG_W("Dimensions are (%d, %d).",
                    fTarget->dimensions().width(), fTarget->dimensions().height());
        return Status::kFail;
    }

    // Assuming one draw pass per renderpasstask for now
    SkASSERT(fDrawPasses.size() == 1);
    for (const auto& drawPass: fDrawPasses) {
        if (!drawPass->prepareResources(resourceProvider, runtimeDict, fRenderPassDesc)) {
            return Status::kFail;
        }
    }

    // Once all internal resources have been prepared and instantiated, reclaim any pending returns
    // from the scratch manager, since at the equivalent point in the task graph's addCommands()
    // phase, the renderpass will have sampled from any scratch textures and their contents no
    // longer have to be preserved.
    scratchManager->notifyResourcesConsumed();
    return Status::kSuccess;
}

Task::Status RenderPassTask::addCommands(Context* context,
                                         CommandBuffer* commandBuffer,
                                         ReplayTargetData replayData) {
    // TBD: Expose the surfaces that will need to be attached within the renderpass?

    // Instantiate the target
    SkASSERT(fTarget && fTarget->isInstantiated());
    SkASSERT(!fDstCopy || fDstCopy->isInstantiated());

    // Set any replay translation and clip, as needed.
    // The clip set here will intersect with any scissor set during this render pass.
    const SkIRect renderTargetBounds = SkIRect::MakeSize(fTarget->dimensions());
    if (fTarget->texture() == replayData.fTarget) {
        // We're drawing to the final replay target, so apply replay translation and clip.
        if (replayData.fClip.isEmpty()) {
            // If no replay clip is defined, default to the render target bounds.
            commandBuffer->setReplayTranslationAndClip(replayData.fTranslation,
                                                       renderTargetBounds);
        } else {
            // If a replay clip is defined, intersect it with the render target bounds.
            // If the intersection is empty, we can skip this entire render pass.
            SkIRect replayClip = replayData.fClip;
            if (!replayClip.intersect(renderTargetBounds)) {
                return Status::kSuccess;
            }
            commandBuffer->setReplayTranslationAndClip(replayData.fTranslation, replayClip);
        }
    } else {
        // We're not drawing to the final replay target, so don't apply replay translation or clip.
        // In this case as well, the clip we set defaults to the render target bounds.
        commandBuffer->setReplayTranslationAndClip({0, 0}, renderTargetBounds);
    }

    // We don't instantiate the MSAA or DS attachments in prepareResources because we want to use
    // the discardable attachments from the Context.
    ResourceProvider* resourceProvider = context->priv().resourceProvider();
    sk_sp<Texture> colorAttachment;
    sk_sp<Texture> resolveAttachment;
    if (fRenderPassDesc.fColorResolveAttachment.fTextureInfo.isValid()) {
        SkASSERT(fTarget->numSamples() == 1 &&
                 fRenderPassDesc.fColorAttachment.fTextureInfo.numSamples() > 1);
        colorAttachment = resourceProvider->findOrCreateDiscardableMSAAAttachment(
                fTarget->dimensions(), fRenderPassDesc.fColorAttachment.fTextureInfo);
        if (!colorAttachment) {
            SKGPU_LOG_W("Could not get Color attachment for RenderPassTask");
            return Status::kFail;
        }
        resolveAttachment = fTarget->refTexture();
    } else {
        colorAttachment = fTarget->refTexture();
    }

    sk_sp<Texture> depthStencilAttachment;
    if (fRenderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid()) {
        // TODO: ensure this is a scratch/recycled texture
        SkASSERT(fTarget->isInstantiated());
        SkISize dimensions = context->priv().caps()->getDepthAttachmentDimensions(
                fTarget->texture()->textureInfo(), fTarget->dimensions());
        depthStencilAttachment = resourceProvider->findOrCreateDepthStencilAttachment(
                dimensions, fRenderPassDesc.fDepthStencilAttachment.fTextureInfo);
        if (!depthStencilAttachment) {
            SKGPU_LOG_W("Could not get DepthStencil attachment for RenderPassTask");
            return Status::kFail;
        }
    }

    // TODO(b/313629288) we always pass in the render target's dimensions as the viewport here.
    // Using the dimensions of the logical device that we're drawing to could reduce flakiness in
    // rendering.
    if (commandBuffer->addRenderPass(fRenderPassDesc,
                                     std::move(colorAttachment),
                                     std::move(resolveAttachment),
                                     std::move(depthStencilAttachment),
                                     fDstCopy ? fDstCopy->texture() : nullptr,
                                     fDstCopyBounds,
                                     fTarget->dimensions(),
                                     fDrawPasses)) {
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

} // namespace skgpu::graphite
