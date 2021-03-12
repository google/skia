/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkOpsRenderPass.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkRect.h"
#include "include/gpu/GrBackendDrawableInfo.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/vk/GrVkAttachment.h"
#include "src/gpu/vk/GrVkBuffer.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkCommandPool.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkPipeline.h"
#include "src/gpu/vk/GrVkRenderPass.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "src/gpu/vk/GrVkResourceProvider.h"
#include "src/gpu/vk/GrVkSemaphore.h"
#include "src/gpu/vk/GrVkTexture.h"

/////////////////////////////////////////////////////////////////////////////

void get_vk_load_store_ops(GrLoadOp loadOpIn, GrStoreOp storeOpIn,
                           VkAttachmentLoadOp* loadOp, VkAttachmentStoreOp* storeOp) {
    switch (loadOpIn) {
        case GrLoadOp::kLoad:
            *loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case GrLoadOp::kClear:
            *loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        case GrLoadOp::kDiscard:
            *loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        default:
            SK_ABORT("Invalid LoadOp");
            *loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    }

    switch (storeOpIn) {
        case GrStoreOp::kStore:
            *storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case GrStoreOp::kDiscard:
            *storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        default:
            SK_ABORT("Invalid StoreOp");
            *storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }
}

GrVkOpsRenderPass::GrVkOpsRenderPass(GrVkGpu* gpu) : fGpu(gpu) {}

void GrVkOpsRenderPass::setAttachmentLayouts(LoadFromResolve loadFromResolve) {
    bool withStencil = fCurrentRenderPass->hasStencilAttachment();
    bool withResolve = fCurrentRenderPass->hasResolveAttachment();

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    GrVkImage* targetImage = vkRT->colorAttachment();

    if (fSelfDependencyFlags == SelfDependencyFlags::kForInputAttachment) {
        // We need to use the GENERAL layout in this case since we'll be using texture barriers
        // with an input attachment.
        VkAccessFlags dstAccess = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        targetImage->setImageLayout(fGpu, VK_IMAGE_LAYOUT_GENERAL, dstAccess, dstStages, false);
    } else {
        // Change layout of our render target so it can be used as the color attachment.
        // TODO: If we know that we will never be blending or loading the attachment we could drop
        // the VK_ACCESS_COLOR_ATTACHMENT_READ_BIT.
        targetImage->setImageLayout(
                fGpu,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                false);
    }

    if (withResolve) {
        GrVkAttachment* resolveAttachment = vkRT->resolveAttachment();
        SkASSERT(resolveAttachment);
        if (loadFromResolve == LoadFromResolve::kLoad) {
            resolveAttachment->setImageLayout(fGpu,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                              false);
        } else {
            resolveAttachment->setImageLayout(
                    fGpu,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    false);
        }
    }

    // If we are using a stencil attachment we also need to update its layout
    if (withStencil) {
        auto* vkStencil = static_cast<GrVkAttachment*>(fRenderTarget->getStencilAttachment());
        SkASSERT(vkStencil);

        // We need the write and read access bits since we may load and store the stencil.
        // The initial load happens in the VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT so we
        // wait there.
        vkStencil->setImageLayout(fGpu,
                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                  false);
    }
}

// The RenderArea bounds we pass into BeginRenderPass must have a start x value that is a multiple
// of the granularity. The width must also be a multiple of the granularity or eaqual to the width
// the the entire attachment. Similar requirements for the y and height components.
void adjust_bounds_to_granularity(SkIRect* dstBounds,
                                  const SkIRect& srcBounds,
                                  const VkExtent2D& granularity,
                                  int maxWidth,
                                  int maxHeight) {
    // Adjust Width
    if ((0 != granularity.width && 1 != granularity.width)) {
        // Start with the right side of rect so we know if we end up going pass the maxWidth.
        int rightAdj = srcBounds.fRight % granularity.width;
        if (rightAdj != 0) {
            rightAdj = granularity.width - rightAdj;
        }
        dstBounds->fRight = srcBounds.fRight + rightAdj;
        if (dstBounds->fRight > maxWidth) {
            dstBounds->fRight = maxWidth;
            dstBounds->fLeft = 0;
        } else {
            dstBounds->fLeft = srcBounds.fLeft - srcBounds.fLeft % granularity.width;
        }
    } else {
        dstBounds->fLeft = srcBounds.fLeft;
        dstBounds->fRight = srcBounds.fRight;
    }

    // Adjust height
    if ((0 != granularity.height && 1 != granularity.height)) {
        // Start with the bottom side of rect so we know if we end up going pass the maxHeight.
        int bottomAdj = srcBounds.fBottom % granularity.height;
        if (bottomAdj != 0) {
            bottomAdj = granularity.height - bottomAdj;
        }
        dstBounds->fBottom = srcBounds.fBottom + bottomAdj;
        if (dstBounds->fBottom > maxHeight) {
            dstBounds->fBottom = maxHeight;
            dstBounds->fTop = 0;
        } else {
            dstBounds->fTop = srcBounds.fTop - srcBounds.fTop % granularity.height;
        }
    } else {
        dstBounds->fTop = srcBounds.fTop;
        dstBounds->fBottom = srcBounds.fBottom;
    }
}

