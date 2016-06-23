/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkCommandBuffer.h"

#include "GrVkFramebuffer.h"
#include "GrVkImageView.h"
#include "GrVkPipeline.h"
#include "GrVkRenderPass.h"
#include "GrVkRenderTarget.h"
#include "GrVkPipelineState.h"
#include "GrVkTransferBuffer.h"
#include "GrVkUtil.h"
#include "SkRect.h"

void GrVkCommandBuffer::invalidateState() {
    fBoundVertexBuffer = VK_NULL_HANDLE;
    fBoundVertexBufferIsValid = false;
    fBoundIndexBuffer = VK_NULL_HANDLE;
    fBoundIndexBufferIsValid = false;

    memset(&fCachedViewport, 0, sizeof(VkViewport));
    fCachedViewport.width = - 1.0f; // Viewport must have a width greater than 0

    memset(&fCachedScissor, 0, sizeof(VkRect2D));
    fCachedScissor.offset.x = -1; // Scissor offset must be greater that 0 to be valid

    for (int i = 0; i < 4; ++i) {
        fCachedBlendConstant[i] = -1.0;
    }
}

void GrVkCommandBuffer::freeGPUData(const GrVkGpu* gpu) const {
    SkASSERT(!fIsActive);
    for (int i = 0; i < fTrackedResources.count(); ++i) {
        fTrackedResources[i]->unref(gpu);
    }

    GR_VK_CALL(gpu->vkInterface(), FreeCommandBuffers(gpu->device(), gpu->cmdPool(),
                                                      1, &fCmdBuffer));

    this->onFreeGPUData(gpu);
}

void GrVkCommandBuffer::abandonSubResources() const {
    for (int i = 0; i < fTrackedResources.count(); ++i) {
        fTrackedResources[i]->unrefAndAbandon();
    }
}

////////////////////////////////////////////////////////////////////////////////
// CommandBuffer commands
////////////////////////////////////////////////////////////////////////////////

void GrVkCommandBuffer::pipelineBarrier(const GrVkGpu* gpu,
                                        VkPipelineStageFlags srcStageMask,
                                        VkPipelineStageFlags dstStageMask,
                                        bool byRegion,
                                        BarrierType barrierType,
                                        void* barrier) const {
    SkASSERT(fIsActive);
    // For images we can have barriers inside of render passes but they require us to add more
    // support in subpasses which need self dependencies to have barriers inside them. Also, we can
    // never have buffer barriers inside of a render pass. For now we will just assert that we are
    // not in a render pass.
    SkASSERT(!fActiveRenderPass);
    VkDependencyFlags dependencyFlags = byRegion ? VK_DEPENDENCY_BY_REGION_BIT : 0;

    switch (barrierType) {
        case kMemory_BarrierType: {
            const VkMemoryBarrier* barrierPtr = reinterpret_cast<VkMemoryBarrier*>(barrier);
            GR_VK_CALL(gpu->vkInterface(), CmdPipelineBarrier(fCmdBuffer, srcStageMask,
                                                              dstStageMask, dependencyFlags,
                                                              1, barrierPtr,
                                                              0, nullptr,
                                                              0, nullptr));
            break;
        }

        case kBufferMemory_BarrierType: {
            const VkBufferMemoryBarrier* barrierPtr =
                                                 reinterpret_cast<VkBufferMemoryBarrier*>(barrier);
            GR_VK_CALL(gpu->vkInterface(), CmdPipelineBarrier(fCmdBuffer, srcStageMask,
                                                              dstStageMask, dependencyFlags,
                                                              0, nullptr,
                                                              1, barrierPtr,
                                                              0, nullptr));
            break;
        }

        case kImageMemory_BarrierType: {
            const VkImageMemoryBarrier* barrierPtr =
                                                  reinterpret_cast<VkImageMemoryBarrier*>(barrier);
            GR_VK_CALL(gpu->vkInterface(), CmdPipelineBarrier(fCmdBuffer, srcStageMask,
                                                              dstStageMask, dependencyFlags,
                                                              0, nullptr,
                                                              0, nullptr,
                                                              1, barrierPtr));
            break;
        }
    }

}

