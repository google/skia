/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkGpuCommandBuffer.h"

#include "GrFixedClip.h"
#include "GrMesh.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrTextureAccess.h"
#include "GrTexturePriv.h"
#include "GrVkCommandBuffer.h"
#include "GrVkGpu.h"
#include "GrVkPipeline.h"
#include "GrVkRenderPass.h"
#include "GrVkRenderTarget.h"
#include "GrVkResourceProvider.h"
#include "GrVkTexture.h"

void get_vk_load_store_ops(const GrGpuCommandBuffer::LoadAndStoreInfo& info,
                           VkAttachmentLoadOp* loadOp, VkAttachmentStoreOp* storeOp) {
    switch (info.fLoadOp) {
        case GrGpuCommandBuffer::LoadOp::kLoad:
            *loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case GrGpuCommandBuffer::LoadOp::kClear:
            *loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        case GrGpuCommandBuffer::LoadOp::kDiscard:
            *loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        default:
            SK_ABORT("Invalid LoadOp");
            *loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    }

    switch (info.fStoreOp) {
        case GrGpuCommandBuffer::StoreOp::kStore:
            *storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case GrGpuCommandBuffer::StoreOp::kDiscard:
            *storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        default:
            SK_ABORT("Invalid StoreOp");
            *storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }
}

GrVkGpuCommandBuffer::GrVkGpuCommandBuffer(GrVkGpu* gpu,
                                           GrVkRenderTarget* target,
                                           const LoadAndStoreInfo& colorInfo,
                                           const LoadAndStoreInfo& stencilInfo)
    : fGpu(gpu)
    , fRenderTarget(target)
    , fIsEmpty(true)
    , fStartsWithClear(false) {
    VkAttachmentLoadOp vkLoadOp;
    VkAttachmentStoreOp vkStoreOp;

    get_vk_load_store_ops(colorInfo, &vkLoadOp, &vkStoreOp);
    GrVkRenderPass::LoadStoreOps vkColorOps(vkLoadOp, vkStoreOp);

    get_vk_load_store_ops(stencilInfo, &vkLoadOp, &vkStoreOp);
    GrVkRenderPass::LoadStoreOps vkStencilOps(vkLoadOp, vkStoreOp);

    const GrVkResourceProvider::CompatibleRPHandle& rpHandle = target->compatibleRenderPassHandle();
    if (rpHandle.isValid()) {
        fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                              vkColorOps,
                                                              vkStencilOps);
    } else {
        fRenderPass = fGpu->resourceProvider().findRenderPass(*target,
                                                              vkColorOps,
                                                              vkStencilOps);
    }

    GrColorToRGBAFloat(colorInfo.fClearColor, fColorClearValue.color.float32);

    fCommandBuffer = gpu->resourceProvider().findOrCreateSecondaryCommandBuffer();
    fCommandBuffer->begin(gpu, target->framebuffer(), fRenderPass);
}

GrVkGpuCommandBuffer::~GrVkGpuCommandBuffer() {
    fCommandBuffer->unref(fGpu);
    fRenderPass->unref(fGpu);
}

GrGpu* GrVkGpuCommandBuffer::gpu() { return fGpu; }

void GrVkGpuCommandBuffer::end() {
    fCommandBuffer->end(fGpu);
}

