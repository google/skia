/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/RenderPassTask.h"

#include "include/core/SkPoint.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureProxy.h"

#include <tuple>
#include <utility>

namespace skgpu::graphite {

namespace {

// Get the required MSAA size for the render pass.
// In some scenarios, the MSAA size can be smaller than the target texture. As long as it is big
// enough to contain the draws' bounds.
std::pair<SkISize, SkIPoint> get_msaa_size_and_resolve_offset(const SkISize& targetSize,
                                                              const SkIRect& drawBounds,
                                                              const Caps& caps,
                                                              LoadOp loadOp) {
    if (caps.differentResolveAttachmentSizeSupport()) {
        // If possible, use approx size that can fit all draws. This reduces the MSAA texture size
        // and also reuses the textures better.
        // Note: we don't do this if loadOp=Clear because it's supposed to update the whole target
        // texture.
        auto smallEnoughBounds = drawBounds;
        if (loadOp != LoadOp::kClear && !smallEnoughBounds.isEmpty() &&
            smallEnoughBounds.intersect(SkIRect::MakeSize(targetSize))) {
            SkIPoint resolveOffset = smallEnoughBounds.topLeft();
            return {GetApproxSize(smallEnoughBounds.size()), resolveOffset};
        } else {
            return {GetApproxSize(targetSize), {0, 0}};
        }
    }

    return {targetSize, {0, 0}};
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

    if (desc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported) {
        // The resolve attachment must match `target`, since that is what's resolved to.
        SkASSERT(desc.fColorResolveAttachment.isCompatible(target->textureInfo()));
        // The resolve attachment should be single sampled and not depth/stencil
        SkASSERT(desc.fColorResolveAttachment.fSampleCount == 1);
        SkASSERT(!TextureFormatIsDepthOrStencil(desc.fColorResolveAttachment.fFormat));
        // If there's a resolve attachment, the color attachment should have the same format and
        // more samples than the resolve.
        SkASSERT(desc.fColorAttachment.fFormat == desc.fColorResolveAttachment.fFormat);
        SkASSERT(desc.fColorAttachment.fSampleCount > 1);
        // The render pass's sample count must match the color attachment's sample count
        SkASSERT(desc.fSampleCount == desc.fColorAttachment.fSampleCount);
    } else {
        // The color attachment must match `target`, as it will be used to render directly into.
        SkASSERT(desc.fColorAttachment.isCompatible(target->textureInfo()));
        // The render pass's sample count must match or the color attachment's must be 1 and
        // the render pass has a higher sample count for msaa-render-to-single-sampled extensions.
        SkASSERT(desc.fColorAttachment.fSampleCount == desc.fSampleCount ||
                 (desc.fColorAttachment.fSampleCount == 1 && desc.fSampleCount > 1));
    }

    if (desc.fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported) {
        // The sample count for any depth/stencil buffer must match the color attachment
        SkASSERT(TextureFormatIsDepthOrStencil(desc.fDepthStencilAttachment.fFormat));
        SkASSERT(desc.fDepthStencilAttachment.fSampleCount == desc.fColorAttachment.fSampleCount);
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

    // Assuming one draw pass per renderpasstask for now
    SkASSERT(fDrawPasses.size() == 1);
    const auto& drawBounds = fDrawPasses[0]->bounds();

    // Only apply the replay translation and clip if we're drawing to the final replay target.
    SkIVector replayTranslation = {0, 0};
    SkIRect replayClip = SkIRect::MakeEmpty();
    if (fTarget->texture() == replayData.fTarget) {
        replayTranslation = replayData.fTranslation;
        replayClip = replayData.fClip;
    }

    // We don't instantiate the MSAA or DS attachments in prepareResources because we want to use
    // the discardable attachments from the Context.
    ResourceProvider* resourceProvider = context->priv().resourceProvider();
    sk_sp<Texture> colorAttachment;
    sk_sp<Texture> resolveAttachment;
    SkIPoint resolveOffset = SkIPoint::Make(0, 0);
    if (fRenderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported) {
        // We always make color msaa attachments shareable. Between any render pass we discard
        // the values of the MSAA texture. Thus it is safe to be used by multiple different render
        // passes without worry of stomping on each other's data. CommandBuffer::addRenderPass is
        // responsible for loading this attachment with the resolve target's original contents.
        TextureInfo colorInfo = context->priv().caps()->getDefaultAttachmentTextureInfo(
                fRenderPassDesc.fColorAttachment, fTarget->isProtected(), Discardable::kYes);

        SkISize msaaSize;
        std::tie(msaaSize, resolveOffset) =
                get_msaa_size_and_resolve_offset(fTarget->dimensions(),
                                                 drawBounds.makeOffset(replayTranslation),
                                                 *context->priv().caps(),
                                                 fRenderPassDesc.fColorAttachment.fLoadOp);
        colorAttachment = resourceProvider->findOrCreateShareableTexture(
                msaaSize, colorInfo, "DiscardableMSAAAttachment");
        if (!colorAttachment) {
            SKGPU_LOG_W("Could not get Color attachment for RenderPassTask");
            return Status::kFail;
        }
        resolveAttachment = fTarget->refTexture();
    } else {
        colorAttachment = fTarget->refTexture();
    }

    sk_sp<Texture> depthStencilAttachment;
    if (fRenderPassDesc.fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported) {
        // We always make depth and stencil attachments shareable. Between any render pass the
        // values are reset. Thus it is safe to be used by multiple different render passes without
        // worry of stomping on each other's data.
        TextureInfo dsInfo = context->priv().caps()->getDefaultAttachmentTextureInfo(
                fRenderPassDesc.fDepthStencilAttachment, fTarget->isProtected(), Discardable::kYes);
        SkISize dimensions = context->priv().caps()->getDepthAttachmentDimensions(
                colorAttachment->textureInfo(), colorAttachment->dimensions());

        depthStencilAttachment = resourceProvider->findOrCreateShareableTexture(
                dimensions, dsInfo, "DepthStencilAttachment");
        if (!depthStencilAttachment) {
            SKGPU_LOG_W("Could not get DepthStencil attachment for RenderPassTask");
            return Status::kFail;
        }
    }

    // The clip set here will intersect with the render target bounds, and then any scissor set
    // during this render pass. If there is no intersection between the clip and the render target
    // bounds, we can skip this entire render pass.
    // Note: if the MSAA texture is allocated smaller than the target texture, we need to apply an
    // additional translation (-resolveOffset) so that the draws' bounds' top left corner
    // will be at (0, 0) on the MSAA texture
    const SkIRect renderTargetBounds = SkIRect::MakeSize(colorAttachment->dimensions());
    if (!commandBuffer->setReplayTranslationAndClip(
                replayTranslation - resolveOffset, replayClip, renderTargetBounds)) {
        return Status::kSuccess;
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
                                     resolveOffset,
                                     fTarget->dimensions(),
                                     fDrawPasses)) {
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

} // namespace skgpu::graphite
