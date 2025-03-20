/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/RenderPassTask.h"

#include "src/gpu/SkBackingFit.h"
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

namespace {

SkISize get_msaa_size(const SkISize& targetSize, const Caps& caps) {
    if (caps.differentResolveAttachmentSizeSupport()) {
        // Use approx size for better reuse.
        return GetApproxSize(targetSize);
    }

    return targetSize;
}

}  // anonymous namespace

sk_sp<RenderPassTask> RenderPassTask::Make(DrawPassList passes,
                                           const RenderPassDesc& desc,
                                           sk_sp<TextureProxy> target,
                                           sk_sp<TextureProxy> dstCopy,
                                           SkIRect dstReadBounds) {
    // For now we have one DrawPass per RenderPassTask
    SkASSERT(passes.size() == 1);
    // If we have a dst copy texture, ensure it is big enough to cover the copy bounds that
    // will be sampled.
    SkASSERT(!dstCopy || (dstCopy->dimensions().width() >= dstReadBounds.width() &&
                          dstCopy->dimensions().height() >= dstReadBounds.height()));
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
                                                    dstReadBounds));
}

RenderPassTask::RenderPassTask(DrawPassList passes,
                               const RenderPassDesc& desc,
                               sk_sp<TextureProxy> target,
                               sk_sp<TextureProxy> dstCopy,
                               SkIRect dstReadBounds)
        : fDrawPasses(std::move(passes))
        , fRenderPassDesc(desc)
        , fTarget(std::move(target))
        , fDstCopy(std::move(dstCopy))
        , fDstReadBounds(dstReadBounds) {}

RenderPassTask::~RenderPassTask() = default;

Task::Status RenderPassTask::prepareResources(ResourceProvider* resourceProvider,
                                              ScratchResourceManager* scratchManager,
                                              const RuntimeEffectDictionary* runtimeDict) {
    SkASSERT(fTarget);

    bool instantiated;
    if (scratchManager->pendingReadCount(fTarget.get()) == 0) {
        // TODO(b/389908339, b/338976898): If there are no pending reads on a scratch texture
        // instantiation request, it means that the scratch Device was caught by a
        // Recorder::flushTrackedDevices() event but hasn't actually been restored to its parent. In
        // this case, the eventual read of the surface will be in another Recording and it can't be
        // allocated as a true scratch resource.
        //
        // Without pending reads, DrawTask does not track its lifecycle to return the scratch
        // resource, so we need to match that and instantiate with a regular non-shareable resource.
        instantiated = TextureProxy::InstantiateIfNotLazy(resourceProvider, fTarget.get());
    } else {
        instantiated = TextureProxy::InstantiateIfNotLazy(scratchManager, fTarget.get());
    }
    if (!instantiated) {
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

    // Only apply the replay translation and clip if we're drawing to the final replay target.
    const SkIRect renderTargetBounds = SkIRect::MakeSize(fTarget->dimensions());
    if (fTarget->texture() == replayData.fTarget) {
        // The clip set here will intersect with the render target bounds, and then any scissor set
        // during this render pass. If there is no intersection between the clip and the render
        // target bounds, we can skip this entire render pass.
        if (!commandBuffer->setReplayTranslationAndClip(
                    replayData.fTranslation, replayData.fClip, renderTargetBounds)) {
            return Status::kSuccess;
        }

    } else {
        // An empty clip is ignored, and will default to the render target bounds.
        constexpr SkIVector kNoReplayTranslation = {0, 0};
        constexpr SkIRect kNoReplayClip = SkIRect::MakeEmpty();
        commandBuffer->setReplayTranslationAndClip(
                kNoReplayTranslation, kNoReplayClip, renderTargetBounds);
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
                get_msaa_size(fTarget->dimensions(), *context->priv().caps()),
                fRenderPassDesc.fColorAttachment.fTextureInfo);
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
                colorAttachment->textureInfo(), colorAttachment->dimensions());

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
                                     fDstReadBounds,
                                     fTarget->dimensions(),
                                     fDrawPasses)) {
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

} // namespace skgpu::graphite