void GrVkGpuCommandBuffer::onSubmit(const SkIRect& bounds) {
    // TODO: We can't add this optimization yet since many things create a scratch texture which
    // adds the discard immediately, but then don't draw to it right away. This causes the discard
    // to be ignored and we get yelled at for loading uninitialized data. However, once MDP lands,
    // the discard will get reordered with the rest of the draw commands and we can re-enable this.
#if 0
    if (fIsEmpty && !fStartsWithClear) {
        // We have sumbitted no actual draw commands to the command buffer and we are not using
        // the render pass to do a clear so there is no need to submit anything.
        return;
    }
#endif

    // Change layout of our render target so it can be used as the color attachment. Currently
    // we don't attach the resolve to the framebuffer so no need to change its layout.
    GrVkImage* targetImage = fRenderTarget->msaaImage() ? fRenderTarget->msaaImage()
                                                        : fRenderTarget;

    // Change layout of our render target so it can be used as the color attachment
    targetImage->setImageLayout(fGpu,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                false);

    // If we are using a stencil attachment we also need to update its layout
    if (GrStencilAttachment* stencil = fRenderTarget->renderTargetPriv().getStencilAttachment()) {
        GrVkStencilAttachment* vkStencil = (GrVkStencilAttachment*)stencil;
        vkStencil->setImageLayout(fGpu,
                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                  VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                  false);
    }

    fGpu->submitSecondaryCommandBuffer(fCommandBuffer, fRenderPass, &fColorClearValue,
                                       fRenderTarget, bounds);
}

void GrVkGpuCommandBuffer::discard(GrRenderTarget* target) {
    if (fIsEmpty) {
        // We will change the render pass to do a clear load instead
        GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VK_ATTACHMENT_STORE_OP_STORE);
        GrVkRenderPass::LoadStoreOps vkStencilOps(VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                  VK_ATTACHMENT_STORE_OP_STORE);

        const GrVkRenderPass* oldRP = fRenderPass;

        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(target);
        const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
            vkRT->compatibleRenderPassHandle();
        if (rpHandle.isValid()) {
            fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                  vkColorOps,
                                                                  vkStencilOps);
        } else {
            fRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                  vkColorOps,
                                                                  vkStencilOps);
        }

        SkASSERT(fRenderPass->isCompatible(*oldRP));
        oldRP->unref(fGpu);
        fStartsWithClear = false;
    }
}

void GrVkGpuCommandBuffer::onClearStencilClip(GrRenderTarget* target,
                                              const GrFixedClip& clip,
                                              bool insideStencilMask) {
    SkASSERT(target);
    SkASSERT(!clip.hasWindowRectangles());

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(target);
    GrStencilAttachment* sb = target->renderTargetPriv().getStencilAttachment();
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
        vkRect.setXYWH(0, 0, vkRT->width(), vkRT->height());
    } else if (kBottomLeft_GrSurfaceOrigin != vkRT->origin()) {
        vkRect = clip.scissorRect();
    } else {
        const SkIRect& scissor = clip.scissorRect();
        vkRect.setLTRB(scissor.fLeft, vkRT->height() - scissor.fBottom,
                       scissor.fRight, vkRT->height() - scissor.fTop);
    }

    clearRect.rect.offset = { vkRect.fLeft, vkRect.fTop };
    clearRect.rect.extent = { (uint32_t)vkRect.width(), (uint32_t)vkRect.height() };

    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    uint32_t stencilIndex;
    SkAssertResult(fRenderPass->stencilAttachmentIndex(&stencilIndex));

    VkClearAttachment attachment;
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    attachment.colorAttachment = 0; // this value shouldn't matter
    attachment.clearValue.depthStencil = vkStencilColor;

    fCommandBuffer->clearAttachments(fGpu, 1, &attachment, 1, &clearRect);
    fIsEmpty = false;
}

