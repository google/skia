/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkCommandBuffer.h"

#include "include/core/SkRect.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/vk/GrVkBuffer.h"
#include "src/gpu/ganesh/vk/GrVkCommandPool.h"
#include "src/gpu/ganesh/vk/GrVkFramebuffer.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"
#include "src/gpu/ganesh/vk/GrVkImageView.h"
#include "src/gpu/ganesh/vk/GrVkPipeline.h"
#include "src/gpu/ganesh/vk/GrVkPipelineState.h"
#include "src/gpu/ganesh/vk/GrVkRenderPass.h"
#include "src/gpu/ganesh/vk/GrVkRenderTarget.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"

using namespace skia_private;

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

void GrVkCommandBuffer::freeGPUData(const GrGpu* gpu, VkCommandPool cmdPool) const {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(!fIsActive);
    SkASSERT(fTrackedResources.empty());
    SkASSERT(fTrackedRecycledResources.empty());
    SkASSERT(fTrackedGpuBuffers.empty());
    SkASSERT(fTrackedGpuSurfaces.empty());
    SkASSERT(cmdPool != VK_NULL_HANDLE);
    SkASSERT(!this->isWrapped());

    const GrVkGpu* vkGpu = (const GrVkGpu*)gpu;
    GR_VK_CALL(vkGpu->vkInterface(), FreeCommandBuffers(vkGpu->device(), cmdPool, 1, &fCmdBuffer));

    this->onFreeGPUData(vkGpu);
}

void GrVkCommandBuffer::releaseResources() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(!fIsActive || this->isWrapped());
    fTrackedResources.clear();
    fTrackedRecycledResources.clear();

    fTrackedGpuBuffers.clear();
    fTrackedGpuSurfaces.clear();

    this->invalidateState();

    this->onReleaseResources();
}

////////////////////////////////////////////////////////////////////////////////
// CommandBuffer commands
////////////////////////////////////////////////////////////////////////////////

