/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkGpuCommandBuffer.h"

#include "GrFixedClip.h"
#include "GrMesh.h"
#include "GrOpFlushState.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrTexturePriv.h"
#include "GrVkCommandBuffer.h"
#include "GrVkGpu.h"
#include "GrVkPipeline.h"
#include "GrVkRenderPass.h"
#include "GrVkRenderTarget.h"
#include "GrVkResourceProvider.h"
#include "GrVkTexture.h"
#include "SkRect.h"

void GrVkGpuTextureCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin,
                                       const SkIRect& srcRect, const SkIPoint& dstPoint) {
    fCopies.emplace_back(src, srcOrigin, srcRect, dstPoint);
}

void GrVkGpuTextureCommandBuffer::insertEventMarker(const char* msg) {
    // TODO: does Vulkan have a correlate?
}

void GrVkGpuTextureCommandBuffer::submit() {
    for (int i = 0; i < fCopies.count(); ++i) {
        CopyInfo& copyInfo = fCopies[i];
        fGpu->copySurface(fTexture, fOrigin, copyInfo.fSrc, copyInfo.fSrcOrigin, copyInfo.fSrcRect,
                          copyInfo.fDstPoint);
    }
}

GrVkGpuTextureCommandBuffer::~GrVkGpuTextureCommandBuffer() {}

////////////////////////////////////////////////////////////////////////////////

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

GrVkGpuRTCommandBuffer::GrVkGpuRTCommandBuffer(GrVkGpu* gpu,
                                               GrRenderTarget* rt, GrSurfaceOrigin origin,
                                               const LoadAndStoreInfo& colorInfo,
                                               const StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu)
        , fClearColor(GrColor4f::FromGrColor(colorInfo.fClearColor))
        , fLastPipelineState(nullptr) {
    get_vk_load_store_ops(colorInfo.fLoadOp, colorInfo.fStoreOp,
                          &fVkColorLoadOp, &fVkColorStoreOp);

    get_vk_load_store_ops(stencilInfo.fLoadOp, stencilInfo.fStoreOp,
                          &fVkStencilLoadOp, &fVkStencilStoreOp);
    fCurrentCmdInfo = -1;

    this->init();
}

void GrVkGpuRTCommandBuffer::init() {
    GrVkRenderPass::LoadStoreOps vkColorOps(fVkColorLoadOp, fVkColorStoreOp);
    GrVkRenderPass::LoadStoreOps vkStencilOps(fVkStencilLoadOp, fVkStencilStoreOp);

    CommandBufferInfo& cbInfo = fCommandBufferInfos.push_back();
    SkASSERT(fCommandBufferInfos.count() == 1);
    fCurrentCmdInfo = 0;

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    const GrVkResourceProvider::CompatibleRPHandle& rpHandle = vkRT->compatibleRenderPassHandle();
    if (rpHandle.isValid()) {
        cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                     vkColorOps,
                                                                     vkStencilOps);
    } else {
        cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                     vkColorOps,
                                                                     vkStencilOps);
    }

    cbInfo.fColorClearValue.color.float32[0] = fClearColor.fRGBA[0];
    cbInfo.fColorClearValue.color.float32[1] = fClearColor.fRGBA[1];
    cbInfo.fColorClearValue.color.float32[2] = fClearColor.fRGBA[2];
    cbInfo.fColorClearValue.color.float32[3] = fClearColor.fRGBA[3];

    if (VK_ATTACHMENT_LOAD_OP_CLEAR == fVkColorLoadOp) {
        cbInfo.fBounds = SkRect::MakeWH(vkRT->width(), vkRT->height());
    } else {
        cbInfo.fBounds.setEmpty();
    }

    if (VK_ATTACHMENT_LOAD_OP_CLEAR == fVkColorLoadOp) {
        cbInfo.fLoadStoreState = LoadStoreState::kStartsWithClear;
    } else if (VK_ATTACHMENT_LOAD_OP_LOAD == fVkColorLoadOp &&
               VK_ATTACHMENT_STORE_OP_STORE == fVkColorStoreOp) {
        cbInfo.fLoadStoreState = LoadStoreState::kLoadAndStore;
    } else if (VK_ATTACHMENT_LOAD_OP_DONT_CARE == fVkColorLoadOp) {
        cbInfo.fLoadStoreState = LoadStoreState::kStartsWithDiscard;
    }

    cbInfo.fCommandBuffers.push_back(fGpu->resourceProvider().findOrCreateSecondaryCommandBuffer());
    cbInfo.currentCmdBuf()->begin(fGpu, vkRT->framebuffer(), cbInfo.fRenderPass);
}