void GrVkCommandBuffer::clearAttachments(const GrVkGpu* gpu,
                                         int numAttachments,
                                         const VkClearAttachment* attachments,
                                         int numRects,
                                         const VkClearRect* clearRects) const {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    SkASSERT(numAttachments > 0);
    SkASSERT(numRects > 0);
#ifdef SK_DEBUG
    for (int i = 0; i < numAttachments; ++i) {
        if (attachments[i].aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) {
            uint32_t testIndex;
            SkAssertResult(fActiveRenderPass->colorAttachmentIndex(&testIndex));
            SkASSERT(testIndex == attachments[i].colorAttachment);
        }
    }
#endif
    GR_VK_CALL(gpu->vkInterface(), CmdClearAttachments(fCmdBuffer,
                                                       numAttachments,
                                                       attachments,
                                                       numRects,
                                                       clearRects));
}

void GrVkCommandBuffer::bindDescriptorSets(const GrVkGpu* gpu,
                                           GrVkPipelineState* pipelineState,
                                           VkPipelineLayout layout,
                                           uint32_t firstSet,
                                           uint32_t setCount,
                                           const VkDescriptorSet* descriptorSets,
                                           uint32_t dynamicOffsetCount,
                                           const uint32_t* dynamicOffsets) {
    SkASSERT(fIsActive);
    GR_VK_CALL(gpu->vkInterface(), CmdBindDescriptorSets(fCmdBuffer,
                                                         VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                         layout,
                                                         firstSet,
                                                         setCount,
                                                         descriptorSets,
                                                         dynamicOffsetCount,
                                                         dynamicOffsets));
    pipelineState->addUniformResources(*this);
}

void GrVkCommandBuffer::bindPipeline(const GrVkGpu* gpu, const GrVkPipeline* pipeline) {
    SkASSERT(fIsActive);
    GR_VK_CALL(gpu->vkInterface(), CmdBindPipeline(fCmdBuffer,
                                                   VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                   pipeline->pipeline()));
    addResource(pipeline);
}

void GrVkCommandBuffer::drawIndexed(const GrVkGpu* gpu,
                                    uint32_t indexCount,
                                    uint32_t instanceCount,
                                    uint32_t firstIndex,
                                    int32_t vertexOffset,
                                    uint32_t firstInstance) const {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    GR_VK_CALL(gpu->vkInterface(), CmdDrawIndexed(fCmdBuffer,
                                                  indexCount,
                                                  instanceCount,
                                                  firstIndex,
                                                  vertexOffset,
                                                  firstInstance));
}

void GrVkCommandBuffer::draw(const GrVkGpu* gpu,
                             uint32_t vertexCount,
                             uint32_t instanceCount,
                             uint32_t firstVertex,
                             uint32_t firstInstance) const {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    GR_VK_CALL(gpu->vkInterface(), CmdDraw(fCmdBuffer,
                                           vertexCount,
                                           instanceCount,
                                           firstVertex,
                                           firstInstance));
}

void GrVkCommandBuffer::setViewport(const GrVkGpu* gpu,
                                    uint32_t firstViewport,
                                    uint32_t viewportCount,
                                    const VkViewport* viewports) {
    SkASSERT(fIsActive);
    SkASSERT(1 == viewportCount);
    if (memcmp(viewports, &fCachedViewport, sizeof(VkViewport))) {
        GR_VK_CALL(gpu->vkInterface(), CmdSetViewport(fCmdBuffer,
                                                      firstViewport,
                                                      viewportCount,
                                                      viewports));
        fCachedViewport = viewports[0];
    }
}

void GrVkCommandBuffer::setScissor(const GrVkGpu* gpu,
                                   uint32_t firstScissor,
                                   uint32_t scissorCount,
                                   const VkRect2D* scissors) {
    SkASSERT(fIsActive);
    SkASSERT(1 == scissorCount);
    if (memcmp(scissors, &fCachedScissor, sizeof(VkRect2D))) {
        GR_VK_CALL(gpu->vkInterface(), CmdSetScissor(fCmdBuffer,
                                                     firstScissor,
                                                     scissorCount,
                                                     scissors));
        fCachedScissor = scissors[0];
    }
}