void GrVkCommandBuffer::pipelineBarrier(const GrVkGpu* gpu,
                                        const GrManagedResource* resource,
                                        VkPipelineStageFlags srcStageMask,
                                        VkPipelineStageFlags dstStageMask,
                                        bool byRegion,
                                        BarrierType barrierType,
                                        void* barrier) {
    SkASSERT(!this->isWrapped());
    SkASSERT(fIsActive);
#ifdef SK_DEBUG
    // For images we can have barriers inside of render passes but they require us to add more
    // support in subpasses which need self dependencies to have barriers inside them. Also, we can
    // never have buffer barriers inside of a render pass. For now we will just assert that we are
    // not in a render pass.
    bool isValidSubpassBarrier = false;
    if (barrierType == kImageMemory_BarrierType) {
        VkImageMemoryBarrier* imgBarrier = static_cast<VkImageMemoryBarrier*>(barrier);
        isValidSubpassBarrier = (imgBarrier->newLayout == imgBarrier->oldLayout) &&
                                (imgBarrier->srcQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) &&
                                (imgBarrier->dstQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) &&
                                byRegion;
    }
    SkASSERT(!fActiveRenderPass || isValidSubpassBarrier);
#endif

    if (barrierType == kBufferMemory_BarrierType) {
        const VkBufferMemoryBarrier* barrierPtr = static_cast<VkBufferMemoryBarrier*>(barrier);
        fBufferBarriers.push_back(*barrierPtr);
    } else {
        SkASSERT(barrierType == kImageMemory_BarrierType);
        const VkImageMemoryBarrier* barrierPtr = static_cast<VkImageMemoryBarrier*>(barrier);
        // We need to check if we are adding a pipeline barrier that covers part of the same
        // subresource range as a barrier that is already in current batch. If it does, then we must
        // submit the first batch because the vulkan spec does not define a specific ordering for
        // barriers submitted in the same batch.
        // TODO: Look if we can gain anything by merging barriers together instead of submitting
        // the old ones.
        for (int i = 0; i < fImageBarriers.size(); ++i) {
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
                if (std::max(newStart, oldStart) <= std::min(newEnd, oldEnd)) {
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
    if (fActiveRenderPass) {
        this->submitPipelineBarriers(gpu, true);
    }
}

void GrVkCommandBuffer::submitPipelineBarriers(const GrVkGpu* gpu, bool forSelfDependency) {
    SkASSERT(fIsActive);

    // Currently we never submit a pipeline barrier without at least one memory barrier.
    if (!fBufferBarriers.empty() || !fImageBarriers.empty()) {
        // For images we can have barriers inside of render passes but they require us to add more
        // support in subpasses which need self dependencies to have barriers inside them. Also, we
        // can never have buffer barriers inside of a render pass. For now we will just assert that
        // we are not in a render pass.
        SkASSERT(!fActiveRenderPass || forSelfDependency);
        SkASSERT(!this->isWrapped());
        SkASSERT(fSrcStageMask && fDstStageMask);

        // TODO(https://crbug.com/1469231): The linked bug references a crash report from calling
        // CmdPipelineBarrier. The checks below were added to ensure that we are passing in buffer
        // counts >= 0, and in the case of >0, that the buffers are non-null. Evaluate whether this
        // change leads to a reduction in crash instances. If not, the issue may lie within the
        // driver itself and these checks can be removed.
        if (!fBufferBarriers.empty() && fBufferBarriers.begin() == nullptr) {
            fBufferBarriers.clear(); // Sets the size to 0
        }
        if (!fImageBarriers.empty() && fImageBarriers.begin() == nullptr) {
            fImageBarriers.clear(); // Sets the size to 0
        }

        VkDependencyFlags dependencyFlags = fBarriersByRegion ? VK_DEPENDENCY_BY_REGION_BIT : 0;
        GR_VK_CALL(gpu->vkInterface(), CmdPipelineBarrier(
                fCmdBuffer, fSrcStageMask, fDstStageMask, dependencyFlags, 0, nullptr,
                fBufferBarriers.size(), fBufferBarriers.begin(),
                fImageBarriers.size(), fImageBarriers.begin()));
        fBufferBarriers.clear();
        fImageBarriers.clear();
        fBarriersByRegion = false;
        fSrcStageMask = 0;
        fDstStageMask = 0;
    }
    SkASSERT(fBufferBarriers.empty());
    SkASSERT(fImageBarriers.empty());
    SkASSERT(!fBarriersByRegion);
    SkASSERT(!fSrcStageMask);
    SkASSERT(!fDstStageMask);
}

void GrVkCommandBuffer::bindInputBuffer(GrVkGpu* gpu, uint32_t binding,
                                        sk_sp<const GrBuffer> buffer) {
    VkBuffer vkBuffer = static_cast<const GrVkBuffer*>(buffer.get())->vkBuffer();
    SkASSERT(VK_NULL_HANDLE != vkBuffer);
    SkASSERT(binding < kMaxInputBuffers);
    // TODO: once vbuffer->offset() no longer always returns 0, we will need to track the offset
    // to know if we can skip binding or not.
    if (vkBuffer != fBoundInputBuffers[binding]) {
        VkDeviceSize offset = 0;
        GR_VK_CALL(gpu->vkInterface(), CmdBindVertexBuffers(fCmdBuffer,
                                                            binding,
                                                            1,
                                                            &vkBuffer,
                                                            &offset));
        fBoundInputBuffers[binding] = vkBuffer;
        this->addGrBuffer(std::move(buffer));
    }
}

void GrVkCommandBuffer::bindIndexBuffer(GrVkGpu* gpu, sk_sp<const GrBuffer> buffer) {
    VkBuffer vkBuffer = static_cast<const GrVkBuffer*>(buffer.get())->vkBuffer();
    SkASSERT(VK_NULL_HANDLE != vkBuffer);
    // TODO: once ibuffer->offset() no longer always returns 0, we will need to track the offset
    // to know if we can skip binding or not.
    if (vkBuffer != fBoundIndexBuffer) {
        GR_VK_CALL(gpu->vkInterface(), CmdBindIndexBuffer(fCmdBuffer,
                                                          vkBuffer, /*offset=*/0,
                                                          VK_INDEX_TYPE_UINT16));
        fBoundIndexBuffer = vkBuffer;
        this->addGrBuffer(std::move(buffer));
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
    if (gpu->vkCaps().mustInvalidatePrimaryCmdBufferStateAfterClearAttachments()) {
        this->invalidateState();
    }
}

void GrVkCommandBuffer::bindDescriptorSets(const GrVkGpu* gpu,
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
}

void GrVkCommandBuffer::bindPipeline(const GrVkGpu* gpu, sk_sp<const GrVkPipeline> pipeline) {
    SkASSERT(fIsActive);
    GR_VK_CALL(gpu->vkInterface(), CmdBindPipeline(fCmdBuffer,
                                                   VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                   pipeline->pipeline()));
    this->addResource(std::move(pipeline));
}

void GrVkCommandBuffer::pushConstants(const GrVkGpu* gpu, VkPipelineLayout layout,
                                      VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                      const void* values) {
    SkASSERT(fIsActive);
    // offset and size must be a multiple of 4
    SkASSERT(!SkToBool(offset & 0x3));
    SkASSERT(!SkToBool(size & 0x3));
    GR_VK_CALL(gpu->vkInterface(), CmdPushConstants(fCmdBuffer,
                                                    layout,
                                                    stageFlags,
                                                    offset,
                                                    size,
                                                    values));
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

void GrVkCommandBuffer::drawIndirect(const GrVkGpu* gpu,
                                     sk_sp<const GrBuffer> indirectBuffer,
                                     VkDeviceSize offset,
                                     uint32_t drawCount,
                                     uint32_t stride) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    SkASSERT(!indirectBuffer->isCpuBuffer());
    this->addingWork(gpu);
    VkBuffer vkBuffer = static_cast<const GrVkBuffer*>(indirectBuffer.get())->vkBuffer();
    GR_VK_CALL(gpu->vkInterface(), CmdDrawIndirect(fCmdBuffer,
                                                   vkBuffer,
                                                   offset,
                                                   drawCount,
                                                   stride));
    this->addGrBuffer(std::move(indirectBuffer));
}

void GrVkCommandBuffer::drawIndexedIndirect(const GrVkGpu* gpu,
                                            sk_sp<const GrBuffer> indirectBuffer,
                                            VkDeviceSize offset,
                                            uint32_t drawCount,
                                            uint32_t stride) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    SkASSERT(!indirectBuffer->isCpuBuffer());
    this->addingWork(gpu);
    VkBuffer vkBuffer = static_cast<const GrVkBuffer*>(indirectBuffer.get())->vkBuffer();
    GR_VK_CALL(gpu->vkInterface(), CmdDrawIndexedIndirect(fCmdBuffer,
                                                          vkBuffer,
                                                          offset,
                                                          drawCount,
                                                          stride));
    this->addGrBuffer(std::move(indirectBuffer));
}

void GrVkCommandBuffer::setViewport(const GrVkGpu* gpu,
                                    uint32_t firstViewport,
                                    uint32_t viewportCount,
                                    const VkViewport* viewports) {
    SkASSERT(fIsActive);
    SkASSERT(1 == viewportCount);
    if (0 != memcmp(viewports, &fCachedViewport, sizeof(VkViewport))) {
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
    if (0 != memcmp(scissors, &fCachedScissor, sizeof(VkRect2D))) {
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
    if (0 != memcmp(blendConstants, fCachedBlendConstant, 4 * sizeof(float))) {
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

GrVkPrimaryCommandBuffer* GrVkPrimaryCommandBuffer::Create(GrVkGpu* gpu,
                                                           VkCommandPool cmdPool) {
    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        cmdPool,                                          // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
        1                                                 // bufferCount
    };

    VkCommandBuffer cmdBuffer;
    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, AllocateCommandBuffers(gpu->device(), &cmdInfo, &cmdBuffer));
    if (err) {
        return nullptr;
    }
    return new GrVkPrimaryCommandBuffer(cmdBuffer);
}

void GrVkPrimaryCommandBuffer::begin(GrVkGpu* gpu) {
    SkASSERT(!fIsActive);
    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = nullptr;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = nullptr;

    GR_VK_CALL_ERRCHECK(gpu, BeginCommandBuffer(fCmdBuffer, &cmdBufferBeginInfo));
    fIsActive = true;
}

void GrVkPrimaryCommandBuffer::end(GrVkGpu* gpu, bool abandoningBuffer) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);

    // If we are in the process of abandoning the context then the GrResourceCache will have freed
    // all resources before destroying the GrVkGpu. When we destroy the GrVkGpu we call end on the
    // command buffer to keep all our state tracking consistent. However, the vulkan validation
    // layers complain about calling end on a command buffer that contains resources that have
    // already been deleted. From the vulkan API it isn't required to end the command buffer to
    // delete it, so we just skip the vulkan API calls and update our own state tracking.
    if (!abandoningBuffer) {
        this->submitPipelineBarriers(gpu);

        GR_VK_CALL_ERRCHECK(gpu, EndCommandBuffer(fCmdBuffer));
    }
    this->invalidateState();
    fIsActive = false;
    fHasWork = false;
}

bool GrVkPrimaryCommandBuffer::beginRenderPass(GrVkGpu* gpu,
                                               const GrVkRenderPass* renderPass,
                                               sk_sp<const GrVkFramebuffer> framebuffer,
                                               const VkClearValue clearValues[],
                                               const GrSurface* target,
                                               const SkIRect& bounds,
                                               bool forSecondaryCB) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);

    SkASSERT(framebuffer);

    this->addingWork(gpu);

    VkRenderPassBeginInfo beginInfo;
    VkRect2D renderArea;
    renderArea.offset = { bounds.fLeft , bounds.fTop };
    renderArea.extent = { (uint32_t)bounds.width(), (uint32_t)bounds.height() };

    memset(&beginInfo, 0, sizeof(VkRenderPassBeginInfo));
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = renderPass->vkRenderPass();
    beginInfo.framebuffer = framebuffer->framebuffer();
    beginInfo.renderArea = renderArea;
    beginInfo.clearValueCount = renderPass->clearValueCount();
    beginInfo.pClearValues = clearValues;

    VkSubpassContents contents = forSecondaryCB ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                                : VK_SUBPASS_CONTENTS_INLINE;

    GR_VK_CALL(gpu->vkInterface(), CmdBeginRenderPass(fCmdBuffer, &beginInfo, contents));
    fActiveRenderPass = renderPass;
    this->addResource(renderPass);
    this->addResource(std::move(framebuffer));
    this->addGrSurface(sk_ref_sp(target));
    return true;
}

void GrVkPrimaryCommandBuffer::endRenderPass(const GrVkGpu* gpu) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    this->addingWork(gpu);
    GR_VK_CALL(gpu->vkInterface(), CmdEndRenderPass(fCmdBuffer));
    fActiveRenderPass = nullptr;
}