GrVkGpuRTCommandBuffer::~GrVkGpuRTCommandBuffer() {
    for (int i = 0; i < fCommandBufferInfos.count(); ++i) {
        CommandBufferInfo& cbInfo = fCommandBufferInfos[i];
        for (int j = 0; j < cbInfo.fCommandBuffers.count(); ++j) {
            cbInfo.fCommandBuffers[j]->unref(fGpu);
        }
        cbInfo.fRenderPass->unref(fGpu);
    }
}

GrGpu* GrVkGpuRTCommandBuffer::gpu() { return fGpu; }

void GrVkGpuRTCommandBuffer::end() {
    if (fCurrentCmdInfo >= 0) {
        fCommandBufferInfos[fCurrentCmdInfo].currentCmdBuf()->end(fGpu);
    }
}

void GrVkGpuRTCommandBuffer::submit() {
    if (!fRenderTarget) {
        return;
    }

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    GrVkImage* targetImage = vkRT->msaaImage() ? vkRT->msaaImage() : vkRT;
    GrStencilAttachment* stencil = fRenderTarget->renderTargetPriv().getStencilAttachment();

    for (int i = 0; i < fCommandBufferInfos.count(); ++i) {
        CommandBufferInfo& cbInfo = fCommandBufferInfos[i];

        for (int j = 0; j < cbInfo.fPreDrawUploads.count(); ++j) {
            InlineUploadInfo& iuInfo = cbInfo.fPreDrawUploads[j];
            iuInfo.fFlushState->doUpload(iuInfo.fUpload);
        }

        for (int j = 0; j < cbInfo.fPreCopies.count(); ++j) {
            CopyInfo& copyInfo = cbInfo.fPreCopies[j];
            fGpu->copySurface(fRenderTarget, fOrigin, copyInfo.fSrc, copyInfo.fSrcOrigin,
                              copyInfo.fSrcRect, copyInfo.fDstPoint, copyInfo.fShouldDiscardDst);
        }


        // TODO: Many things create a scratch texture which adds the discard immediately, but then
        // don't draw to it right away. This causes the discard to be ignored and we get yelled at
        // for loading uninitialized data. However, once MDB lands with reordering, the discard will
        // get reordered with the rest of the draw commands and we can remove the discard check.
        if (cbInfo.fIsEmpty &&
            cbInfo.fLoadStoreState != LoadStoreState::kStartsWithClear &&
            cbInfo.fLoadStoreState != LoadStoreState::kStartsWithDiscard) {
            // We have sumbitted no actual draw commands to the command buffer and we are not using
            // the render pass to do a clear so there is no need to submit anything.
            continue;
        }

        // Make sure if we only have a discard load that we execute the discard on the whole image.
        // TODO: Once we improve our tracking of discards so that we never end up flushing a discard
        // call with no actually ops, remove this.
        if (cbInfo.fIsEmpty && cbInfo.fLoadStoreState == LoadStoreState::kStartsWithDiscard) {
            cbInfo.fBounds = SkRect::MakeWH(vkRT->width(), vkRT->height());
        }

        if (cbInfo.fBounds.intersect(0, 0,
                                     SkIntToScalar(fRenderTarget->width()),
                                     SkIntToScalar(fRenderTarget->height()))) {
            // Make sure we do the following layout changes after all copies, uploads, or any other
            // pre-work is done since we may change the layouts in the pre-work. Also since the
            // draws will be submitted in different render passes, we need to guard againts write
            // and write issues.

            // Change layout of our render target so it can be used as the color attachment.
            targetImage->setImageLayout(fGpu,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                        false);

            // If we are using a stencil attachment we also need to update its layout
            if (stencil) {
                GrVkStencilAttachment* vkStencil = (GrVkStencilAttachment*)stencil;
                vkStencil->setImageLayout(fGpu,
                                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                          VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                          false);
            }

            // If we have any sampled images set their layout now.
            for (int j = 0; j < cbInfo.fSampledImages.count(); ++j) {
                cbInfo.fSampledImages[j]->setImageLayout(fGpu,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                         VK_ACCESS_SHADER_READ_BIT,
                                                         VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                                         false);
            }

            SkIRect iBounds;
            cbInfo.fBounds.roundOut(&iBounds);

            fGpu->submitSecondaryCommandBuffer(cbInfo.fCommandBuffers, cbInfo.fRenderPass,
                                               &cbInfo.fColorClearValue, vkRT, fOrigin, iBounds);
        }
    }
}

