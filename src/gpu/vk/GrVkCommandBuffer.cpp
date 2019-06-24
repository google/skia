/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkCommandBuffer.h"

#include "include/core/SkRect.h"
#include "src/gpu/vk/GrVkCommandPool.h"
#include "src/gpu/vk/GrVkFramebuffer.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkIndexBuffer.h"
#include "src/gpu/vk/GrVkPipeline.h"
#include "src/gpu/vk/GrVkPipelineLayout.h"
#include "src/gpu/vk/GrVkPipelineState.h"
#include "src/gpu/vk/GrVkPipelineState.h"
#include "src/gpu/vk/GrVkRenderPass.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "src/gpu/vk/GrVkTransferBuffer.h"
#include "src/gpu/vk/GrVkUtil.h"
#include "src/gpu/vk/GrVkVertexBuffer.h"

void GrVkCommandBuffer::invalidateState() {
    for (auto& boundInputBuffer : fBoundInputBuffers) {
        boundInputBuffer = VK_NULL_HANDLE;
    }
    fBoundIndexBuffer = VK_NULL_HANDLE;

    memset(&fCachedViewport, 0, sizeof(VkViewport));
    fCachedViewport.width = - 1.0f; // Viewport must have a width greater than 0

    memset(&fCachedScissor, 0, sizeof(VkRect2D));
    fCachedScissor.offset.x = -1; // Scissor offset must be greater that 0 to be valid

    for (int i = 0; i < 4; ++i) {
        fCachedBlendConstant[i] = -1.0;
    }
}

void GrVkCommandBuffer::freeGPUData(GrVkGpu* gpu) const {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(!fIsActive);
    for (int i = 0; i < fTrackedResources.count(); ++i) {
        fTrackedResources[i]->notifyRemovedFromCommandBuffer();
        fTrackedResources[i]->unref(gpu);
    }

    for (int i = 0; i < fTrackedRecycledResources.count(); ++i) {
        fTrackedRecycledResources[i]->notifyRemovedFromCommandBuffer();
        fTrackedRecycledResources[i]->recycle(const_cast<GrVkGpu*>(gpu));
    }

    for (int i = 0; i < fTrackedRecordingResources.count(); ++i) {
        fTrackedRecordingResources[i]->notifyRemovedFromCommandBuffer();
        fTrackedRecordingResources[i]->unref(gpu);
    }

    if (!this->isWrapped()) {
        GR_VK_CALL(gpu->vkInterface(), FreeCommandBuffers(gpu->device(), fCmdPool->vkCommandPool(),
                                                          1, &fCmdBuffer));
    }

    this->onFreeGPUData(gpu);
}

void GrVkCommandBuffer::abandonGPUData() const {
    SkDEBUGCODE(fResourcesReleased = true;)
    for (int i = 0; i < fTrackedResources.count(); ++i) {
        fTrackedResources[i]->notifyRemovedFromCommandBuffer();
        fTrackedResources[i]->unrefAndAbandon();
    }

    for (int i = 0; i < fTrackedRecycledResources.count(); ++i) {
        fTrackedRecycledResources[i]->notifyRemovedFromCommandBuffer();
        // We don't recycle resources when abandoning them.
        fTrackedRecycledResources[i]->unrefAndAbandon();
    }

    for (int i = 0; i < fTrackedRecordingResources.count(); ++i) {
        fTrackedRecordingResources[i]->notifyRemovedFromCommandBuffer();
        fTrackedRecordingResources[i]->unrefAndAbandon();
    }

    this->onAbandonGPUData();
}