bool GrVkOpsRenderPass::beginRenderPass(const VkClearValue& clearColor,
                                        LoadFromResolve loadFromResolve) {
    this->setAttachmentLayouts(loadFromResolve);

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);

    bool firstSubpassUsesSecondaryCB =
            loadFromResolve != LoadFromResolve::kLoad && SkToBool(fCurrentSecondaryCommandBuffer);

    bool useFullBounds = fCurrentRenderPass->hasResolveAttachment() &&
                         fGpu->vkCaps().mustLoadFullImageWithDiscardableMSAA();

    auto nativeBounds = GrNativeRect::MakeIRectRelativeTo(
            fOrigin,
            vkRT->height(),
            useFullBounds ? SkIRect::MakeSize(vkRT->dimensions()) : fBounds);

    // The bounds we use for the render pass should be of the granularity supported
    // by the device.
    const VkExtent2D& granularity = fCurrentRenderPass->granularity();
    SkIRect adjustedBounds;
    if ((0 != granularity.width && 1 != granularity.width) ||
        (0 != granularity.height && 1 != granularity.height)) {
        adjust_bounds_to_granularity(&adjustedBounds,
                                     nativeBounds,
                                     granularity,
                                     vkRT->width(),
                                     vkRT->height());
    } else {
        adjustedBounds = nativeBounds;
    }

    if (!fGpu->beginRenderPass(fCurrentRenderPass, &clearColor, vkRT, adjustedBounds,
                               firstSubpassUsesSecondaryCB)) {
        if (fCurrentSecondaryCommandBuffer) {
            fCurrentSecondaryCommandBuffer->end(fGpu);
        }
        fCurrentRenderPass = nullptr;
        return false;
    }

    if (loadFromResolve == LoadFromResolve::kLoad) {
        this->loadResolveIntoMSAA(adjustedBounds);
    }

    return true;
}

bool GrVkOpsRenderPass::init(const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                             const GrOpsRenderPass::LoadAndStoreInfo& resolveInfo,
                             const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
                             std::array<float, 4> clearColor,
                             bool withResolve,
                             bool withStencil) {
    VkAttachmentLoadOp loadOp;
    VkAttachmentStoreOp storeOp;
    get_vk_load_store_ops(colorInfo.fLoadOp, colorInfo.fStoreOp, &loadOp, &storeOp);
    GrVkRenderPass::LoadStoreOps vkColorOps(loadOp, storeOp);

    get_vk_load_store_ops(resolveInfo.fLoadOp, resolveInfo.fStoreOp, &loadOp, &storeOp);
    GrVkRenderPass::LoadStoreOps vkResolveOps(loadOp, storeOp);

    get_vk_load_store_ops(stencilInfo.fLoadOp, stencilInfo.fStoreOp, &loadOp, &storeOp);
    GrVkRenderPass::LoadStoreOps vkStencilOps(loadOp, storeOp);

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);

    const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
            vkRT->compatibleRenderPassHandle(withResolve, withStencil, fSelfDependencyFlags,
                                             fLoadFromResolve);
    if (rpHandle.isValid()) {
        fCurrentRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                     vkColorOps,
                                                                     vkResolveOps,
                                                                     vkStencilOps);
    } else {
        fCurrentRenderPass = fGpu->resourceProvider().findRenderPass(vkRT,
                                                                     vkColorOps,
                                                                     vkResolveOps,
                                                                     vkStencilOps,
                                                                     nullptr,
                                                                     withResolve,
                                                                     withStencil,
                                                                     fSelfDependencyFlags,
                                                                     fLoadFromResolve);
    }
    if (!fCurrentRenderPass) {
        return false;
    }

    if (!fGpu->vkCaps().preferPrimaryOverSecondaryCommandBuffers()) {
        SkASSERT(fGpu->cmdPool());
        fCurrentSecondaryCommandBuffer = fGpu->cmdPool()->findOrCreateSecondaryCommandBuffer(fGpu);
        if (!fCurrentSecondaryCommandBuffer) {
            fCurrentRenderPass = nullptr;
            return false;
        }
        const GrVkFramebuffer* framebuffer = vkRT->getFramebuffer(
                withResolve, withStencil, fSelfDependencyFlags, fLoadFromResolve);
        fCurrentSecondaryCommandBuffer->begin(fGpu, framebuffer, fCurrentRenderPass);
    }

    VkClearValue vkClearColor;
    vkClearColor.color.float32[0] = clearColor[0];
    vkClearColor.color.float32[1] = clearColor[1];
    vkClearColor.color.float32[2] = clearColor[2];
    vkClearColor.color.float32[3] = clearColor[3];

    return this->beginRenderPass(vkClearColor, fLoadFromResolve);
}