void GrVkGpuRTCommandBuffer::discard() {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);

    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];
    if (cbInfo.fIsEmpty) {
        // Change the render pass to do a don't-care load for both color & stencil
        GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_STORE);
        GrVkRenderPass::LoadStoreOps vkStencilOps(VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                  VK_ATTACHMENT_STORE_OP_STORE);

        const GrVkRenderPass* oldRP = cbInfo.fRenderPass;

        const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
            vkRT->compatibleRenderPassHandle();
        if (rpHandle.isValid()) {
            cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                         vkColorOps,
                                                                         vkStencilOps);
        } else {
            cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                         vkColorOps,
                                                                         vkStencilOps);
        }

        SkASSERT(cbInfo.fRenderPass->isCompatible(*oldRP));
        oldRP->unref(fGpu);
        cbInfo.fBounds.join(fRenderTarget->getBoundsRect());
        cbInfo.fLoadStoreState = LoadStoreState::kStartsWithDiscard;
        // If we are going to discard the whole render target then the results of any copies we did
        // immediately before to the target won't matter, so just drop them.
        cbInfo.fPreCopies.reset();
    }
}

void GrVkGpuRTCommandBuffer::insertEventMarker(const char* msg) {
    // TODO: does Vulkan have a correlate?
}

void GrVkGpuRTCommandBuffer::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    SkASSERT(!clip.hasWindowRectangles());

    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];

    GrStencilAttachment* sb = fRenderTarget->renderTargetPriv().getStencilAttachment();
    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);
    int stencilBitCount = sb->bits();

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
    if (!clip.scissorEnabled()) {
        vkRect.setXYWH(0, 0, fRenderTarget->width(), fRenderTarget->height());
    } else if (kBottomLeft_GrSurfaceOrigin != fOrigin) {
        vkRect = clip.scissorRect();
    } else {
        const SkIRect& scissor = clip.scissorRect();
        vkRect.setLTRB(scissor.fLeft, fRenderTarget->height() - scissor.fBottom,
                       scissor.fRight, fRenderTarget->height() - scissor.fTop);
    }

    clearRect.rect.offset = { vkRect.fLeft, vkRect.fTop };
    clearRect.rect.extent = { (uint32_t)vkRect.width(), (uint32_t)vkRect.height() };

    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    uint32_t stencilIndex;
    SkAssertResult(cbInfo.fRenderPass->stencilAttachmentIndex(&stencilIndex));

    VkClearAttachment attachment;
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    attachment.colorAttachment = 0; // this value shouldn't matter
    attachment.clearValue.depthStencil = vkStencilColor;

    cbInfo.currentCmdBuf()->clearAttachments(fGpu, 1, &attachment, 1, &clearRect);
    cbInfo.fIsEmpty = false;

    // Update command buffer bounds
    if (!clip.scissorEnabled()) {
        cbInfo.fBounds.join(fRenderTarget->getBoundsRect());
    } else {
        cbInfo.fBounds.join(SkRect::Make(clip.scissorRect()));
    }
}