void GrVkCommandBuffer::releaseResources(GrVkGpu* gpu) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkDEBUGCODE(fResourcesReleased = true;)
    SkASSERT(!fIsActive);
    for (int i = 0; i < fTrackedResources.count(); ++i) {
        fTrackedResources[i]->notifyRemovedFromCommandBuffer();
        fTrackedResources[i]->unref(gpu);
    }
    for (int i = 0; i < fTrackedRecycledResources.count(); ++i) {
        fTrackedRecycledResources[i]->notifyRemovedFromCommandBuffer();
        fTrackedRecycledResources[i]->recycle(const_cast<GrVkGpu*>(gpu));
    }

    for (int i = 0; i < fTrackedRecordingResources.count(); ++i) {
        fTrackedRecordingResources[i]->notifyRemovedFromCommandBuffer();
        fTrackedRecordingResources[i]->unref(gpu);
    }

    if (++fNumResets > kNumRewindResetsBeforeFullReset) {
        fTrackedResources.reset();
        fTrackedRecycledResources.reset();
        fTrackedRecordingResources.reset();
        fTrackedResources.setReserve(kInitialTrackedResourcesCount);
        fTrackedRecycledResources.setReserve(kInitialTrackedResourcesCount);
        fTrackedRecordingResources.setReserve(kInitialTrackedResourcesCount);
        fNumResets = 0;
    } else {
        fTrackedResources.rewind();
        fTrackedRecycledResources.rewind();
        fTrackedRecordingResources.rewind();
    }

    this->invalidateState();

    this->onReleaseResources(gpu);
}

////////////////////////////////////////////////////////////////////////////////
// CommandBuffer commands
////////////////////////////////////////////////////////////////////////////////

void GrVkCommandBuffer::pipelineBarrier(const GrVkGpu* gpu,
                                        const GrVkResource* resource,
                                        VkPipelineStageFlags srcStageMask,
                                        VkPipelineStageFlags dstStageMask,
                                        bool byRegion,
                                        BarrierType barrierType,
                                        void* barrier) {
    SkASSERT(!this->isWrapped());
    SkASSERT(fIsActive);
    // For images we can have barriers inside of render passes but they require us to add more
    // support in subpasses which need self dependencies to have barriers inside them. Also, we can
    // never have buffer barriers inside of a render pass. For now we will just assert that we are
    // not in a render pass.
    SkASSERT(!fActiveRenderPass);

    if (barrierType == kBufferMemory_BarrierType) {
        const VkBufferMemoryBarrier* barrierPtr = reinterpret_cast<VkBufferMemoryBarrier*>(barrier);
        fBufferBarriers.push_back(*barrierPtr);
    } else {
        SkASSERT(barrierType == kImageMemory_BarrierType);
        const VkImageMemoryBarrier* barrierPtr = reinterpret_cast<VkImageMemoryBarrier*>(barrier);
        // We need to check if we are adding a pipeline barrier that covers part of the same
        // subresource range as a barrier that is already in current batch. If it does, then we must
        // submit the first batch because the vulkan spec does not define a specific ordering for
        // barriers submitted in the same batch.
        // TODO: Look if we can gain anything by merging barriers together instead of submitting
        // the old ones.
        for (int i = 0; i < fImageBarriers.count(); ++i) {
            VkImageMemoryBarrier& currentBarrier = fImageBarriers[i];
            if (barrierPtr->image == currentBarrier.image) {
                const VkImageSubresourceRange newRange = barrierPtr->subresourceRange;
                const VkImageSubresourceRange oldRange = currentBarrier.subresourceRange;
                SkASSERT(newRange.aspectMask == oldRange.aspectMask);
                SkASSERT(newRange.baseArrayLayer == oldRange.baseArrayLayer);
                SkASSERT(newRange.layerCount == oldRange.layerCount);
                uint32_t newStart = newRange.baseMipLevel;
                uint32_t newEnd = newRange.baseMipLevel + newRange.levelCount - 1;
                uint32_t oldStart = oldRange.baseMipLevel;
                uint32_t oldEnd = oldRange.baseMipLevel + oldRange.levelCount - 1;
                if (SkTMax(newStart, oldStart) <= SkTMin(newEnd, oldEnd)) {
                    this->submitPipelineBarriers(gpu);
                    break;
                }
            }
        }
        fImageBarriers.push_back(*barrierPtr);
    }
    fBarriersByRegion |= byRegion;

    fSrcStageMask = fSrcStageMask | srcStageMask;
    fDstStageMask = fDstStageMask | dstStageMask;

    fHasWork = true;
    if (resource) {
        this->addResource(resource);
    }
}