bool GrVkOpsRenderPass::initWrapped() {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    SkASSERT(vkRT->wrapsSecondaryCommandBuffer());
    fCurrentRenderPass = vkRT->externalRenderPass();
    SkASSERT(fCurrentRenderPass);
    fCurrentRenderPass->ref();

    fCurrentSecondaryCommandBuffer.reset(
            GrVkSecondaryCommandBuffer::Create(vkRT->getExternalSecondaryCommandBuffer()));
    if (!fCurrentSecondaryCommandBuffer) {
        return false;
    }
    fCurrentSecondaryCommandBuffer->begin(fGpu, nullptr, fCurrentRenderPass);
    return true;
}

GrVkOpsRenderPass::~GrVkOpsRenderPass() {
    this->reset();
}

GrGpu* GrVkOpsRenderPass::gpu() { return fGpu; }

GrVkCommandBuffer* GrVkOpsRenderPass::currentCommandBuffer() {
    if (fCurrentSecondaryCommandBuffer) {
        return fCurrentSecondaryCommandBuffer.get();
    }
    // We checked this when we setup the GrVkOpsRenderPass and it should not have changed while we
    // are still using this object.
    SkASSERT(fGpu->currentCommandBuffer());
    return fGpu->currentCommandBuffer();
}

void GrVkOpsRenderPass::loadResolveIntoMSAA(const SkIRect& nativeBounds) {
    fGpu->loadMSAAFromResolve(this->currentCommandBuffer(), *fCurrentRenderPass, fRenderTarget,
                              fRenderTarget, nativeBounds);
    fGpu->currentCommandBuffer()->nexSubpass(fGpu, SkToBool(fCurrentSecondaryCommandBuffer));

    // If we loaded the resolve attachment, then we would have set the image layout to be
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL so that it could be used at the start as an input
    // attachment. However, when we switched to the main subpass it will transition the layout
    // internally to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL. Thus we need to update our tracking
    // of the layout to match the new layout.
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    vkRT->resolveAttachment()->updateImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void GrVkOpsRenderPass::submit() {
    if (!fRenderTarget) {
        return;
    }
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }

    // We don't want to actually submit the secondary command buffer if it is wrapped.
    if (this->wrapsSecondaryCommandBuffer()) {
        // We pass the ownership of the GrVkSecondaryCommandBuffer to the special wrapped
        // GrVkRenderTarget since it's lifetime matches the lifetime we need to keep the
        // GrManagedResources on the GrVkSecondaryCommandBuffer alive.
        static_cast<GrVkRenderTarget*>(fRenderTarget)->addWrappedGrSecondaryCommandBuffer(
                std::move(fCurrentSecondaryCommandBuffer));
        return;
    }

    if (fCurrentSecondaryCommandBuffer) {
        fGpu->submitSecondaryCommandBuffer(std::move(fCurrentSecondaryCommandBuffer));
    }
    fGpu->endRenderPass(fRenderTarget, fOrigin, fBounds);
}