void GrVkPrimaryCommandBuffer::nexSubpass(GrVkGpu* gpu, bool forSecondaryCB) {
    SkASSERT(fIsActive);
    SkASSERT(fActiveRenderPass);
    VkSubpassContents contents = forSecondaryCB ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                                : VK_SUBPASS_CONTENTS_INLINE;
    GR_VK_CALL(gpu->vkInterface(), CmdNextSubpass(fCmdBuffer, contents));
}

void GrVkPrimaryCommandBuffer::executeCommands(const GrVkGpu* gpu,
                                               std::unique_ptr<GrVkSecondaryCommandBuffer> buffer) {
    // The Vulkan spec allows secondary command buffers to be executed on a primary command buffer
    // if the command pools both were created from were created with the same queue family. However,
    // we currently always create them from the same pool.
    SkASSERT(fIsActive);
    SkASSERT(!buffer->fIsActive);
    SkASSERT(fActiveRenderPass);
    SkASSERT(fActiveRenderPass->isCompatible(*buffer->fActiveRenderPass));

    this->addingWork(gpu);

    GR_VK_CALL(gpu->vkInterface(), CmdExecuteCommands(fCmdBuffer, 1, &buffer->fCmdBuffer));
    fSecondaryCommandBuffers.push_back(std::move(buffer));
    // When executing a secondary command buffer all state (besides render pass state) becomes
    // invalidated and must be reset. This includes bound buffers, pipelines, dynamic state, etc.
    this->invalidateState();
}