void GrVkCommandBuffer::submitPipelineBarriers(const GrVkGpu* gpu) {
    SkASSERT(fIsActive);

    // Currently we never submit a pipeline barrier without at least one memory barrier.
    if (fBufferBarriers.count() || fImageBarriers.count()) {
        // For images we can have barriers inside of render passes but they require us to add more
        // support in subpasses which need self dependencies to have barriers inside them. Also, we
        // can never have buffer barriers inside of a render pass. For now we will just assert that
        // we are not in a render pass.
        SkASSERT(!fActiveRenderPass);
        SkASSERT(!this->isWrapped());
        SkASSERT(fSrcStageMask && fDstStageMask);

        VkDependencyFlags dependencyFlags = fBarriersByRegion ? VK_DEPENDENCY_BY_REGION_BIT : 0;
        GR_VK_CALL(gpu->vkInterface(), CmdPipelineBarrier(
                fCmdBuffer, fSrcStageMask, fDstStageMask, dependencyFlags, 0, nullptr,
                fBufferBarriers.count(), fBufferBarriers.begin(),
                fImageBarriers.count(), fImageBarriers.begin()));
        fBufferBarriers.reset();
        fImageBarriers.reset();
        fBarriersByRegion = false;
        fSrcStageMask = 0;
        fDstStageMask = 0;
    }
    SkASSERT(!fBufferBarriers.count());
    SkASSERT(!fImageBarriers.count());
    SkASSERT(!fBarriersByRegion);
    SkASSERT(!fSrcStageMask);
    SkASSERT(!fDstStageMask);
}


void GrVkCommandBuffer::bindInputBuffer(GrVkGpu* gpu, uint32_t binding,
                                        const GrVkVertexBuffer* vbuffer) {
    VkBuffer vkBuffer = vbuffer->buffer();
    SkASSERT(VK_NULL_HANDLE != vkBuffer);
    SkASSERT(binding < kMaxInputBuffers);
    // TODO: once vbuffer->offset() no longer always returns 0, we will need to track the offset
    // to know if we can skip binding or not.
    if (vkBuffer != fBoundInputBuffers[binding]) {
        VkDeviceSize offset = vbuffer->offset();
        GR_VK_CALL(gpu->vkInterface(), CmdBindVertexBuffers(fCmdBuffer,
                                                            binding,
                                                            1,
                                                            &vkBuffer,
                                                            &offset));
        fBoundInputBuffers[binding] = vkBuffer;
        this->addResource(vbuffer->resource());
    }
}

void GrVkCommandBuffer::bindIndexBuffer(GrVkGpu* gpu, const GrVkIndexBuffer* ibuffer) {
    VkBuffer vkBuffer = ibuffer->buffer();
    SkASSERT(VK_NULL_HANDLE != vkBuffer);
    // TODO: once ibuffer->offset() no longer always returns 0, we will need to track the offset
    // to know if we can skip binding or not.
    if (vkBuffer != fBoundIndexBuffer) {
        GR_VK_CALL(gpu->vkInterface(), CmdBindIndexBuffer(fCmdBuffer,
                                                          vkBuffer,
                                                          ibuffer->offset(),
                                                          VK_INDEX_TYPE_UINT16));
        fBoundIndexBuffer = vkBuffer;
        this->addResource(ibuffer->resource());
    }
}

void GrVkCommandBuffer::clearAttachments(const GrVkGpu* gpu,
                                         int numAttachments,
                                         const VkClearAttachment* attachments,
                                         int numRects,
                                         const VkClearRect* clearRects) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    SkASSERT(numAttachments > 0);
    SkASSERT(numRects > 0);

    this->addingWork(gpu);

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
                                           GrVkPipelineLayout* layout,
                                           uint32_t firstSet,
                                           uint32_t setCount,
                                           const VkDescriptorSet* descriptorSets,
                                           uint32_t dynamicOffsetCount,
                                           const uint32_t* dynamicOffsets) {
    SkASSERT(fIsActive);
    GR_VK_CALL(gpu->vkInterface(), CmdBindDescriptorSets(fCmdBuffer,
                                                         VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                         layout->layout(),
                                                         firstSet,
                                                         setCount,
                                                         descriptorSets,
                                                         dynamicOffsetCount,
                                                         dynamicOffsets));
    this->addRecordingResource(layout);
}