bool GrVkOpsRenderPass::set(GrRenderTarget* rt,
                            GrAttachment* stencil,
                            GrSurfaceOrigin origin,
                            const SkIRect& bounds,
                            const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
                            const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
                            GrXferBarrierFlags renderPassXferBarriers) {
    SkASSERT(!fRenderTarget);
    SkASSERT(fGpu == rt->getContext()->priv().getGpu());

#ifdef SK_DEBUG
    fIsActive = true;
#endif

    // We check to make sure the GrVkGpu has a valid current command buffer instead of each time we
    // access it. If the command buffer is valid here should be valid throughout the use of the
    // render pass since nothing should trigger a submit while this render pass is active.
    if (!fGpu->currentCommandBuffer()) {
        return false;
    }

    this->INHERITED::set(rt, origin);

    for (int i = 0; i < sampledProxies.count(); ++i) {
        if (sampledProxies[i]->isInstantiated()) {
            SkASSERT(sampledProxies[i]->asTextureProxy());
            GrVkTexture* vkTex = static_cast<GrVkTexture*>(sampledProxies[i]->peekTexture());
            SkASSERT(vkTex);
            GrVkAttachment* texture = vkTex->textureAttachment();
            SkASSERT(texture);
            texture->setImageLayout(
                    fGpu, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, false);
        }
    }

    SkASSERT(bounds.isEmpty() || SkIRect::MakeWH(rt->width(), rt->height()).contains(bounds));
    fBounds = bounds;

    if (renderPassXferBarriers & GrXferBarrierFlags::kBlend) {
        fSelfDependencyFlags |= GrVkRenderPass::SelfDependencyFlags::kForNonCoherentAdvBlend;
    }
    if (renderPassXferBarriers & GrXferBarrierFlags::kTexture) {
        fSelfDependencyFlags |= GrVkRenderPass::SelfDependencyFlags::kForInputAttachment;
    }

    if (this->wrapsSecondaryCommandBuffer()) {
        return this->initWrapped();
    }

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);

    GrOpsRenderPass::LoadAndStoreInfo localColorInfo = colorInfo;

    bool withResolve = false;
    GrOpsRenderPass::LoadAndStoreInfo resolveInfo{GrLoadOp::kLoad, GrStoreOp::kStore, {}};
    if (fRenderTarget->numSamples() > 1 && fGpu->vkCaps().preferDiscardableMSAAAttachment() &&
        vkRT->resolveAttachment() && vkRT->resolveAttachment()->supportsInputAttachmentUsage()) {
        withResolve = true;
        localColorInfo.fStoreOp = GrStoreOp::kDiscard;
        if (colorInfo.fLoadOp == GrLoadOp::kLoad) {
            fLoadFromResolve = LoadFromResolve::kLoad;
            localColorInfo.fLoadOp = GrLoadOp::kDiscard;
        } else {
            resolveInfo.fLoadOp = GrLoadOp::kDiscard;
        }
    }

    return this->init(localColorInfo, resolveInfo, stencilInfo, colorInfo.fClearColor, withResolve,
                      SkToBool(stencil));
}

void GrVkOpsRenderPass::reset() {
    if (fCurrentSecondaryCommandBuffer) {
        // The active GrVkCommandPool on the GrVkGpu should still be the same pool we got the
        // secondary command buffer from since we haven't submitted any work yet.
        SkASSERT(fGpu->cmdPool());
        fCurrentSecondaryCommandBuffer.release()->recycle(fGpu->cmdPool());
    }
    if (fCurrentRenderPass) {
        fCurrentRenderPass->unref();
        fCurrentRenderPass = nullptr;
    }
    fCurrentCBIsEmpty = true;

    fRenderTarget = nullptr;

    fSelfDependencyFlags = GrVkRenderPass::SelfDependencyFlags::kNone;

    fLoadFromResolve = LoadFromResolve::kNo;
    fOverridePipelinesForResolveLoad = false;

#ifdef SK_DEBUG
    fIsActive = false;
#endif
}

bool GrVkOpsRenderPass::wrapsSecondaryCommandBuffer() const {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    return vkRT->wrapsSecondaryCommandBuffer();
}

////////////////////////////////////////////////////////////////////////////////