void GrVkCommandBuffer::setBlendConstants(const GrVkGpu* gpu,
                                          const float blendConstants[4]) {
    SkASSERT(fIsActive);
    if (memcmp(blendConstants, fCachedBlendConstant, 4 * sizeof(float))) {
        GR_VK_CALL(gpu->vkInterface(), CmdSetBlendConstants(fCmdBuffer, blendConstants));
        memcpy(fCachedBlendConstant, blendConstants, 4 * sizeof(float));
    }
}

///////////////////////////////////////////////////////////////////////////////
// PrimaryCommandBuffer
////////////////////////////////////////////////////////////////////////////////
GrVkPrimaryCommandBuffer::~GrVkPrimaryCommandBuffer() {
    // Should have ended any render pass we're in the middle of
    SkASSERT(!fActiveRenderPass);
}

GrVkPrimaryCommandBuffer* GrVkPrimaryCommandBuffer::Create(const GrVkGpu* gpu,
                                                           VkCommandPool cmdPool) {
    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        NULL,                                             // pNext
        cmdPool,                                          // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
        1                                                 // bufferCount
    };

    VkCommandBuffer cmdBuffer;
    VkResult err = GR_VK_CALL(gpu->vkInterface(), AllocateCommandBuffers(gpu->device(),
                                                                         &cmdInfo,
                                                                         &cmdBuffer));
    if (err) {
        return nullptr;
    }
    return new GrVkPrimaryCommandBuffer(cmdBuffer);
}

void GrVkPrimaryCommandBuffer::begin(const GrVkGpu* gpu) {
    SkASSERT(!fIsActive);
    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = nullptr;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = nullptr;

    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), BeginCommandBuffer(fCmdBuffer,
                                                               &cmdBufferBeginInfo));
    fIsActive = true;
}

void GrVkPrimaryCommandBuffer::end(const GrVkGpu* gpu) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), EndCommandBuffer(fCmdBuffer));
    this->invalidateState();
    fIsActive = false;
}

void GrVkPrimaryCommandBuffer::beginRenderPass(const GrVkGpu* gpu,
                                               const GrVkRenderPass* renderPass,
                                               uint32_t clearCount,
                                               const VkClearValue* clearValues,
                                               const GrVkRenderTarget& target,
                                               const SkIRect& bounds,
                                               bool forSecondaryCB) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    SkASSERT(renderPass->isCompatible(target));

    VkRenderPassBeginInfo beginInfo;
    VkRect2D renderArea;
    renderArea.offset = { bounds.fLeft , bounds.fTop };
    renderArea.extent = { (uint32_t)bounds.width(), (uint32_t)bounds.height() };

    memset(&beginInfo, 0, sizeof(VkRenderPassBeginInfo));
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = renderPass->vkRenderPass();
    beginInfo.framebuffer = target.framebuffer()->framebuffer();
    beginInfo.renderArea = renderArea;
    beginInfo.clearValueCount = clearCount;
    beginInfo.pClearValues = clearValues;

    VkSubpassContents contents = forSecondaryCB ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                                : VK_SUBPASS_CONTENTS_INLINE;

    GR_VK_CALL(gpu->vkInterface(), CmdBeginRenderPass(fCmdBuffer, &beginInfo, contents));
    fActiveRenderPass = renderPass;
    this->addResource(renderPass);
    target.addResources(*this);
}

void GrVkPrimaryCommandBuffer::endRenderPass(const GrVkGpu* gpu) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    GR_VK_CALL(gpu->vkInterface(), CmdEndRenderPass(fCmdBuffer));
    fActiveRenderPass = nullptr;
}

void GrVkPrimaryCommandBuffer::executeCommands(const GrVkGpu* gpu,
                                               const GrVkSecondaryCommandBuffer* buffer) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    SkASSERT(fActiveRenderPass->isCompatible(*buffer->fActiveRenderPass));

    GR_VK_CALL(gpu->vkInterface(), CmdExecuteCommands(fCmdBuffer, 1, &buffer->fCmdBuffer));
    this->addResource(buffer);
    // When executing a secondary command buffer all state (besides render pass state) becomes
    // invalidated and must be reset. This includes bound buffers, pipelines, dynamic state, etc.
    this->invalidateState();
}