void GrVkCommandBuffer::bindDescriptorSets(const GrVkGpu* gpu,
                                           const SkTArray<const GrVkRecycledResource*>& recycled,
                                           const SkTArray<const GrVkResource*>& resources,
                                           GrVkPipelineLayout* layout,
                                           uint32_t firstSet,
                                           uint32_t setCount,
                                           const VkDescriptorSet* descriptorSets,
                                           uint32_t dynamicOffsetCount,
                                           const uint32_t* dynamicOffsets) {
    SkASSERT(fIsActive);
    GR_VK_CALL(gpu->vkInterface(), CmdBindDescriptorSets(fCmdBuffer,
                                                         VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                         layout->layout(),
                                                         firstSet,
                                                         setCount,
                                                         descriptorSets,
                                                         dynamicOffsetCount,
                                                         dynamicOffsets));
    this->addRecordingResource(layout);
    for (int i = 0; i < recycled.count(); ++i) {
        this->addRecycledResource(recycled[i]);
    }
    for (int i = 0; i < resources.count(); ++i) {
        this->addResource(resources[i]);
    }
}

void GrVkCommandBuffer::bindPipeline(const GrVkGpu* gpu, const GrVkPipeline* pipeline) {
    SkASSERT(fIsActive);
    GR_VK_CALL(gpu->vkInterface(), CmdBindPipeline(fCmdBuffer,
                                                   VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                   pipeline->pipeline()));
    this->addResource(pipeline);
}

void GrVkCommandBuffer::drawIndexed(const GrVkGpu* gpu,
                                    uint32_t indexCount,
                                    uint32_t instanceCount,
                                    uint32_t firstIndex,
                                    int32_t vertexOffset,
                                    uint32_t firstInstance) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    this->addingWork(gpu);
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
                             uint32_t firstInstance) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    this->addingWork(gpu);
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

void GrVkCommandBuffer::addingWork(const GrVkGpu* gpu) {
    this->submitPipelineBarriers(gpu);
    fHasWork = true;
}

///////////////////////////////////////////////////////////////////////////////
// PrimaryCommandBuffer
////////////////////////////////////////////////////////////////////////////////
GrVkPrimaryCommandBuffer::~GrVkPrimaryCommandBuffer() {
    // Should have ended any render pass we're in the middle of
    SkASSERT(!fActiveRenderPass);
}

GrVkPrimaryCommandBuffer* GrVkPrimaryCommandBuffer::Create(const GrVkGpu* gpu,
                                                           GrVkCommandPool* cmdPool) {
    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        cmdPool->vkCommandPool(),                         // commandPool
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
    return new GrVkPrimaryCommandBuffer(cmdBuffer, cmdPool);
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

void GrVkPrimaryCommandBuffer::end(GrVkGpu* gpu) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);

    this->submitPipelineBarriers(gpu);

    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), EndCommandBuffer(fCmdBuffer));
    for (int i = 0; i < fTrackedRecordingResources.count(); ++i) {
        fTrackedRecordingResources[i]->unref(gpu);
    }
    fTrackedRecordingResources.rewind();
    this->invalidateState();
    fIsActive = false;
    fHasWork = false;
}

void GrVkPrimaryCommandBuffer::beginRenderPass(const GrVkGpu* gpu,
                                               const GrVkRenderPass* renderPass,
                                               const VkClearValue clearValues[],
                                               const GrVkRenderTarget& target,
                                               const SkIRect& bounds,
                                               bool forSecondaryCB) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    SkASSERT(renderPass->isCompatible(target));

    this->addingWork(gpu);

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
    beginInfo.clearValueCount = renderPass->clearValueCount();
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
    this->addingWork(gpu);
    GR_VK_CALL(gpu->vkInterface(), CmdEndRenderPass(fCmdBuffer));
    fActiveRenderPass = nullptr;
}

void GrVkPrimaryCommandBuffer::executeCommands(const GrVkGpu* gpu,
                                               GrVkSecondaryCommandBuffer* buffer) {
    // The Vulkan spec allows secondary command buffers to be executed on a primary command buffer
    // if the command pools both were created from were created with the same queue family. However,
    // we currently always create them from the same pool.
    SkASSERT(buffer->commandPool() == fCmdPool);
    SkASSERT(fIsActive);
    SkASSERT(!buffer->fIsActive);
    SkASSERT(fActiveRenderPass);
    SkASSERT(fActiveRenderPass->isCompatible(*buffer->fActiveRenderPass));

    this->addingWork(gpu);

    GR_VK_CALL(gpu->vkInterface(), CmdExecuteCommands(fCmdBuffer, 1, &buffer->fCmdBuffer));
    buffer->ref();
    fSecondaryCommandBuffers.push_back(buffer);
    // When executing a secondary command buffer all state (besides render pass state) becomes
    // invalidated and must be reset. This includes bound buffers, pipelines, dynamic state, etc.
    this->invalidateState();
}