void GrVkGpuRTCommandBuffer::onClear(const GrFixedClip& clip, GrColor color) {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);

    // parent class should never let us get here with no RT
    SkASSERT(!clip.hasWindowRectangles());

    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];

    VkClearColorValue vkColor;
    GrColorToRGBAFloat(color, vkColor.float32);

    if (cbInfo.fIsEmpty && !clip.scissorEnabled()) {
        // Change the render pass to do a clear load
        GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                VK_ATTACHMENT_STORE_OP_STORE);
        // Preserve the stencil buffer's load & store settings
        GrVkRenderPass::LoadStoreOps vkStencilOps(fVkStencilLoadOp, fVkStencilStoreOp);

        const GrVkRenderPass* oldRP = cbInfo.fRenderPass;

        const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
            vkRT->compatibleRenderPassHandle();
        if (rpHandle.isValid()) {
            cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                         vkColorOps,
                                                                         vkStencilOps);
        } else {
            cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                         vkColorOps,
                                                                         vkStencilOps);
        }

        SkASSERT(cbInfo.fRenderPass->isCompatible(*oldRP));
        oldRP->unref(fGpu);

        GrColorToRGBAFloat(color, cbInfo.fColorClearValue.color.float32);
        cbInfo.fLoadStoreState = LoadStoreState::kStartsWithClear;
        // If we are going to clear the whole render target then the results of any copies we did
        // immediately before to the target won't matter, so just drop them.
        cbInfo.fPreCopies.reset();

        // Update command buffer bounds
        cbInfo.fBounds.join(fRenderTarget->getBoundsRect());
        return;
    }

    // We always do a sub rect clear with clearAttachments since we are inside a render pass
    VkClearRect clearRect;
    // Flip rect if necessary
    SkIRect vkRect;
    if (!clip.scissorEnabled()) {
        vkRect.setXYWH(0, 0, fRenderTarget->width(), fRenderTarget->height());
    } else if (kBottomLeft_GrSurfaceOrigin != fOrigin) {
        vkRect = clip.scissorRect();
    } else {
        const SkIRect& scissor = clip.scissorRect();
        vkRect.setLTRB(scissor.fLeft, fRenderTarget->height() - scissor.fBottom,
                       scissor.fRight, fRenderTarget->height() - scissor.fTop);
    }
    clearRect.rect.offset = { vkRect.fLeft, vkRect.fTop };
    clearRect.rect.extent = { (uint32_t)vkRect.width(), (uint32_t)vkRect.height() };
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    uint32_t colorIndex;
    SkAssertResult(cbInfo.fRenderPass->colorAttachmentIndex(&colorIndex));

    VkClearAttachment attachment;
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = colorIndex;
    attachment.clearValue.color = vkColor;

    cbInfo.currentCmdBuf()->clearAttachments(fGpu, 1, &attachment, 1, &clearRect);
    cbInfo.fIsEmpty = false;

    // Update command buffer bounds
    if (!clip.scissorEnabled()) {
        cbInfo.fBounds.join(fRenderTarget->getBoundsRect());
    } else {
        cbInfo.fBounds.join(SkRect::Make(clip.scissorRect()));
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////

void GrVkGpuRTCommandBuffer::addAdditionalCommandBuffer() {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);

    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];
    cbInfo.currentCmdBuf()->end(fGpu);
    cbInfo.fCommandBuffers.push_back(fGpu->resourceProvider().findOrCreateSecondaryCommandBuffer());
    cbInfo.currentCmdBuf()->begin(fGpu, vkRT->framebuffer(), cbInfo.fRenderPass);
}

