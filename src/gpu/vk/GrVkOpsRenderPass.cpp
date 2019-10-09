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
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
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

void GrVkOpsRenderPass::init(const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                             const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
                             const SkPMColor4f& clearColor) {

    VkAttachmentLoadOp loadOp;
    VkAttachmentStoreOp storeOp;
    get_vk_load_store_ops(colorInfo.fLoadOp, colorInfo.fStoreOp,
                          &loadOp, &storeOp);
    GrVkRenderPass::LoadStoreOps vkColorOps(loadOp, storeOp);

    get_vk_load_store_ops(stencilInfo.fLoadOp, stencilInfo.fStoreOp,
                          &loadOp, &storeOp);
    GrVkRenderPass::LoadStoreOps vkStencilOps(loadOp, storeOp);

    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    GrVkImage* targetImage = vkRT->msaaImage() ? vkRT->msaaImage() : vkRT;

    // Change layout of our render target so it can be used as the color attachment.
    // TODO: If we know that we will never be blending or loading the attachment we could drop the
    // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT.
    targetImage->setImageLayout(fGpu,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                false);

    // If we are using a stencil attachment we also need to update its layout
    if (GrStencilAttachment* stencil = fRenderTarget->renderTargetPriv().getStencilAttachment()) {
        GrVkStencilAttachment* vkStencil = (GrVkStencilAttachment*)stencil;
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

    const GrVkResourceProvider::CompatibleRPHandle& rpHandle = vkRT->compatibleRenderPassHandle();
    if (rpHandle.isValid()) {
        fCurrentRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                     vkColorOps,
                                                                     vkStencilOps);
    } else {
        fCurrentRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                     vkColorOps,
                                                                     vkStencilOps);
    }
    SkASSERT(fCurrentRenderPass);

    VkClearValue vkClearColor;
    vkClearColor.color.float32[0] = clearColor[0];
    vkClearColor.color.float32[1] = clearColor[1];
    vkClearColor.color.float32[2] = clearColor[2];
    vkClearColor.color.float32[3] = clearColor[3];

    if (!fGpu->vkCaps().preferPrimaryOverSecondaryCommandBuffers()) {
        fCurrentSecondaryCommandBuffer = fGpu->cmdPool()->findOrCreateSecondaryCommandBuffer(fGpu);
        fCurrentSecondaryCommandBuffer->begin(fGpu, vkRT->framebuffer(), fCurrentRenderPass);
    }

    fGpu->beginRenderPass(fCurrentRenderPass, &vkClearColor, vkRT, fOrigin, fBounds,
                          SkToBool(fCurrentSecondaryCommandBuffer));
}

void GrVkOpsRenderPass::initWrapped() {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    SkASSERT(vkRT->wrapsSecondaryCommandBuffer());
    fCurrentRenderPass = vkRT->externalRenderPass();
    SkASSERT(fCurrentRenderPass);
    fCurrentRenderPass->ref();

    fCurrentSecondaryCommandBuffer.reset(
            GrVkSecondaryCommandBuffer::Create(vkRT->getExternalSecondaryCommandBuffer()));
    fCurrentSecondaryCommandBuffer->begin(fGpu, nullptr, fCurrentRenderPass);
}

GrVkOpsRenderPass::~GrVkOpsRenderPass() {
    this->reset();
}

GrGpu* GrVkOpsRenderPass::gpu() { return fGpu; }

GrVkCommandBuffer* GrVkOpsRenderPass::currentCommandBuffer() {
    if (fCurrentSecondaryCommandBuffer) {
        return fCurrentSecondaryCommandBuffer.get();
    }
    return fGpu->currentCommandBuffer();
}

void GrVkOpsRenderPass::end() {
    if (fCurrentSecondaryCommandBuffer) {
        fCurrentSecondaryCommandBuffer->end(fGpu);
    }
}