static void submit_to_queue(const GrVkInterface* interface,
                            VkQueue queue,
                            VkFence fence,
                            uint32_t waitCount,
                            const VkSemaphore* waitSemaphores,
                            const VkPipelineStageFlags* waitStages,
                            uint32_t commandBufferCount,
                            const VkCommandBuffer* commandBuffers,
                            uint32_t signalCount,
                            const VkSemaphore* signalSemaphores,
                            GrProtected protectedContext) {
    VkProtectedSubmitInfo protectedSubmitInfo;
    if (protectedContext == GrProtected::kYes) {
        memset(&protectedSubmitInfo, 0, sizeof(VkProtectedSubmitInfo));
        protectedSubmitInfo.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.pNext = nullptr;
        protectedSubmitInfo.protectedSubmit = VK_TRUE;
    }

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = protectedContext == GrProtected::kYes ? &protectedSubmitInfo : nullptr;
    submitInfo.waitSemaphoreCount = waitCount;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = commandBuffers;
    submitInfo.signalSemaphoreCount = signalCount;
    submitInfo.pSignalSemaphores = signalSemaphores;
    GR_VK_CALL_ERRCHECK(interface, QueueSubmit(queue, 1, &submitInfo, fence));
}

void GrVkPrimaryCommandBuffer::submitToQueue(
        const GrVkGpu* gpu,
        VkQueue queue,
        GrVkGpu::SyncQueue sync,
        SkTArray<GrVkSemaphore::Resource*>& signalSemaphores,
        SkTArray<GrVkSemaphore::Resource*>& waitSemaphores) {
    SkASSERT(!fIsActive);

    VkResult err;
    if (VK_NULL_HANDLE == fSubmitFence) {
        VkFenceCreateInfo fenceInfo;
        memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        err = GR_VK_CALL(gpu->vkInterface(), CreateFence(gpu->device(), &fenceInfo, nullptr,
                                                         &fSubmitFence));
        SkASSERT(!err);
    } else {
        GR_VK_CALL(gpu->vkInterface(), ResetFences(gpu->device(), 1, &fSubmitFence));
    }

    int signalCount = signalSemaphores.count();
    int waitCount = waitSemaphores.count();

    if (0 == signalCount && 0 == waitCount) {
        // This command buffer has no dependent semaphores so we can simply just submit it to the
        // queue with no worries.
        submit_to_queue(gpu->vkInterface(), queue, fSubmitFence, 0, nullptr, nullptr, 1,
                        &fCmdBuffer, 0, nullptr,
                        gpu->protectedContext() ? GrProtected::kYes : GrProtected::kNo);
    } else {
        SkTArray<VkSemaphore> vkSignalSems(signalCount);
        for (int i = 0; i < signalCount; ++i) {
            if (signalSemaphores[i]->shouldSignal()) {
                this->addResource(signalSemaphores[i]);
                vkSignalSems.push_back(signalSemaphores[i]->semaphore());
            }
        }

        SkTArray<VkSemaphore> vkWaitSems(waitCount);
        SkTArray<VkPipelineStageFlags> vkWaitStages(waitCount);
        for (int i = 0; i < waitCount; ++i) {
            if (waitSemaphores[i]->shouldWait()) {
                this->addResource(waitSemaphores[i]);
                vkWaitSems.push_back(waitSemaphores[i]->semaphore());
                vkWaitStages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
            }
        }
        submit_to_queue(gpu->vkInterface(), queue, fSubmitFence, vkWaitSems.count(),
                        vkWaitSems.begin(), vkWaitStages.begin(), 1, &fCmdBuffer,
                        vkSignalSems.count(), vkSignalSems.begin(),
                        gpu->protectedContext() ? GrProtected::kYes : GrProtected::kNo);
        for (int i = 0; i < signalCount; ++i) {
            signalSemaphores[i]->markAsSignaled();
        }
        for (int i = 0; i < waitCount; ++i) {
            waitSemaphores[i]->markAsWaited();
        }
    }

    if (GrVkGpu::kForce_SyncQueue == sync) {
        err = GR_VK_CALL(gpu->vkInterface(),
                         WaitForFences(gpu->device(), 1, &fSubmitFence, true, UINT64_MAX));
        if (VK_TIMEOUT == err) {
            SkDebugf("Fence failed to signal: %d\n", err);
            SK_ABORT("failing");
        }
        SkASSERT(!err);

        fFinishedProcs.reset();

        // Destroy the fence
        GR_VK_CALL(gpu->vkInterface(), DestroyFence(gpu->device(), fSubmitFence, nullptr));
        fSubmitFence = VK_NULL_HANDLE;
    }
}