void GrVkGpuCommandBuffer::onClear(GrRenderTarget* target, const GrFixedClip& clip, GrColor color) {
    // parent class should never let us get here with no RT
    SkASSERT(target);
    SkASSERT(!clip.hasWindowRectangles());

    VkClearColorValue vkColor;
    GrColorToRGBAFloat(color, vkColor.float32);

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(target);

    if (fIsEmpty && !clip.scissorEnabled()) {
        // We will change the render pass to do a clear load instead
        GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                VK_ATTACHMENT_STORE_OP_STORE);
        GrVkRenderPass::LoadStoreOps vkStencilOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                                  VK_ATTACHMENT_STORE_OP_STORE);

        const GrVkRenderPass* oldRP = fRenderPass;

        const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
            vkRT->compatibleRenderPassHandle();
        if (rpHandle.isValid()) {
            fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                  vkColorOps,
                                                                  vkStencilOps);
        } else {
            fRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                  vkColorOps,
                                                                  vkStencilOps);
        }

        SkASSERT(fRenderPass->isCompatible(*oldRP));
        oldRP->unref(fGpu);

        GrColorToRGBAFloat(color, fColorClearValue.color.float32);
        fStartsWithClear = true;
        return;
    }

    // We always do a sub rect clear with clearAttachments since we are inside a render pass
    VkClearRect clearRect;
    // Flip rect if necessary
    SkIRect vkRect;
    if (!clip.scissorEnabled()) {
        vkRect.setXYWH(0, 0, vkRT->width(), vkRT->height());
    } else if (kBottomLeft_GrSurfaceOrigin != vkRT->origin()) {
        vkRect = clip.scissorRect();
    } else {
        const SkIRect& scissor = clip.scissorRect();
        vkRect.setLTRB(scissor.fLeft, vkRT->height() - scissor.fBottom,
                       scissor.fRight, vkRT->height() - scissor.fTop);
    }
    clearRect.rect.offset = { vkRect.fLeft, vkRect.fTop };
    clearRect.rect.extent = { (uint32_t)vkRect.width(), (uint32_t)vkRect.height() };
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    uint32_t colorIndex;
    SkAssertResult(fRenderPass->colorAttachmentIndex(&colorIndex));

    VkClearAttachment attachment;
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = colorIndex;
    attachment.clearValue.color = vkColor;

    fCommandBuffer->clearAttachments(fGpu, 1, &attachment, 1, &clearRect);
    fIsEmpty = false;
    return;
}

////////////////////////////////////////////////////////////////////////////////

void GrVkGpuCommandBuffer::bindGeometry(const GrPrimitiveProcessor& primProc,
                                        const GrNonInstancedMesh& mesh) {
    // There is no need to put any memory barriers to make sure host writes have finished here.
    // When a command buffer is submitted to a queue, there is an implicit memory barrier that
    // occurs for all host writes. Additionally, BufferMemoryBarriers are not allowed inside of
    // an active RenderPass.
    SkASSERT(!mesh.vertexBuffer()->isCPUBacked());
    GrVkVertexBuffer* vbuf;
    vbuf = (GrVkVertexBuffer*)mesh.vertexBuffer();
    SkASSERT(vbuf);
    SkASSERT(!vbuf->isMapped());

    fCommandBuffer->bindVertexBuffer(fGpu, vbuf);

    if (mesh.isIndexed()) {
        SkASSERT(!mesh.indexBuffer()->isCPUBacked());
        GrVkIndexBuffer* ibuf = (GrVkIndexBuffer*)mesh.indexBuffer();
        SkASSERT(ibuf);
        SkASSERT(!ibuf->isMapped());

        fCommandBuffer->bindIndexBuffer(fGpu, ibuf);
    }
}

sk_sp<GrVkPipelineState> GrVkGpuCommandBuffer::prepareDrawState(
                                                               const GrPipeline& pipeline,
                                                               const GrPrimitiveProcessor& primProc,
                                                               GrPrimitiveType primitiveType,
                                                               const GrVkRenderPass& renderPass) {
    sk_sp<GrVkPipelineState> pipelineState =
        fGpu->resourceProvider().findOrCreateCompatiblePipelineState(pipeline,
                                                                     primProc,
                                                                     primitiveType,
                                                                     renderPass);
    if (!pipelineState) {
        return pipelineState;
    }

    pipelineState->setData(fGpu, primProc, pipeline);

    pipelineState->bind(fGpu, fCommandBuffer);

    GrVkPipeline::SetDynamicState(fGpu, fCommandBuffer, pipeline);

    return pipelineState;
}