void GrVkGpuRTCommandBuffer::addAdditionalRenderPass() {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);

    fCommandBufferInfos[fCurrentCmdInfo].currentCmdBuf()->end(fGpu);

    CommandBufferInfo& cbInfo = fCommandBufferInfos.push_back();
    fCurrentCmdInfo++;

    GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                            VK_ATTACHMENT_STORE_OP_STORE);
    GrVkRenderPass::LoadStoreOps vkStencilOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                              VK_ATTACHMENT_STORE_OP_STORE);

    const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
            vkRT->compatibleRenderPassHandle();
    if (rpHandle.isValid()) {
        cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                     vkColorOps,
                                                                     vkStencilOps);
    } else {
        cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                     vkColorOps,
                                                                     vkStencilOps);
    }
    cbInfo.fLoadStoreState = LoadStoreState::kLoadAndStore;

    cbInfo.fCommandBuffers.push_back(fGpu->resourceProvider().findOrCreateSecondaryCommandBuffer());
    // It shouldn't matter what we set the clear color to here since we will assume loading of the
    // attachment.
    memset(&cbInfo.fColorClearValue, 0, sizeof(VkClearValue));
    cbInfo.fBounds.setEmpty();

    cbInfo.currentCmdBuf()->begin(fGpu, vkRT->framebuffer(), cbInfo.fRenderPass);
}

void GrVkGpuRTCommandBuffer::inlineUpload(GrOpFlushState* state,
                                          GrDeferredTextureUploadFn& upload) {
    if (!fCommandBufferInfos[fCurrentCmdInfo].fIsEmpty) {
        this->addAdditionalRenderPass();
    }
    fCommandBufferInfos[fCurrentCmdInfo].fPreDrawUploads.emplace_back(state, upload);
}

void GrVkGpuRTCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];
    if (!cbInfo.fIsEmpty || LoadStoreState::kStartsWithClear == cbInfo.fLoadStoreState) {
        this->addAdditionalRenderPass();
    }

    fCommandBufferInfos[fCurrentCmdInfo].fPreCopies.emplace_back(
            src, srcOrigin, srcRect, dstPoint,
            LoadStoreState::kStartsWithDiscard == cbInfo.fLoadStoreState);

    if (LoadStoreState::kLoadAndStore != cbInfo.fLoadStoreState) {
        // Change the render pass to do a load and store so we don't lose the results of our copy
        GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                                VK_ATTACHMENT_STORE_OP_STORE);
        GrVkRenderPass::LoadStoreOps vkStencilOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                                  VK_ATTACHMENT_STORE_OP_STORE);

        const GrVkRenderPass* oldRP = cbInfo.fRenderPass;

        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
        const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
                vkRT->compatibleRenderPassHandle();
        if (rpHandle.isValid()) {
            cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                         vkColorOps,
                                                                         vkStencilOps);
        } else {
            cbInfo.fRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                         vkColorOps,
                                                                         vkStencilOps);
        }
        SkASSERT(cbInfo.fRenderPass->isCompatible(*oldRP));
        oldRP->unref(fGpu);

        cbInfo.fLoadStoreState = LoadStoreState::kLoadAndStore;

    }
}

////////////////////////////////////////////////////////////////////////////////

void GrVkGpuRTCommandBuffer::bindGeometry(const GrBuffer* indexBuffer,
                                          const GrBuffer* vertexBuffer,
                                          const GrBuffer* instanceBuffer) {
    GrVkSecondaryCommandBuffer* currCmdBuf = fCommandBufferInfos[fCurrentCmdInfo].currentCmdBuf();
    // There is no need to put any memory barriers to make sure host writes have finished here.
    // When a command buffer is submitted to a queue, there is an implicit memory barrier that
    // occurs for all host writes. Additionally, BufferMemoryBarriers are not allowed inside of
    // an active RenderPass.

    // Here our vertex and instance inputs need to match the same 0-based bindings they were
    // assigned in GrVkPipeline. That is, vertex first (if any) followed by instance.
    uint32_t binding = 0;

    if (vertexBuffer) {
        SkASSERT(vertexBuffer);
        SkASSERT(!vertexBuffer->isCPUBacked());
        SkASSERT(!vertexBuffer->isMapped());

        currCmdBuf->bindInputBuffer(fGpu, binding++,
                                    static_cast<const GrVkVertexBuffer*>(vertexBuffer));
    }

    if (instanceBuffer) {
        SkASSERT(instanceBuffer);
        SkASSERT(!instanceBuffer->isCPUBacked());
        SkASSERT(!instanceBuffer->isMapped());

        currCmdBuf->bindInputBuffer(fGpu, binding++,
                                    static_cast<const GrVkVertexBuffer*>(instanceBuffer));
    }

    if (indexBuffer) {
        SkASSERT(indexBuffer);
        SkASSERT(!indexBuffer->isMapped());
        SkASSERT(!indexBuffer->isCPUBacked());

        currCmdBuf->bindIndexBuffer(fGpu, static_cast<const GrVkIndexBuffer*>(indexBuffer));
    }
}