bool GrVkPrimaryCommandBuffer::finished(const GrVkGpu* gpu) {
    SkASSERT(!fIsActive);
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
            SK_ABORT("failing");
            break;
    }

    return false;
}

void GrVkPrimaryCommandBuffer::addFinishedProc(sk_sp<GrRefCntedCallback> finishedProc) {
    fFinishedProcs.push_back(std::move(finishedProc));
}

void GrVkPrimaryCommandBuffer::onReleaseResources(GrVkGpu* gpu) {
    for (int i = 0; i < fSecondaryCommandBuffers.count(); ++i) {
        fSecondaryCommandBuffers[i]->releaseResources(gpu);
    }
    fFinishedProcs.reset();
}

void GrVkPrimaryCommandBuffer::recycleSecondaryCommandBuffers() {
    for (int i = 0; i < fSecondaryCommandBuffers.count(); ++i) {
        SkASSERT(fSecondaryCommandBuffers[i]->commandPool() == fCmdPool);
        fCmdPool->recycleSecondaryCommandBuffer(fSecondaryCommandBuffers[i]);
    }
    fSecondaryCommandBuffers.reset();
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
    this->addingWork(gpu);
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
    this->addingWork(gpu);
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

void GrVkPrimaryCommandBuffer::blitImage(const GrVkGpu* gpu,
                                         const GrVkImage& srcImage,
                                         const GrVkImage& dstImage,
                                         uint32_t blitRegionCount,
                                         const VkImageBlit* blitRegions,
                                         VkFilter filter) {
    this->blitImage(gpu,
                    srcImage.resource(),
                    srcImage.image(),
                    srcImage.currentLayout(),
                    dstImage.resource(),
                    dstImage.image(),
                    dstImage.currentLayout(),
                    blitRegionCount,
                    blitRegions,
                    filter);
}


void GrVkPrimaryCommandBuffer::copyImageToBuffer(const GrVkGpu* gpu,
                                                 GrVkImage* srcImage,
                                                 VkImageLayout srcLayout,
                                                 GrVkTransferBuffer* dstBuffer,
                                                 uint32_t copyRegionCount,
                                                 const VkBufferImageCopy* copyRegions) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addingWork(gpu);
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
    this->addingWork(gpu);
    this->addResource(srcBuffer->resource());
    this->addResource(dstImage->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdCopyBufferToImage(fCmdBuffer,
                                                        srcBuffer->buffer(),
                                                        dstImage->image(),
                                                        dstLayout,
                                                        copyRegionCount,
                                                        copyRegions));
}


void GrVkPrimaryCommandBuffer::copyBuffer(GrVkGpu* gpu,
                                          GrVkBuffer* srcBuffer,
                                          GrVkBuffer* dstBuffer,
                                          uint32_t regionCount,
                                          const VkBufferCopy* regions) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addingWork(gpu);
#ifdef SK_DEBUG
    for (uint32_t i = 0; i < regionCount; ++i) {
        const VkBufferCopy& region = regions[i];
        SkASSERT(region.size > 0);
        SkASSERT(region.srcOffset < srcBuffer->size());
        SkASSERT(region.dstOffset < dstBuffer->size());
        SkASSERT(region.srcOffset + region.size <= srcBuffer->size());
        SkASSERT(region.dstOffset + region.size <= dstBuffer->size());
    }
#endif
    this->addResource(srcBuffer->resource());
    this->addResource(dstBuffer->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdCopyBuffer(fCmdBuffer,
                                                 srcBuffer->buffer(),
                                                 dstBuffer->buffer(),
                                                 regionCount,
                                                 regions));
}