static bool submit_to_queue(GrVkGpu* gpu,
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
    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, QueueSubmit(queue, 1, &submitInfo, fence));
    return result == VK_SUCCESS;
}

bool GrVkPrimaryCommandBuffer::submitToQueue(
        GrVkGpu* gpu,
        VkQueue queue,
        TArray<GrVkSemaphore::Resource*>& signalSemaphores,
        TArray<GrVkSemaphore::Resource*>& waitSemaphores) {
    SkASSERT(!fIsActive);

    VkResult err;
    if (VK_NULL_HANDLE == fSubmitFence) {
        VkFenceCreateInfo fenceInfo;
        memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        GR_VK_CALL_RESULT(gpu, err, CreateFence(gpu->device(), &fenceInfo, nullptr,
                                                &fSubmitFence));
        if (err) {
            fSubmitFence = VK_NULL_HANDLE;
            return false;
        }
    } else {
        // This cannot return DEVICE_LOST so we assert we succeeded.
        GR_VK_CALL_RESULT(gpu, err, ResetFences(gpu->device(), 1, &fSubmitFence));
        SkASSERT(err == VK_SUCCESS);
    }

    int signalCount = signalSemaphores.size();
    int waitCount = waitSemaphores.size();

    bool submitted = false;

    if (0 == signalCount && 0 == waitCount) {
        // This command buffer has no dependent semaphores so we can simply just submit it to the
        // queue with no worries.
        submitted = submit_to_queue(
                gpu, queue, fSubmitFence, 0, nullptr, nullptr, 1, &fCmdBuffer, 0, nullptr,
                GrProtected(gpu->protectedContext()));
    } else {
        TArray<VkSemaphore> vkSignalSems(signalCount);
        for (int i = 0; i < signalCount; ++i) {
            if (signalSemaphores[i]->shouldSignal()) {
                this->addResource(signalSemaphores[i]);
                vkSignalSems.push_back(signalSemaphores[i]->semaphore());
            }
        }

        TArray<VkSemaphore> vkWaitSems(waitCount);
        TArray<VkPipelineStageFlags> vkWaitStages(waitCount);
        for (int i = 0; i < waitCount; ++i) {
            if (waitSemaphores[i]->shouldWait()) {
                this->addResource(waitSemaphores[i]);
                vkWaitSems.push_back(waitSemaphores[i]->semaphore());
                // We only block the fragment stage since client provided resources are not used
                // before the fragment stage. This allows the driver to begin vertex work while
                // waiting on the semaphore. We also add in the transfer stage for uses of clients
                // calling read or write pixels.
                vkWaitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                       VK_PIPELINE_STAGE_TRANSFER_BIT);
            }
        }
        submitted = submit_to_queue(gpu, queue, fSubmitFence, vkWaitSems.size(),
                                    vkWaitSems.begin(), vkWaitStages.begin(), 1, &fCmdBuffer,
                                    vkSignalSems.size(), vkSignalSems.begin(),
                                    GrProtected(gpu->protectedContext()));
        if (submitted) {
            for (int i = 0; i < signalCount; ++i) {
                signalSemaphores[i]->markAsSignaled();
            }
            for (int i = 0; i < waitCount; ++i) {
                waitSemaphores[i]->markAsWaited();
            }
        }
    }

    if (!submitted) {
        // Destroy the fence or else we will try to wait forever for it to finish.
        GR_VK_CALL(gpu->vkInterface(), DestroyFence(gpu->device(), fSubmitFence, nullptr));
        fSubmitFence = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

void GrVkPrimaryCommandBuffer::forceSync(GrVkGpu* gpu) {
    if (fSubmitFence == VK_NULL_HANDLE) {
        return;
    }
    GR_VK_CALL_ERRCHECK(gpu, WaitForFences(gpu->device(), 1, &fSubmitFence, true, UINT64_MAX));
}

bool GrVkPrimaryCommandBuffer::finished(GrVkGpu* gpu) {
    SkASSERT(!fIsActive);
    if (VK_NULL_HANDLE == fSubmitFence) {
        return true;
    }

    VkResult err;
    GR_VK_CALL_RESULT_NOCHECK(gpu, err, GetFenceStatus(gpu->device(), fSubmitFence));
    switch (err) {
        case VK_SUCCESS:
        case VK_ERROR_DEVICE_LOST:
            return true;

        case VK_NOT_READY:
            return false;

        default:
            SkDebugf("Error getting fence status: %d\n", err);
            SK_ABORT("Got an invalid fence status");
            return false;
    }
}

void GrVkPrimaryCommandBuffer::addFinishedProc(sk_sp<skgpu::RefCntedCallback> finishedProc) {
    fFinishedProcs.push_back(std::move(finishedProc));
}

void GrVkPrimaryCommandBuffer::onReleaseResources() {
    for (int i = 0; i < fSecondaryCommandBuffers.size(); ++i) {
        fSecondaryCommandBuffers[i]->releaseResources();
    }
    this->callFinishedProcs();
}

void GrVkPrimaryCommandBuffer::recycleSecondaryCommandBuffers(GrVkCommandPool* cmdPool) {
    for (int i = 0; i < fSecondaryCommandBuffers.size(); ++i) {
        fSecondaryCommandBuffers[i].release()->recycle(cmdPool);
    }
    fSecondaryCommandBuffers.clear();
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
                                         const GrManagedResource* srcResource,
                                         VkImage srcImage,
                                         VkImageLayout srcLayout,
                                         const GrManagedResource* dstResource,
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
                                                 sk_sp<GrGpuBuffer> dstBuffer,
                                                 uint32_t copyRegionCount,
                                                 const VkBufferImageCopy* copyRegions) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addingWork(gpu);
    GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(dstBuffer.get());
    GR_VK_CALL(gpu->vkInterface(), CmdCopyImageToBuffer(fCmdBuffer,
                                                        srcImage->image(),
                                                        srcLayout,
                                                        vkBuffer->vkBuffer(),
                                                        copyRegionCount,
                                                        copyRegions));
    this->addResource(srcImage->resource());
    this->addGrBuffer(std::move(dstBuffer));
}