void GrVkOpsRenderPass::onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) {
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }

    GrAttachment* sb = fRenderTarget->getStencilAttachment();
    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);
    int stencilBitCount = GrBackendFormatStencilBits(sb->backendFormat());

    // The contract with the callers does not guarantee that we preserve all bits in the stencil
    // during this clear. Thus we will clear the entire stencil to the desired value.

    VkClearDepthStencilValue vkStencilColor;
    memset(&vkStencilColor, 0, sizeof(VkClearDepthStencilValue));
    if (insideStencilMask) {
        vkStencilColor.stencil = (1 << (stencilBitCount - 1));
    } else {
        vkStencilColor.stencil = 0;
    }

    VkClearRect clearRect;
    // Flip rect if necessary
    SkIRect vkRect;
    if (!scissor.enabled()) {
        vkRect.setXYWH(0, 0, fRenderTarget->width(), fRenderTarget->height());
    } else if (kBottomLeft_GrSurfaceOrigin != fOrigin) {
        vkRect = scissor.rect();
    } else {
        vkRect.setLTRB(scissor.rect().fLeft, fRenderTarget->height() - scissor.rect().fBottom,
                       scissor.rect().fRight, fRenderTarget->height() - scissor.rect().fTop);
    }

    clearRect.rect.offset = { vkRect.fLeft, vkRect.fTop };
    clearRect.rect.extent = { (uint32_t)vkRect.width(), (uint32_t)vkRect.height() };

    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    uint32_t stencilIndex;
    SkAssertResult(fCurrentRenderPass->stencilAttachmentIndex(&stencilIndex));

    VkClearAttachment attachment;
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    attachment.colorAttachment = 0; // this value shouldn't matter
    attachment.clearValue.depthStencil = vkStencilColor;

    this->currentCommandBuffer()->clearAttachments(fGpu, 1, &attachment, 1, &clearRect);
    fCurrentCBIsEmpty = false;
}

void GrVkOpsRenderPass::onClear(const GrScissorState& scissor, std::array<float, 4> color) {
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }

    VkClearColorValue vkColor = {{color[0], color[1], color[2], color[3]}};

    // If we end up in a situation where we are calling clear without a scissior then in general it
    // means we missed an opportunity higher up the stack to set the load op to be a clear. However,
    // there are situations where higher up we couldn't discard the previous ops and set a clear
    // load op (e.g. if we needed to execute a wait op). Thus we also have the empty check here.
    // TODO: Make the waitOp a RenderTask instead so we can clear out the GrOpsTask for a clear. We
    // can then reenable this assert assuming we can't get messed up by a waitOp.
    //SkASSERT(!fCurrentCBIsEmpty || scissor);

    // We always do a sub rect clear with clearAttachments since we are inside a render pass
    VkClearRect clearRect;
    // Flip rect if necessary
    SkIRect vkRect;
    if (!scissor.enabled()) {
        vkRect.setXYWH(0, 0, fRenderTarget->width(), fRenderTarget->height());
    } else if (kBottomLeft_GrSurfaceOrigin != fOrigin) {
        vkRect = scissor.rect();
    } else {
        vkRect.setLTRB(scissor.rect().fLeft, fRenderTarget->height() - scissor.rect().fBottom,
                       scissor.rect().fRight, fRenderTarget->height() - scissor.rect().fTop);
    }
    clearRect.rect.offset = { vkRect.fLeft, vkRect.fTop };
    clearRect.rect.extent = { (uint32_t)vkRect.width(), (uint32_t)vkRect.height() };
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    uint32_t colorIndex;
    SkAssertResult(fCurrentRenderPass->colorAttachmentIndex(&colorIndex));

    VkClearAttachment attachment;
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = colorIndex;
    attachment.clearValue.color = vkColor;

    this->currentCommandBuffer()->clearAttachments(fGpu, 1, &attachment, 1, &clearRect);
    fCurrentCBIsEmpty = false;
    return;
}

////////////////////////////////////////////////////////////////////////////////