void GrVkOpsRenderPass::submit() {
    if (!fRenderTarget) {
        return;
    }

    // We don't want to actually submit the secondary command buffer if it is wrapped.
    if (this->wrapsSecondaryCommandBuffer()) {
        return;
    }

    if (fCurrentSecondaryCommandBuffer) {
        fGpu->submitSecondaryCommandBuffer(std::move(fCurrentSecondaryCommandBuffer));
    }
    fGpu->endRenderPass(fRenderTarget, fOrigin, fBounds);
}

void GrVkOpsRenderPass::set(GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
                            const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
                            const SkTArray<GrTextureProxy*, true>& sampledProxies) {
    SkASSERT(!fRenderTarget);
    SkASSERT(fGpu == rt->getContext()->priv().getGpu());

#ifdef SK_DEBUG
    fIsActive = true;
#endif

    this->INHERITED::set(rt, origin);

    for (int i = 0; i < sampledProxies.count(); ++i) {
        if (sampledProxies[i]->isInstantiated()) {
            GrVkTexture* vkTex = static_cast<GrVkTexture*>(sampledProxies[i]->peekTexture());
            SkASSERT(vkTex);
            vkTex->setImageLayout(
                    fGpu, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, false);
        }
    }

    SkASSERT(bounds.isEmpty() || SkIRect::MakeWH(rt->width(), rt->height()).contains(bounds));
    fBounds = bounds;

    if (this->wrapsSecondaryCommandBuffer()) {
        this->initWrapped();
        return;
    }

    this->init(colorInfo, stencilInfo, colorInfo.fClearColor);
}

void GrVkOpsRenderPass::reset() {
    if (fCurrentSecondaryCommandBuffer) {
        fCurrentSecondaryCommandBuffer.release()->recycle(fGpu);
    }
    if (fCurrentRenderPass) {
        fCurrentRenderPass->unref(fGpu);
        fCurrentRenderPass = nullptr;
    }
    fCurrentCBIsEmpty = true;

    fRenderTarget = nullptr;

#ifdef SK_DEBUG
    fIsActive = false;
#endif
}

bool GrVkOpsRenderPass::wrapsSecondaryCommandBuffer() const {
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);
    return vkRT->wrapsSecondaryCommandBuffer();
}

////////////////////////////////////////////////////////////////////////////////

void GrVkOpsRenderPass::insertEventMarker(const char* msg) {
    // TODO: does Vulkan have a correlate?
}

void GrVkOpsRenderPass::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    SkASSERT(!clip.hasWindowRectangles());

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
    SkAssertResult(fCurrentRenderPass->stencilAttachmentIndex(&stencilIndex));

    VkClearAttachment attachment;
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    attachment.colorAttachment = 0; // this value shouldn't matter
    attachment.clearValue.depthStencil = vkStencilColor;

    this->currentCommandBuffer()->clearAttachments(fGpu, 1, &attachment, 1, &clearRect);
    fCurrentCBIsEmpty = false;
}

void GrVkOpsRenderPass::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    // parent class should never let us get here with no RT
    SkASSERT(!clip.hasWindowRectangles());

    VkClearColorValue vkColor = {{color.fR, color.fG, color.fB, color.fA}};

    // If we end up in a situation where we are calling clear without a scissior then in general it
    // means we missed an opportunity higher up the stack to set the load op to be a clear. However,
    // there are situations where higher up we couldn't discard the previous ops and set a clear
    // load op (e.g. if we needed to execute a wait op). Thus we also have the empty check here.
    // TODO: Make the waitOp a RenderTask instead so we can clear out the GrOpsTask for a clear. We
    // can then reenable this assert assuming we can't get messed up by a waitOp.
    //SkASSERT(!fCurrentCBIsEmpty || clip.scissorEnabled());

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
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(fRenderTarget);

    GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                            VK_ATTACHMENT_STORE_OP_STORE);
    GrVkRenderPass::LoadStoreOps vkStencilOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                              VK_ATTACHMENT_STORE_OP_STORE);

    const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
            vkRT->compatibleRenderPassHandle();
    SkASSERT(fCurrentRenderPass);
    fCurrentRenderPass->unref(fGpu);
    if (rpHandle.isValid()) {
        fCurrentRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                                     vkColorOps,
                                                                     vkStencilOps);
    } else {
        fCurrentRenderPass = fGpu->resourceProvider().findRenderPass(*vkRT,
                                                                     vkColorOps,
                                                                     vkStencilOps);
    }
    SkASSERT(fCurrentRenderPass);

    VkClearValue vkClearColor;
    memset(&vkClearColor, 0, sizeof(VkClearValue));

    if (!fGpu->vkCaps().preferPrimaryOverSecondaryCommandBuffers() ||
        mustUseSecondaryCommandBuffer) {
        fCurrentSecondaryCommandBuffer = fGpu->cmdPool()->findOrCreateSecondaryCommandBuffer(fGpu);
        fCurrentSecondaryCommandBuffer->begin(fGpu, vkRT->framebuffer(), fCurrentRenderPass);
    }

    // We use the same fBounds as the whole GrVkOpsRenderPass since we have no way of tracking the
    // bounds in GrOpsTask for parts before and after inline uploads separately.
    fGpu->beginRenderPass(fCurrentRenderPass, &vkClearColor, vkRT, fOrigin, fBounds,
                          SkToBool(fCurrentSecondaryCommandBuffer));
}