void GrVkPrimaryCommandBuffer::copyBufferToImage(const GrVkGpu* gpu,
                                                 VkBuffer srcBuffer,
                                                 GrVkImage* dstImage,
                                                 VkImageLayout dstLayout,
                                                 uint32_t copyRegionCount,
                                                 const VkBufferImageCopy* copyRegions) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addingWork(gpu);

    GR_VK_CALL(gpu->vkInterface(), CmdCopyBufferToImage(fCmdBuffer,
                                                        srcBuffer,
                                                        dstImage->image(),
                                                        dstLayout,
                                                        copyRegionCount,
                                                        copyRegions));
    this->addResource(dstImage->resource());
}

void GrVkPrimaryCommandBuffer::fillBuffer(GrVkGpu* gpu,
                                          sk_sp<GrGpuBuffer> buffer,
                                          VkDeviceSize offset,
                                          VkDeviceSize size,
                                          uint32_t data) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    this->addingWork(gpu);

    const GrVkBuffer* bufferVk = static_cast<GrVkBuffer*>(buffer.get());

    GR_VK_CALL(gpu->vkInterface(), CmdFillBuffer(fCmdBuffer,
                                                 bufferVk->vkBuffer(),
                                                 offset,
                                                 size,
                                                 data));
    this->addGrBuffer(std::move(buffer));
}