void GrVkOpsRenderPass::addAdditionalRenderPass(bool mustUseSecondaryCommandBuffer) {
    SkASSERT(!this->wrapsSecondaryCommandBuffer());

    bool withStencil = fCurrentRenderPass->hasStencilAttachment();
    bool withResolve = fCurrentRenderPass->hasResolveAttachment();

    // If we have a resolve attachment we must do a resolve load in the new render pass since we
    // broke up the original one. GrProgramInfos were made without any knowledge that the render
    // pass may be split up. Thus they may try to make VkPipelines that only use one subpass. We
    // need to override that to make sure they are compatible with the extra load subpass.
    fOverridePipelinesForResolveLoad |=
            withResolve && fCurrentRenderPass->loadFromResolve() != LoadFromResolve::kLoad;

    GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                            VK_ATTACHMENT_STORE_OP_STORE);
    GrVkRenderPass::LoadStoreOps vkResolveOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                              VK_ATTACHMENT_STORE_OP_STORE);
    LoadFromResolve loadFromResolve = LoadFromResolve::kNo;
    if (withResolve) {
        vkColorOps = {VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE};
        loadFromResolve = LoadFromResolve::kLoad;
    }
    GrVkRenderPass::LoadStoreOps vkStencilOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                              VK_ATTACHMENT_STORE_OP_STORE);

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
            vkRT->compatibleRenderPassHandle(withResolve, withStencil, fSelfDependencyFlags,
                                             loadFromResolve);
    SkASSERT(fCurrentRenderPass);
    fCurrentRenderPass->unref();
    if (rpHandle.isValid()) {
        fCurrentRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                     vkColorOps,
                                                                     vkResolveOps,
                                                                     vkStencilOps);
    } else {
        fCurrentRenderPass = fGpu->resourceProvider().findRenderPass(vkRT,
                                                                     vkColorOps,
                                                                     vkResolveOps,
                                                                     vkStencilOps,
                                                                     nullptr,
                                                                     withResolve,
                                                                     withStencil,
                                                                     fSelfDependencyFlags,
                                                                     loadFromResolve);
    }
    if (!fCurrentRenderPass) {
        return;
    }

    if (!fGpu->vkCaps().preferPrimaryOverSecondaryCommandBuffers() ||
        mustUseSecondaryCommandBuffer) {
        SkASSERT(fGpu->cmdPool());
        fCurrentSecondaryCommandBuffer = fGpu->cmdPool()->findOrCreateSecondaryCommandBuffer(fGpu);
        if (!fCurrentSecondaryCommandBuffer) {
            fCurrentRenderPass = nullptr;
            return;
        }
        const GrVkFramebuffer* framebuffer = vkRT->getFramebuffer(
                withResolve, withStencil, fSelfDependencyFlags, loadFromResolve);
        fCurrentSecondaryCommandBuffer->begin(fGpu, framebuffer, fCurrentRenderPass);
    }

    VkClearValue vkClearColor;
    memset(&vkClearColor, 0, sizeof(VkClearValue));

    this->beginRenderPass(vkClearColor, loadFromResolve);
}

void GrVkOpsRenderPass::inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) {
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }
    if (fCurrentSecondaryCommandBuffer) {
        fCurrentSecondaryCommandBuffer->end(fGpu);
        fGpu->submitSecondaryCommandBuffer(std::move(fCurrentSecondaryCommandBuffer));
    }
    fGpu->endRenderPass(fRenderTarget, fOrigin, fBounds);

    // We pass in true here to signal that after the upload we need to set the upload textures
    // layout back to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
    state->doUpload(upload, true);

    this->addAdditionalRenderPass(false);
}

////////////////////////////////////////////////////////////////////////////////

void GrVkOpsRenderPass::onEnd() {
    if (fCurrentSecondaryCommandBuffer) {
        fCurrentSecondaryCommandBuffer->end(fGpu);
    }
}

bool GrVkOpsRenderPass::onBindPipeline(const GrProgramInfo& programInfo, const SkRect& drawBounds) {
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return false;
    }

    SkRect rtRect = SkRect::Make(fBounds);
    if (rtRect.intersect(drawBounds)) {
        rtRect.roundOut(&fCurrentPipelineBounds);
    } else {
        fCurrentPipelineBounds.setEmpty();
    }

    GrVkCommandBuffer* currentCB = this->currentCommandBuffer();
    SkASSERT(fCurrentRenderPass);

    VkRenderPass compatibleRenderPass = fCurrentRenderPass->vkRenderPass();

    fCurrentPipelineState = fGpu->resourceProvider().findOrCreateCompatiblePipelineState(
            fRenderTarget, programInfo, compatibleRenderPass, fOverridePipelinesForResolveLoad);
    if (!fCurrentPipelineState) {
        return false;
    }

    fCurrentPipelineState->bindPipeline(fGpu, currentCB);

    // Both the 'programInfo' and this renderPass have an origin. Since they come from the
    // same place (i.e., the target renderTargetProxy) they had best agree.
    SkASSERT(programInfo.origin() == fOrigin);

    if (!fCurrentPipelineState->setAndBindUniforms(fGpu, fRenderTarget, programInfo, currentCB)) {
        return false;
    }

    if (!programInfo.pipeline().isScissorTestEnabled()) {
        // "Disable" scissor by setting it to the full pipeline bounds.
        GrVkPipeline::SetDynamicScissorRectState(fGpu, currentCB, fRenderTarget, fOrigin,
                                                 fCurrentPipelineBounds);
    }
    GrVkPipeline::SetDynamicViewportState(fGpu, currentCB, fRenderTarget);
    GrVkPipeline::SetDynamicBlendConstantState(fGpu, currentCB,
                                               programInfo.pipeline().writeSwizzle(),
                                               programInfo.pipeline().getXferProcessor());

    return true;
}