void GrVkPrimaryCommandBuffer::submitToQueue(const GrVkGpu* gpu,
                                             VkQueue queue,
                                             GrVkGpu::SyncQueue sync) {
    SkASSERT(!fIsActive);

    VkResult err;
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    err = GR_VK_CALL(gpu->vkInterface(), CreateFence(gpu->device(), &fenceInfo, nullptr,
                                                     &fSubmitFence));
    SkASSERT(!err);

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = 0;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &fCmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), QueueSubmit(queue, 1, &submitInfo, fSubmitFence));

    if (GrVkGpu::kForce_SyncQueue == sync) {
        err = GR_VK_CALL(gpu->vkInterface(),
                         WaitForFences(gpu->device(), 1, &fSubmitFence, true, UINT64_MAX));
        if (VK_TIMEOUT == err) {
            SkDebugf("Fence failed to signal: %d\n", err);
            SkFAIL("failing");
        }
        SkASSERT(!err);

        // Destroy the fence
        GR_VK_CALL(gpu->vkInterface(), DestroyFence(gpu->device(), fSubmitFence, nullptr));
        fSubmitFence = VK_NULL_HANDLE;
    }
}

bool GrVkPrimaryCommandBuffer::finished(const GrVkGpu* gpu) const {
    if (VK_NULL_HANDLE == fSubmitFence) {
        return true;
    }

    VkResult err = GR_VK_CALL(gpu->vkInterface(), GetFenceStatus(gpu->device(), fSubmitFence));
    switch (err) {
        case VK_SUCCESS:
            return true;

        case VK_NOT_READY:
            return false;

        default:
            SkDebugf("Error getting fence status: %d\n", err);
            SkFAIL("failing");
            break;
    }

    return false;
}

void GrVkPrimaryCommandBuffer::copyImage(const GrVkGpu* gpu,
                                         GrVkImage* srcImage,
                                         VkImageLayout srcLayout,
                                         GrVkImage* dstImage,
                                         VkImageLayout dstLayout,
                                         uint32_t copyRegionCount,
                                         const VkImageCopy* copyRegions) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addResource(srcImage->resource());
    this->addResource(dstImage->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdCopyImage(fCmdBuffer,
                                                srcImage->image(),
                                                srcLayout,
                                                dstImage->image(),
                                                dstLayout,
                                                copyRegionCount,
                                                copyRegions));
}

void GrVkPrimaryCommandBuffer::blitImage(const GrVkGpu* gpu,
                                         const GrVkResource* srcResource,
                                         VkImage srcImage,
                                         VkImageLayout srcLayout,
                                         const GrVkResource* dstResource,
                                         VkImage dstImage,
                                         VkImageLayout dstLayout,
                                         uint32_t blitRegionCount,
                                         const VkImageBlit* blitRegions,
                                         VkFilter filter) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addResource(srcResource);
    this->addResource(dstResource);
    GR_VK_CALL(gpu->vkInterface(), CmdBlitImage(fCmdBuffer,
                                                srcImage,
                                                srcLayout,
                                                dstImage,
                                                dstLayout,
                                                blitRegionCount,
                                                blitRegions,
                                                filter));
}

void GrVkPrimaryCommandBuffer::copyImageToBuffer(const GrVkGpu* gpu,
                                                 GrVkImage* srcImage,
                                                 VkImageLayout srcLayout,
                                                 GrVkTransferBuffer* dstBuffer,
                                                 uint32_t copyRegionCount,
                                                 const VkBufferImageCopy* copyRegions) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addResource(srcImage->resource());
    this->addResource(dstBuffer->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdCopyImageToBuffer(fCmdBuffer,
                                                        srcImage->image(),
                                                        srcLayout,
                                                        dstBuffer->buffer(),
                                                        copyRegionCount,
                                                        copyRegions));
}