GrVkPipelineState* GrVkGpuRTCommandBuffer::prepareDrawState(
        const GrPrimitiveProcessor& primProc,
        const GrPipeline& pipeline,
        const GrPipeline::FixedDynamicState* fixedDynamicState,
        const GrPipeline::DynamicStateArrays* dynamicStateArrays,
        GrPrimitiveType primitiveType) {
    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];
    SkASSERT(cbInfo.fRenderPass);

    GrVkPipelineState* pipelineState =
        fGpu->resourceProvider().findOrCreateCompatiblePipelineState(pipeline,
                                                                     primProc,
                                                                     primitiveType,
                                                                     *cbInfo.fRenderPass);
    if (!pipelineState) {
        return pipelineState;
    }

    if (!cbInfo.fIsEmpty &&
        fLastPipelineState && fLastPipelineState != pipelineState &&
        fGpu->vkCaps().newCBOnPipelineChange()) {
        this->addAdditionalCommandBuffer();
    }
    fLastPipelineState = pipelineState;

    pipelineState->setData(fGpu, primProc, pipeline);

    pipelineState->bind(fGpu, cbInfo.currentCmdBuf());

    GrRenderTarget* rt = pipeline.renderTarget();

    if (!pipeline.isScissorEnabled()) {
        GrVkPipeline::SetDynamicScissorRectState(fGpu, cbInfo.currentCmdBuf(),
                                                 rt, pipeline.proxy()->origin(),
                                                 SkIRect::MakeWH(rt->width(), rt->height()));
    } else if (!dynamicStateArrays || !dynamicStateArrays->fScissorRects) {
        SkASSERT(fixedDynamicState);
        GrVkPipeline::SetDynamicScissorRectState(fGpu, cbInfo.currentCmdBuf(), rt,
                                                 pipeline.proxy()->origin(),
                                                 fixedDynamicState->fScissorRect);
    }
    GrVkPipeline::SetDynamicViewportState(fGpu, cbInfo.currentCmdBuf(), rt);
    GrVkPipeline::SetDynamicBlendConstantState(fGpu, cbInfo.currentCmdBuf(), rt->config(),
                                               pipeline.getXferProcessor());

    return pipelineState;
}

static void prepare_sampled_images(const GrResourceIOProcessor& processor,
                                   SkTArray<GrVkImage*>* sampledImages,
                                   GrVkGpu* gpu) {
    for (int i = 0; i < processor.numTextureSamplers(); ++i) {
        const GrResourceIOProcessor::TextureSampler& sampler = processor.textureSampler(i);
        GrVkTexture* vkTexture = static_cast<GrVkTexture*>(sampler.peekTexture());

        // We may need to resolve the texture first if it is also a render target
        GrVkRenderTarget* texRT = static_cast<GrVkRenderTarget*>(vkTexture->asRenderTarget());
        if (texRT) {
            gpu->onResolveRenderTarget(texRT);
        }

        // Check if we need to regenerate any mip maps
        if (GrSamplerState::Filter::kMipMap == sampler.samplerState().filter() &&
            (vkTexture->width() != 1 || vkTexture->height() != 1)) {
            SkASSERT(vkTexture->texturePriv().mipMapped() == GrMipMapped::kYes);
            if (vkTexture->texturePriv().mipMapsAreDirty()) {
                gpu->regenerateMipMapLevels(vkTexture);
            }
        }
        sampledImages->push_back(vkTexture);
    }
}