void GrVkPrimaryCommandBuffer::updateBuffer(GrVkGpu* gpu,
                                            GrVkBuffer* dstBuffer,
                                            VkDeviceSize dstOffset,
                                            VkDeviceSize dataSize,
                                            const void* data) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    SkASSERT(0 == (dstOffset & 0x03));   // four byte aligned
    // TODO: handle larger transfer sizes
    SkASSERT(dataSize <= 65536);
    SkASSERT(0 == (dataSize & 0x03));    // four byte aligned
    this->addingWork(gpu);
    this->addResource(dstBuffer->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdUpdateBuffer(fCmdBuffer,
                                                   dstBuffer->buffer(),
                                                   dstOffset,
                                                   dataSize,
                                                   (const uint32_t*) data));
}

void GrVkPrimaryCommandBuffer::clearColorImage(const GrVkGpu* gpu,
                                               GrVkImage* image,
                                               const VkClearColorValue* color,
                                               uint32_t subRangeCount,
                                               const VkImageSubresourceRange* subRanges) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addingWork(gpu);
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
    this->addingWork(gpu);
    this->addResource(image->resource());
    GR_VK_CALL(gpu->vkInterface(), CmdClearDepthStencilImage(fCmdBuffer,
                                                             image->image(),
                                                             image->currentLayout(),
                                                             color,
                                                             subRangeCount,
                                                             subRanges));
}

void GrVkPrimaryCommandBuffer::resolveImage(GrVkGpu* gpu,
                                            const GrVkImage& srcImage,
                                            const GrVkImage& dstImage,
                                            uint32_t regionCount,
                                            const VkImageResolve* regions) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);

    this->addingWork(gpu);
    this->addResource(srcImage.resource());
    this->addResource(dstImage.resource());

    GR_VK_CALL(gpu->vkInterface(), CmdResolveImage(fCmdBuffer,
                                                   srcImage.image(),
                                                   srcImage.currentLayout(),
                                                   dstImage.image(),
                                                   dstImage.currentLayout(),
                                                   regionCount,
                                                   regions));
}

void GrVkPrimaryCommandBuffer::onFreeGPUData(GrVkGpu* gpu) const {
    SkASSERT(!fActiveRenderPass);
    // Destroy the fence, if any
    if (VK_NULL_HANDLE != fSubmitFence) {
        GR_VK_CALL(gpu->vkInterface(), DestroyFence(gpu->device(), fSubmitFence, nullptr));
    }
    for (GrVkSecondaryCommandBuffer* buffer : fSecondaryCommandBuffers) {
        buffer->unref(gpu);
    }
}

void GrVkPrimaryCommandBuffer::onAbandonGPUData() const {
    SkASSERT(!fActiveRenderPass);
    for (GrVkSecondaryCommandBuffer* buffer : fSecondaryCommandBuffers) {
        buffer->unrefAndAbandon();
    }
}

///////////////////////////////////////////////////////////////////////////////
// SecondaryCommandBuffer
////////////////////////////////////////////////////////////////////////////////

GrVkSecondaryCommandBuffer* GrVkSecondaryCommandBuffer::Create(const GrVkGpu* gpu,
                                                               GrVkCommandPool* cmdPool) {
    SkASSERT(cmdPool);
    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        cmdPool->vkCommandPool(),                         // commandPool
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
    return new GrVkSecondaryCommandBuffer(cmdBuffer, cmdPool);
}

GrVkSecondaryCommandBuffer* GrVkSecondaryCommandBuffer::Create(VkCommandBuffer cmdBuffer) {
    return new GrVkSecondaryCommandBuffer(cmdBuffer, nullptr);
}

void GrVkSecondaryCommandBuffer::begin(const GrVkGpu* gpu, const GrVkFramebuffer* framebuffer,
                                       const GrVkRenderPass* compatibleRenderPass) {
    SkASSERT(!fIsActive);
    SkASSERT(compatibleRenderPass);
    fActiveRenderPass = compatibleRenderPass;

    if (!this->isWrapped()) {
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
    }
    fIsActive = true;
}

void GrVkSecondaryCommandBuffer::end(GrVkGpu* gpu) {
    SkASSERT(fIsActive);
    if (!this->isWrapped()) {
        GR_VK_CALL_ERRCHECK(gpu->vkInterface(), EndCommandBuffer(fCmdBuffer));
    }
    this->invalidateState();
    fIsActive = false;
    fHasWork = false;
}