static void prepare_sampled_images(const GrProcessor& processor, GrVkGpu* gpu) {
    for (int i = 0; i < processor.numTextures(); ++i) {
        const GrTextureAccess& texAccess = processor.textureAccess(i);
        GrVkTexture* vkTexture = static_cast<GrVkTexture*>(processor.texture(i));
        SkASSERT(vkTexture);

        // We may need to resolve the texture first if it is also a render target
        GrVkRenderTarget* texRT = static_cast<GrVkRenderTarget*>(vkTexture->asRenderTarget());
        if (texRT) {
            gpu->onResolveRenderTarget(texRT);
        }

        const GrTextureParams& params = texAccess.getParams();
        // Check if we need to regenerate any mip maps
        if (GrTextureParams::kMipMap_FilterMode == params.filterMode()) {
            if (vkTexture->texturePriv().mipMapsAreDirty()) {
                gpu->generateMipmap(vkTexture);
                vkTexture->texturePriv().dirtyMipMaps(false);
            }
        }

        // TODO: If we ever decide to create the secondary command buffers ahead of time before we
        // are actually going to submit them, we will need to track the sampled images and delay
        // adding the layout change/barrier until we are ready to submit.
        vkTexture->setImageLayout(gpu,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_ACCESS_SHADER_READ_BIT,
                                  VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                  false);
    }
}

void GrVkGpuCommandBuffer::onDraw(const GrPipeline& pipeline,
                                  const GrPrimitiveProcessor& primProc,
                                  const GrMesh* meshes,
                                  int meshCount) {
    if (!meshCount) {
        return;
    }
    GrRenderTarget* rt = pipeline.getRenderTarget();
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);
    const GrVkRenderPass* renderPass = vkRT->simpleRenderPass();
    SkASSERT(renderPass);

    prepare_sampled_images(primProc, fGpu);
    GrFragmentProcessor::Iter iter(pipeline);
    while (const GrFragmentProcessor* fp = iter.next()) {
        prepare_sampled_images(*fp, fGpu);
    }
    prepare_sampled_images(pipeline.getXferProcessor(), fGpu);

    GrPrimitiveType primitiveType = meshes[0].primitiveType();
    sk_sp<GrVkPipelineState> pipelineState = this->prepareDrawState(pipeline,
                                                                    primProc,
                                                                    primitiveType,
                                                                    *renderPass);
    if (!pipelineState) {
        return;
    }

    for (int i = 0; i < meshCount; ++i) {
        const GrMesh& mesh = meshes[i];
        GrMesh::Iterator iter;
        const GrNonInstancedMesh* nonIdxMesh = iter.init(mesh);
        do {
            if (nonIdxMesh->primitiveType() != primitiveType) {
                // Technically we don't have to call this here (since there is a safety check in
                // pipelineState:setData but this will allow for quicker freeing of resources if the
                // pipelineState sits in a cache for a while.
                pipelineState->freeTempResources(fGpu);
                SkDEBUGCODE(pipelineState = nullptr);
                primitiveType = nonIdxMesh->primitiveType();
                pipelineState = this->prepareDrawState(pipeline,
                                                       primProc,
                                                       primitiveType,
                                                       *renderPass);
                if (!pipelineState) {
                    return;
                }
            }
            SkASSERT(pipelineState);
            this->bindGeometry(primProc, *nonIdxMesh);

            if (nonIdxMesh->isIndexed()) {
                fCommandBuffer->drawIndexed(fGpu,
                                            nonIdxMesh->indexCount(),
                                            1,
                                            nonIdxMesh->startIndex(),
                                            nonIdxMesh->startVertex(),
                                            0);
            } else {
                fCommandBuffer->draw(fGpu,
                                     nonIdxMesh->vertexCount(),
                                     1,
                                     nonIdxMesh->startVertex(),
                                     0);
            }
            fIsEmpty = false;

            fGpu->stats()->incNumDraws();
        } while ((nonIdxMesh = iter.next()));
    }

    // Technically we don't have to call this here (since there is a safety check in
    // pipelineState:setData but this will allow for quicker freeing of resources if the
    // pipelineState sits in a cache for a while.
    pipelineState->freeTempResources(fGpu);
}