void GrVkGpuRTCommandBuffer::onDraw(const GrPrimitiveProcessor& primProc,
                                    const GrPipeline& pipeline,
                                    const GrPipeline::FixedDynamicState* fixedDynamicState,
                                    const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                    const GrMesh meshes[],
                                    int meshCount,
                                    const SkRect& bounds) {
    SkASSERT(pipeline.renderTarget() == fRenderTarget);

    if (!meshCount) {
        return;
    }

    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];

    prepare_sampled_images(primProc, &cbInfo.fSampledImages, fGpu);
    GrFragmentProcessor::Iter iter(pipeline);
    while (const GrFragmentProcessor* fp = iter.next()) {
        prepare_sampled_images(*fp, &cbInfo.fSampledImages, fGpu);
    }
    if (GrTexture* dstTexture = pipeline.peekDstTexture()) {
        cbInfo.fSampledImages.push_back(static_cast<GrVkTexture*>(dstTexture));
    }

    GrPrimitiveType primitiveType = meshes[0].primitiveType();
    GrVkPipelineState* pipelineState = this->prepareDrawState(primProc, pipeline, fixedDynamicState,
                                                              dynamicStateArrays, primitiveType);
    if (!pipelineState) {
        return;
    }

    bool dynamicScissor =
            pipeline.isScissorEnabled() && dynamicStateArrays && dynamicStateArrays->fScissorRects;

    for (int i = 0; i < meshCount; ++i) {
        const GrMesh& mesh = meshes[i];
        if (mesh.primitiveType() != primitiveType) {
            // Technically we don't have to call this here (since there is a safety check in
            // pipelineState:setData but this will allow for quicker freeing of resources if the
            // pipelineState sits in a cache for a while.
            pipelineState->freeTempResources(fGpu);
            SkDEBUGCODE(pipelineState = nullptr);
            primitiveType = mesh.primitiveType();
            pipelineState = this->prepareDrawState(primProc, pipeline, fixedDynamicState,
                                                   dynamicStateArrays, primitiveType);
            if (!pipelineState) {
                return;
            }
        }

        if (dynamicScissor) {
            GrVkPipeline::SetDynamicScissorRectState(fGpu, cbInfo.currentCmdBuf(), fRenderTarget,
                                                     pipeline.proxy()->origin(),
                                                     dynamicStateArrays->fScissorRects[i]);
        }

        SkASSERT(pipelineState);
        mesh.sendToGpu(this);
    }

    cbInfo.fBounds.join(bounds);
    cbInfo.fIsEmpty = false;

    // Technically we don't have to call this here (since there is a safety check in
    // pipelineState:setData but this will allow for quicker freeing of resources if the
    // pipelineState sits in a cache for a while.
    pipelineState->freeTempResources(fGpu);
}

void GrVkGpuRTCommandBuffer::sendInstancedMeshToGpu(GrPrimitiveType,
                                                    const GrBuffer* vertexBuffer,
                                                    int vertexCount,
                                                    int baseVertex,
                                                    const GrBuffer* instanceBuffer,
                                                    int instanceCount,
                                                    int baseInstance) {
    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];
    this->bindGeometry(nullptr, vertexBuffer, instanceBuffer);
    cbInfo.currentCmdBuf()->draw(fGpu, vertexCount, instanceCount, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrVkGpuRTCommandBuffer::sendIndexedInstancedMeshToGpu(GrPrimitiveType,
                                                           const GrBuffer* indexBuffer,
                                                           int indexCount,
                                                           int baseIndex,
                                                           const GrBuffer* vertexBuffer,
                                                           int baseVertex,
                                                           const GrBuffer* instanceBuffer,
                                                           int instanceCount,
                                                           int baseInstance,
                                                           GrPrimitiveRestart restart) {
    SkASSERT(restart == GrPrimitiveRestart::kNo);
    CommandBufferInfo& cbInfo = fCommandBufferInfos[fCurrentCmdInfo];
    this->bindGeometry(indexBuffer, vertexBuffer, instanceBuffer);
    cbInfo.currentCmdBuf()->drawIndexed(fGpu, indexCount, instanceCount,
                                        baseIndex, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}