void GrVkPrimaryCommandBuffer::copyBuffer(GrVkGpu* gpu,
                                          sk_sp<GrGpuBuffer> srcBuffer,
                                          sk_sp<GrGpuBuffer> dstBuffer,
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

    const GrVkBuffer* srcVk = static_cast<GrVkBuffer*>(srcBuffer.get());
    const GrVkBuffer* dstVk = static_cast<GrVkBuffer*>(dstBuffer.get());

    GR_VK_CALL(gpu->vkInterface(), CmdCopyBuffer(fCmdBuffer,
                                                 srcVk->vkBuffer(),
                                                 dstVk->vkBuffer(),
                                                 regionCount,
                                                 regions));
    this->addGrBuffer(std::move(srcBuffer));
    this->addGrBuffer(std::move(dstBuffer));
}

void GrVkPrimaryCommandBuffer::updateBuffer(GrVkGpu* gpu,
                                            sk_sp<GrVkBuffer> dstBuffer,
                                            VkDeviceSize dstOffset,
                                            VkDeviceSize dataSize,
                                            const void* data) {
    SkASSERT(fIsActive);
    SkASSERT(!fActiveRenderPass);
    SkASSERT(0 == (dstOffset & 0x03));  // four byte aligned
    // TODO: handle larger transfer sizes
    SkASSERT(dataSize <= 65536);
    SkASSERT(0 == (dataSize & 0x03));  // four byte aligned
    this->addingWork(gpu);
    GR_VK_CALL(
            gpu->vkInterface(),
            CmdUpdateBuffer(
                    fCmdBuffer, dstBuffer->vkBuffer(), dstOffset, dataSize, (const uint32_t*)data));
    this->addGrBuffer(std::move(dstBuffer));
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

void GrVkPrimaryCommandBuffer::onFreeGPUData(const GrVkGpu* gpu) const {
    SkASSERT(!fActiveRenderPass);
    // Destroy the fence, if any
    if (VK_NULL_HANDLE != fSubmitFence) {
        GR_VK_CALL(gpu->vkInterface(), DestroyFence(gpu->device(), fSubmitFence, nullptr));
    }
    SkASSERT(fSecondaryCommandBuffers.empty());
}

///////////////////////////////////////////////////////////////////////////////
// SecondaryCommandBuffer
////////////////////////////////////////////////////////////////////////////////

GrVkSecondaryCommandBuffer* GrVkSecondaryCommandBuffer::Create(GrVkGpu* gpu,
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
    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, AllocateCommandBuffers(gpu->device(), &cmdInfo, &cmdBuffer));
    if (err) {
        return nullptr;
    }
    return new GrVkSecondaryCommandBuffer(cmdBuffer, /*externalRenderPass=*/nullptr);
}