void GrVkOpsRenderPass::onSetScissorRect(const SkIRect& scissor) {
    SkIRect combinedScissorRect;
    if (!combinedScissorRect.intersect(fCurrentPipelineBounds, scissor)) {
        combinedScissorRect = SkIRect::MakeEmpty();
    }
    GrVkPipeline::SetDynamicScissorRectState(fGpu, this->currentCommandBuffer(), fRenderTarget,
                                             fOrigin, combinedScissorRect);
}

#ifdef SK_DEBUG
void check_sampled_texture(GrTexture* tex, GrRenderTarget* rt, GrVkGpu* gpu) {
    SkASSERT(!tex->isProtected() || (rt->isProtected() && gpu->protectedContext()));
    auto vkTex = static_cast<GrVkTexture*>(tex)->textureAttachment();
    SkASSERT(vkTex->currentLayout() == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
#endif

bool GrVkOpsRenderPass::onBindTextures(const GrPrimitiveProcessor& primProc,
                                       const GrSurfaceProxy* const primProcTextures[],
                                       const GrPipeline& pipeline) {
#ifdef SK_DEBUG
    SkASSERT(fCurrentPipelineState);
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        check_sampled_texture(primProcTextures[i]->peekTexture(), fRenderTarget, fGpu);
    }
    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        check_sampled_texture(te.texture(), fRenderTarget, fGpu);
    });
    if (GrTexture* dstTexture = pipeline.peekDstTexture()) {
        check_sampled_texture(dstTexture, fRenderTarget, fGpu);
    }
#endif
    if (!fCurrentPipelineState->setAndBindTextures(fGpu, primProc, pipeline, primProcTextures,
                                                   this->currentCommandBuffer())) {
        return false;
    }
    if (fSelfDependencyFlags == SelfDependencyFlags::kForInputAttachment) {
        return fCurrentPipelineState->setAndBindInputAttachment(
                fGpu, static_cast<GrVkRenderTarget*>(fRenderTarget), this->currentCommandBuffer());
    }
    return true;
}

void GrVkOpsRenderPass::onBindBuffers(sk_sp<const GrBuffer> indexBuffer,
                                      sk_sp<const GrBuffer> instanceBuffer,
                                      sk_sp<const GrBuffer> vertexBuffer,
                                      GrPrimitiveRestart primRestart) {
    SkASSERT(GrPrimitiveRestart::kNo == primRestart);
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }
    SkASSERT(fCurrentPipelineState);
    SkASSERT(!fGpu->caps()->usePrimitiveRestart());  // Ignore primitiveRestart parameter.

    GrVkCommandBuffer* currCmdBuf = this->currentCommandBuffer();
    SkASSERT(currCmdBuf);

    // There is no need to put any memory barriers to make sure host writes have finished here.
    // When a command buffer is submitted to a queue, there is an implicit memory barrier that
    // occurs for all host writes. Additionally, BufferMemoryBarriers are not allowed inside of
    // an active RenderPass.

    // Here our vertex and instance inputs need to match the same 0-based bindings they were
    // assigned in GrVkPipeline. That is, vertex first (if any) followed by instance.
    uint32_t binding = 0;
    if (auto* gpuVertexBuffer = static_cast<const GrGpuBuffer*>(vertexBuffer.get())) {
        SkASSERT(!gpuVertexBuffer->isCpuBuffer());
        SkASSERT(!gpuVertexBuffer->isMapped());
        currCmdBuf->bindInputBuffer(fGpu, binding++, std::move(vertexBuffer));
    }
    if (auto* gpuInstanceBuffer = static_cast<const GrGpuBuffer*>(instanceBuffer.get())) {
        SkASSERT(!gpuInstanceBuffer->isCpuBuffer());
        SkASSERT(!gpuInstanceBuffer->isMapped());
        currCmdBuf->bindInputBuffer(fGpu, binding++, std::move(instanceBuffer));
    }
    if (auto* gpuIndexBuffer = static_cast<const GrGpuBuffer*>(indexBuffer.get())) {
        SkASSERT(!gpuIndexBuffer->isCpuBuffer());
        SkASSERT(!gpuIndexBuffer->isMapped());
        currCmdBuf->bindIndexBuffer(fGpu, std::move(indexBuffer));
    }
}

void GrVkOpsRenderPass::onDrawInstanced(int instanceCount,
                                        int baseInstance,
                                        int vertexCount, int baseVertex) {
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }
    SkASSERT(fCurrentPipelineState);
    this->currentCommandBuffer()->draw(fGpu, vertexCount, instanceCount, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
    fCurrentCBIsEmpty = false;
}