void GrVkPrimaryCommandBuffer::copyBufferToImage(const GrVkGpu* gpu,
                                                 GrVkTransferBuffer* srcBuffer,
                                                 GrVkImage* dstImage,
                                                 VkImageLayout dstLayout,
                                                 uint32_t copyRegionCount,
                                                 const VkBufferImageCopy* copyRegions) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addResource(srcBuffer->resource());
    this->addResource(dstImage->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdCopyBufferToImage(fCmdBuffer,
                                                        srcBuffer->buffer(),
                                                        dstImage->image(),
                                                        dstLayout,
                                                        copyRegionCount,
                                                        copyRegions));
}

void GrVkPrimaryCommandBuffer::clearColorImage(const GrVkGpu* gpu,
                                               GrVkImage* image,
                                               const VkClearColorValue* color,
                                               uint32_t subRangeCount,
                                               const VkImageSubresourceRange* subRanges) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addResource(image->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdClearColorImage(fCmdBuffer,
                                                      image->image(),
                                                      image->currentLayout(),
                                                      color,
                                                      subRangeCount,
                                                      subRanges));
}

void GrVkPrimaryCommandBuffer::clearDepthStencilImage(const GrVkGpu* gpu,
                                                      GrVkImage* image,
                                                      const VkClearDepthStencilValue* color,
                                                      uint32_t subRangeCount,
                                                      const VkImageSubresourceRange* subRanges) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addResource(image->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdClearDepthStencilImage(fCmdBuffer,
                                                             image->image(),
                                                             image->currentLayout(),
                                                             color,
                                                             subRangeCount,
                                                             subRanges));
}

void GrVkPrimaryCommandBuffer::onFreeGPUData(const GrVkGpu* gpu) const {
    SkASSERT(!fActiveRenderPass);
    // Destroy the fence, if any
    if (VK_NULL_HANDLE != fSubmitFence) {
        GR_VK_CALL(gpu->vkInterface(), DestroyFence(gpu->device(), fSubmitFence, nullptr));
    }
}

///////////////////////////////////////////////////////////////////////////////
// SecondaryCommandBuffer
////////////////////////////////////////////////////////////////////////////////

GrVkSecondaryCommandBuffer* GrVkSecondaryCommandBuffer::Create(
                                                       const GrVkGpu* gpu,
                                                       VkCommandPool cmdPool,
                                                       const GrVkRenderPass* compatibleRenderPass) {
    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        NULL,                                             // pNext
        cmdPool,                                          // commandPool
        VK_COMMAND_BUFFER_LEVEL_SECONDARY,                // level
        1                                                 // bufferCount
    };

    VkCommandBuffer cmdBuffer;
    VkResult err = GR_VK_CALL(gpu->vkInterface(), AllocateCommandBuffers(gpu->device(),
                                                                         &cmdInfo,
                                                                         &cmdBuffer));
    if (err) {
        return nullptr;
    }
    return new GrVkSecondaryCommandBuffer(cmdBuffer, compatibleRenderPass);
}


void GrVkSecondaryCommandBuffer::begin(const GrVkGpu* gpu, const GrVkFramebuffer* framebuffer) {
    SkASSERT(!fIsActive);
    SkASSERT(fActiveRenderPass);

    VkCommandBufferInheritanceInfo inheritanceInfo;
    memset(&inheritanceInfo, 0, sizeof(VkCommandBufferInheritanceInfo));
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext = nullptr;
    inheritanceInfo.renderPass = fActiveRenderPass->vkRenderPass();
    inheritanceInfo.subpass = 0; // Currently only using 1 subpass for each render pass
    inheritanceInfo.framebuffer = framebuffer ? framebuffer->framebuffer() : VK_NULL_HANDLE;
    inheritanceInfo.occlusionQueryEnable = false;
    inheritanceInfo.queryFlags = 0;
    inheritanceInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = nullptr;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
                               VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), BeginCommandBuffer(fCmdBuffer,
                                                               &cmdBufferBeginInfo));
    fIsActive = true;
}

void GrVkSecondaryCommandBuffer::end(const GrVkGpu* gpu) {
    SkASSERT(fIsActive);
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), EndCommandBuffer(fCmdBuffer));
    this->invalidateState();
    fIsActive = false;
}