void GrVkOpsRenderPass::inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) {
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

void GrVkOpsRenderPass::bindGeometry(const GrGpuBuffer* indexBuffer,
                                          const GrGpuBuffer* vertexBuffer,
                                          const GrGpuBuffer* instanceBuffer) {
    GrVkCommandBuffer* currCmdBuf = this->currentCommandBuffer();
    // There is no need to put any memory barriers to make sure host writes have finished here.
    // When a command buffer is submitted to a queue, there is an implicit memory barrier that
    // occurs for all host writes. Additionally, BufferMemoryBarriers are not allowed inside of
    // an active RenderPass.

    // Here our vertex and instance inputs need to match the same 0-based bindings they were
    // assigned in GrVkPipeline. That is, vertex first (if any) followed by instance.
    uint32_t binding = 0;

    if (vertexBuffer) {
        SkASSERT(vertexBuffer);
        SkASSERT(!vertexBuffer->isMapped());

        currCmdBuf->bindInputBuffer(fGpu, binding++,
                                    static_cast<const GrVkVertexBuffer*>(vertexBuffer));
    }

    if (instanceBuffer) {
        SkASSERT(instanceBuffer);
        SkASSERT(!instanceBuffer->isMapped());

        currCmdBuf->bindInputBuffer(fGpu, binding++,
                                    static_cast<const GrVkVertexBuffer*>(instanceBuffer));
    }
    if (indexBuffer) {
        SkASSERT(indexBuffer);
        SkASSERT(!indexBuffer->isMapped());

        currCmdBuf->bindIndexBuffer(fGpu, static_cast<const GrVkIndexBuffer*>(indexBuffer));
    }
}

GrVkPipelineState* GrVkOpsRenderPass::prepareDrawState(
        const GrProgramInfo& programInfo,
        GrPrimitiveType primitiveType,
        const SkIRect& renderPassScissorRect) {
    GrVkCommandBuffer* currentCB = this->currentCommandBuffer();
    SkASSERT(fCurrentRenderPass);

    VkRenderPass compatibleRenderPass = fCurrentRenderPass->vkRenderPass();

    GrVkPipelineState* pipelineState =
        fGpu->resourceProvider().findOrCreateCompatiblePipelineState(fRenderTarget,
                                                                     programInfo,
                                                                     primitiveType,
                                                                     compatibleRenderPass);
    if (!pipelineState) {
        return pipelineState;
    }

    pipelineState->bindPipeline(fGpu, currentCB);

    // Both the 'programInfo' and this renderPass have an origin. Since they come from the
    // same place (i.e., the target renderTargetProxy) that had best agree.
    SkASSERT(programInfo.origin() == fOrigin);

    // TODO: just pass in GrProgramInfo
    pipelineState->setAndBindUniforms(fGpu, fRenderTarget, fOrigin,
                                      programInfo.primProc(), programInfo.pipeline(), currentCB);

    // Check whether we need to bind textures between each GrMesh. If not we can bind them all now.
    if (!programInfo.hasDynamicPrimProcTextures()) {
        // TODO: just pass in GrProgramInfo
        pipelineState->setAndBindTextures(fGpu, programInfo.primProc(), programInfo.pipeline(),
                                          programInfo.primProcProxies(), currentCB);
    }

    if (!programInfo.pipeline().isScissorEnabled()) {
        GrVkPipeline::SetDynamicScissorRectState(fGpu, currentCB, fRenderTarget, fOrigin,
                                                 renderPassScissorRect);
    } else if (!programInfo.hasDynamicScissors()) {
        SkASSERT(programInfo.hasFixedScissor());

        SkIRect combinedScissorRect;
        if (!combinedScissorRect.intersect(renderPassScissorRect, programInfo.fixedScissor())) {
            combinedScissorRect = SkIRect::MakeEmpty();
        }
        GrVkPipeline::SetDynamicScissorRectState(fGpu, currentCB, fRenderTarget, fOrigin,
                                                 combinedScissorRect);
    }
    GrVkPipeline::SetDynamicViewportState(fGpu, currentCB, fRenderTarget);
    GrVkPipeline::SetDynamicBlendConstantState(fGpu, currentCB,
                                               programInfo.pipeline().outputSwizzle(),
                                               programInfo.pipeline().getXferProcessor());

    return pipelineState;
}

#ifdef SK_DEBUG
void check_sampled_texture(GrTexture* tex, GrRenderTarget* rt, GrVkGpu* gpu) {
    SkASSERT(!tex->isProtected() || (rt->isProtected() && gpu->protectedContext()));
    GrVkTexture* vkTex = static_cast<GrVkTexture*>(tex);
    SkASSERT(vkTex->currentLayout() == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
#endif


void GrVkOpsRenderPass::onDraw(const GrProgramInfo& programInfo,
                               const GrMesh meshes[], int meshCount,
                               const SkRect& bounds) {

    SkASSERT(meshCount); // guaranteed by GrOpsRenderPass::draw

#ifdef SK_DEBUG
    if (programInfo.hasDynamicPrimProcTextures()) {
        for (int m = 0; m < meshCount; ++m) {
            auto dynamicPrimProcTextures = programInfo.dynamicPrimProcTextures(m);

            for (int s = 0; s < programInfo.primProc().numTextureSamplers(); ++s) {
                auto texture = dynamicPrimProcTextures[s]->peekTexture();
                check_sampled_texture(texture, fRenderTarget, fGpu);
            }
        }
    } else if (programInfo.hasFixedPrimProcTextures()) {
        auto fixedPrimProcTextures = programInfo.fixedPrimProcTextures();

        for (int s = 0; s < programInfo.primProc().numTextureSamplers(); ++s) {
            auto texture = fixedPrimProcTextures[s]->peekTexture();
            check_sampled_texture(texture, fRenderTarget, fGpu);
        }
    }

    GrFragmentProcessor::Iter iter(programInfo.pipeline());
    while (const GrFragmentProcessor* fp = iter.next()) {
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            const GrFragmentProcessor::TextureSampler& sampler = fp->textureSampler(i);
            check_sampled_texture(sampler.peekTexture(), fRenderTarget, fGpu);
        }
    }
    if (GrTexture* dstTexture = programInfo.pipeline().peekDstTexture()) {
        check_sampled_texture(dstTexture, fRenderTarget, fGpu);
    }

    // Both the 'programInfo' and this renderPass have an origin. Since they come from the
    // same place (i.e., the target renderTargetProxy) that had best agree.
    SkASSERT(programInfo.origin() == fOrigin);
#endif

    SkRect scissorRect = SkRect::Make(fBounds);
    SkIRect renderPassScissorRect = SkIRect::MakeEmpty();
    if (scissorRect.intersect(bounds)) {
        scissorRect.roundOut(&renderPassScissorRect);
    }

    GrPrimitiveType primitiveType = meshes[0].primitiveType();
    GrVkPipelineState* pipelineState = this->prepareDrawState(programInfo, primitiveType,
                                                              renderPassScissorRect);
    if (!pipelineState) {
        return;
    }

    bool hasDynamicScissors = programInfo.hasDynamicScissors();
    bool hasDynamicTextures = programInfo.hasDynamicPrimProcTextures();

    for (int i = 0; i < meshCount; ++i) {
        const GrMesh& mesh = meshes[i];
        if (mesh.primitiveType() != primitiveType) {
            SkDEBUGCODE(pipelineState = nullptr);
            primitiveType = mesh.primitiveType();
            pipelineState = this->prepareDrawState(programInfo, primitiveType,
                                                   renderPassScissorRect);
            if (!pipelineState) {
                return;
            }
        }

        if (hasDynamicScissors) {
            SkIRect combinedScissorRect;
            if (!combinedScissorRect.intersect(renderPassScissorRect,
                                               programInfo.dynamicScissor(i))) {
                combinedScissorRect = SkIRect::MakeEmpty();
            }
            GrVkPipeline::SetDynamicScissorRectState(fGpu, this->currentCommandBuffer(),
                                                     fRenderTarget, fOrigin,
                                                     combinedScissorRect);
        }
        if (hasDynamicTextures) {
            auto meshProxies = programInfo.dynamicPrimProcTextures(i);
            pipelineState->setAndBindTextures(fGpu, programInfo.primProc(), programInfo.pipeline(),
                                              meshProxies,
                                              this->currentCommandBuffer());
        }
        SkASSERT(pipelineState);
        mesh.sendToGpu(this);
    }

    fCurrentCBIsEmpty = false;
}

void GrVkOpsRenderPass::sendInstancedMeshToGpu(GrPrimitiveType,
                                               const GrBuffer* vertexBuffer,
                                               int vertexCount,
                                               int baseVertex,
                                               const GrBuffer* instanceBuffer,
                                               int instanceCount,
                                               int baseInstance) {
    SkASSERT(!vertexBuffer || !vertexBuffer->isCpuBuffer());
    SkASSERT(!instanceBuffer || !instanceBuffer->isCpuBuffer());
    auto gpuVertexBuffer = static_cast<const GrGpuBuffer*>(vertexBuffer);
    auto gpuInstanceBuffer = static_cast<const GrGpuBuffer*>(instanceBuffer);
    this->bindGeometry(nullptr, gpuVertexBuffer, gpuInstanceBuffer);
    this->currentCommandBuffer()->draw(fGpu, vertexCount, instanceCount, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrVkOpsRenderPass::sendIndexedInstancedMeshToGpu(GrPrimitiveType,
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
    SkASSERT(!vertexBuffer || !vertexBuffer->isCpuBuffer());
    SkASSERT(!instanceBuffer || !instanceBuffer->isCpuBuffer());
    SkASSERT(!indexBuffer->isCpuBuffer());
    auto gpuIndexxBuffer = static_cast<const GrGpuBuffer*>(indexBuffer);
    auto gpuVertexBuffer = static_cast<const GrGpuBuffer*>(vertexBuffer);
    auto gpuInstanceBuffer = static_cast<const GrGpuBuffer*>(instanceBuffer);
    this->bindGeometry(gpuIndexxBuffer, gpuVertexBuffer, gpuInstanceBuffer);
    this->currentCommandBuffer()->drawIndexed(fGpu, indexCount, instanceCount,
                                              baseIndex, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

////////////////////////////////////////////////////////////////////////////////

void GrVkOpsRenderPass::executeDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable) {
    GrVkRenderTarget* target = static_cast<GrVkRenderTarget*>(fRenderTarget);

    GrVkImage* targetImage = target->msaaImage() ? target->msaaImage() : target;

    VkRect2D bounds;
    bounds.offset = { 0, 0 };
    bounds.extent = { 0, 0 };

    if (!fCurrentSecondaryCommandBuffer) {
        fGpu->endRenderPass(fRenderTarget, fOrigin, fBounds);
        this->addAdditionalRenderPass(true);
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