void GrVkOpsRenderPass::onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount,
                                               int baseInstance, int baseVertex) {
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }
    SkASSERT(fCurrentPipelineState);
    this->currentCommandBuffer()->drawIndexed(fGpu, indexCount, instanceCount,
                                              baseIndex, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
    fCurrentCBIsEmpty = false;
}

void GrVkOpsRenderPass::onDrawIndirect(const GrBuffer* drawIndirectBuffer, size_t offset,
                                       int drawCount) {
    SkASSERT(!drawIndirectBuffer->isCpuBuffer());
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }
    const GrVkCaps& caps = fGpu->vkCaps();
    SkASSERT(caps.nativeDrawIndirectSupport());
    SkASSERT(fCurrentPipelineState);

    const uint32_t maxDrawCount = caps.maxDrawIndirectDrawCount();
    uint32_t remainingDraws = drawCount;
    const size_t stride = sizeof(GrDrawIndirectCommand);
    while (remainingDraws >= 1) {
        uint32_t currDrawCount = std::min(remainingDraws, maxDrawCount);
        this->currentCommandBuffer()->drawIndirect(
                fGpu, sk_ref_sp(drawIndirectBuffer), offset, currDrawCount, stride);
        remainingDraws -= currDrawCount;
        offset += stride * currDrawCount;
        fGpu->stats()->incNumDraws();
    }
    fCurrentCBIsEmpty = false;
}

void GrVkOpsRenderPass::onDrawIndexedIndirect(const GrBuffer* drawIndirectBuffer, size_t offset,
                                              int drawCount) {
    SkASSERT(!drawIndirectBuffer->isCpuBuffer());
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }
    const GrVkCaps& caps = fGpu->vkCaps();
    SkASSERT(caps.nativeDrawIndirectSupport());
    SkASSERT(fCurrentPipelineState);
    const uint32_t maxDrawCount = caps.maxDrawIndirectDrawCount();
    uint32_t remainingDraws = drawCount;
    const size_t stride = sizeof(GrDrawIndexedIndirectCommand);
    while (remainingDraws >= 1) {
        uint32_t currDrawCount = std::min(remainingDraws, maxDrawCount);
        this->currentCommandBuffer()->drawIndexedIndirect(
                fGpu, sk_ref_sp(drawIndirectBuffer), offset, currDrawCount, stride);
        remainingDraws -= currDrawCount;
        offset += stride * currDrawCount;
        fGpu->stats()->incNumDraws();
    }
    fCurrentCBIsEmpty = false;
}

////////////////////////////////////////////////////////////////////////////////

void GrVkOpsRenderPass::onExecuteDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable) {
    if (!fCurrentRenderPass) {
        SkASSERT(fGpu->isDeviceLost());
        return;
    }
    GrVkRenderTarget* target = static_cast<GrVkRenderTarget*>(fRenderTarget);

    GrVkImage* targetImage = target->colorAttachment();

    VkRect2D bounds;
    bounds.offset = { 0, 0 };
    bounds.extent = { 0, 0 };

    if (!fCurrentSecondaryCommandBuffer) {
        fGpu->endRenderPass(fRenderTarget, fOrigin, fBounds);
        this->addAdditionalRenderPass(true);
        // We may have failed to start a new render pass
        if (!fCurrentRenderPass) {
            SkASSERT(fGpu->isDeviceLost());
            return;
        }
    }
    SkASSERT(fCurrentSecondaryCommandBuffer);

    GrVkDrawableInfo vkInfo;
    vkInfo.fSecondaryCommandBuffer = fCurrentSecondaryCommandBuffer->vkCommandBuffer();
    vkInfo.fCompatibleRenderPass = fCurrentRenderPass->vkRenderPass();
    SkAssertResult(fCurrentRenderPass->colorAttachmentIndex(&vkInfo.fColorAttachmentIndex));
    vkInfo.fFormat = targetImage->imageFormat();
    vkInfo.fDrawBounds = &bounds;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    vkInfo.fImage = targetImage->image();
#else
    vkInfo.fImage = VK_NULL_HANDLE;
#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK

    GrBackendDrawableInfo info(vkInfo);

    // After we draw into the command buffer via the drawable, cached state we have may be invalid.
    this->currentCommandBuffer()->invalidateState();
    // Also assume that the drawable produced output.
    fCurrentCBIsEmpty = false;

    drawable->draw(info);
    fGpu->addDrawable(std::move(drawable));
}