GrVkSecondaryCommandBuffer* GrVkSecondaryCommandBuffer::Create(
        VkCommandBuffer cmdBuffer, const GrVkRenderPass* externalRenderPass) {
    return new GrVkSecondaryCommandBuffer(cmdBuffer, externalRenderPass);
}

void GrVkSecondaryCommandBuffer::begin(GrVkGpu* gpu, const GrVkFramebuffer* framebuffer,
                                       const GrVkRenderPass* compatibleRenderPass) {
    SkASSERT(!fIsActive);
    SkASSERT(!this->isWrapped());
    SkASSERT(compatibleRenderPass);
    fActiveRenderPass = compatibleRenderPass;

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

    GR_VK_CALL_ERRCHECK(gpu, BeginCommandBuffer(fCmdBuffer, &cmdBufferBeginInfo));

    fIsActive = true;
}

void GrVkSecondaryCommandBuffer::end(GrVkGpu* gpu) {
    SkASSERT(fIsActive);
    SkASSERT(!this->isWrapped());
    GR_VK_CALL_ERRCHECK(gpu, EndCommandBuffer(fCmdBuffer));
    this->invalidateState();
    fHasWork = false;
    fIsActive = false;
}

void GrVkSecondaryCommandBuffer::recycle(GrVkCommandPool* cmdPool) {
    if (this->isWrapped()) {
        delete this;
    } else {
        cmdPool->recycleSecondaryCommandBuffer(this);
    }
}
