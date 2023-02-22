/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

namespace skgpu::graphite {

std::unique_ptr<VulkanCommandBuffer> VulkanCommandBuffer::Make(
        const VulkanSharedContext* sharedContext,
        VulkanResourceProvider* resourceProvider) {
    // Create VkCommandPool
    VkCommandPoolCreateFlags cmdPoolCreateFlags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (sharedContext->isProtected() == Protected::kYes) {
        cmdPoolCreateFlags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    }

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
        nullptr,                                     // pNext
        cmdPoolCreateFlags,                          // CmdPoolCreateFlags
        sharedContext->queueIndex(),                 // queueFamilyIndex
    };
    auto interface = sharedContext->interface();
    VkResult result;
    VkCommandPool pool;
    VULKAN_CALL_RESULT(interface, result, CreateCommandPool(sharedContext->device(),
                                                            &cmdPoolInfo,
                                                            nullptr,
                                                            &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        pool,                                             // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
        1                                                 // bufferCount
    };

    VkCommandBuffer primaryCmdBuffer;
    VULKAN_CALL_RESULT(interface, result, AllocateCommandBuffers(sharedContext->device(),
                                                                 &cmdInfo,
                                                                 &primaryCmdBuffer));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(interface, DestroyCommandPool(sharedContext->device(), pool, nullptr));
        return nullptr;
    }

    return std::unique_ptr<VulkanCommandBuffer>(new VulkanCommandBuffer(pool,
                                                                        primaryCmdBuffer,
                                                                        sharedContext,
                                                                        resourceProvider));
}

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool pool,
                                         VkCommandBuffer primaryCommandBuffer,
                                         const VulkanSharedContext* sharedContext,
                                         VulkanResourceProvider* resourceProvider)
        : fPool(pool)
        , fPrimaryCommandBuffer(primaryCommandBuffer)
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider) {

    // TODO: Remove this line. It is only here to hide compiler warnings/errors about unused
    // member variables.
    (void) fResourceProvider;
    // When making a new command buffer, we automatically begin the command buffer
    this->begin();
}

VulkanCommandBuffer::~VulkanCommandBuffer() {}

void VulkanCommandBuffer::onResetCommandBuffer() {
    SkASSERT(!fActive);
    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), ResetCommandPool(fSharedContext->device(),
                                                                       fPool,
                                                                       0));
}

bool VulkanCommandBuffer::setNewCommandBufferResources() {
    this->begin();
    return true;
}

void VulkanCommandBuffer::begin() {
    SkASSERT(!fActive);
    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = nullptr;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = nullptr;

    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), BeginCommandBuffer(fPrimaryCommandBuffer,
                                                                         &cmdBufferBeginInfo));
    SkDEBUGCODE(fActive = true;)
}

void VulkanCommandBuffer::end() {
    SkASSERT(fActive);

    this->submitPipelineBarriers();

    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), EndCommandBuffer(fPrimaryCommandBuffer));

    SkDEBUGCODE(fActive = false;)
}

static bool submit_to_queue(const VulkanInterface* interface,
                            VkQueue queue,
                            VkFence fence,
                            uint32_t waitCount,
                            const VkSemaphore* waitSemaphores,
                            const VkPipelineStageFlags* waitStages,
                            uint32_t commandBufferCount,
                            const VkCommandBuffer* commandBuffers,
                            uint32_t signalCount,
                            const VkSemaphore* signalSemaphores,
                            Protected protectedContext) {
    VkProtectedSubmitInfo protectedSubmitInfo;
    if (protectedContext == Protected::kYes) {
        memset(&protectedSubmitInfo, 0, sizeof(VkProtectedSubmitInfo));
        protectedSubmitInfo.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.pNext = nullptr;
        protectedSubmitInfo.protectedSubmit = VK_TRUE;
    }

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = protectedContext == Protected::kYes ? &protectedSubmitInfo : nullptr;
    submitInfo.waitSemaphoreCount = waitCount;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = commandBuffers;
    submitInfo.signalSemaphoreCount = signalCount;
    submitInfo.pSignalSemaphores = signalSemaphores;
    VkResult result;
    VULKAN_CALL_RESULT(interface, result, QueueSubmit(queue, 1, &submitInfo, fence));
    if (result != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool VulkanCommandBuffer::submit(VkQueue queue) {
    this->end();

    auto interface = fSharedContext->interface();
    auto device = fSharedContext->device();
    VkResult err;

    if (fSubmitFence == VK_NULL_HANDLE) {
        VkFenceCreateInfo fenceInfo;
        memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VULKAN_CALL_RESULT(interface, err, CreateFence(device,
                                                       &fenceInfo,
                                                       nullptr,
                                                       &fSubmitFence));
        if (err) {
            fSubmitFence = VK_NULL_HANDLE;
            return false;
        }
    } else {
        // This cannot return DEVICE_LOST so we assert we succeeded.
        VULKAN_CALL_RESULT(interface, err, ResetFences(device, 1, &fSubmitFence));
        SkASSERT(err == VK_SUCCESS);
    }

    SkASSERT(fSubmitFence != VK_NULL_HANDLE);

    bool submitted = submit_to_queue(interface,
                                     queue,
                                     fSubmitFence,
                                     /*waitCount=*/0,
                                     /*waitSemaphores=*/nullptr,
                                     /*waitStages=*/nullptr,
                                     /*commandBufferCount*/1,
                                     &fPrimaryCommandBuffer,
                                     /*signalCount=*/0,
                                     /*signalSemaphores=*/nullptr,
                                     fSharedContext->isProtected());
    if (!submitted) {
        // Destroy the fence or else we will try to wait forever for it to finish.
        VULKAN_CALL(interface, DestroyFence(device, fSubmitFence, nullptr));
        fSubmitFence = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

bool VulkanCommandBuffer::isFinished() {
    SkASSERT(!fActive);
    if (VK_NULL_HANDLE == fSubmitFence) {
        return true;
    }

    VkResult err;
    VULKAN_CALL_RESULT_NOCHECK(fSharedContext->interface(), err,
                               GetFenceStatus(fSharedContext->device(), fSubmitFence));
    switch (err) {
        case VK_SUCCESS:
        case VK_ERROR_DEVICE_LOST:
            return true;

        case VK_NOT_READY:
            return false;

        default:
            SKGPU_LOG_F("Error calling vkGetFenceStatus. Error: %d", err);
            SK_ABORT("Got an invalid fence status");
            return false;
    }
}

void VulkanCommandBuffer::waitUntilFinished() {
    if (fSubmitFence == VK_NULL_HANDLE) {
        return;
    }
    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), WaitForFences(fSharedContext->device(),
                                                                    1,
                                                                    &fSubmitFence,
                                                                    /*waitAll=*/true,
                                                                    /*timeout=*/UINT64_MAX));
}

bool VulkanCommandBuffer::onAddRenderPass(
        const RenderPassDesc&,
        const Texture* colorTexture,
        const Texture* resolveTexture,
        const Texture* depthStencilTexture,
        SkRect viewport,
        const std::vector<std::unique_ptr<DrawPass>>& drawPasses) {
    return false;
}

bool VulkanCommandBuffer::onAddComputePass(const ComputePassDesc&,
                                           const ComputePipeline*,
                                           const std::vector<ResourceBinding>& bindings) {
    return false;
}

bool VulkanCommandBuffer::onCopyBufferToBuffer(const Buffer* srcBuffer,
                                               size_t srcOffset,
                                               const Buffer* dstBuffer,
                                               size_t dstOffset,
                                               size_t size) {
    return false;
}

bool VulkanCommandBuffer::onCopyTextureToBuffer(const Texture* texture,
                                                SkIRect srcRect,
                                                const Buffer* buffer,
                                                size_t bufferOffset,
                                                size_t bufferRowBytes) {
    this->submitPipelineBarriers();

    const VulkanTexture* srcTexture = static_cast<const VulkanTexture*>(texture);
    VkBuffer dstBuffer = static_cast<const VulkanBuffer*>(buffer)->vkBuffer();

    // Obtain the VkFormat of the source texture so we can determine bytes per block.
    VulkanTextureInfo srcTextureInfo;
    texture->textureInfo().getVulkanTextureInfo(&srcTextureInfo);
    size_t bytesPerBlock = VkFormatBytesPerBlock(srcTextureInfo.fFormat);

    // Set up copy region
    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = bufferOffset;
    // Vulkan expects bufferRowLength in texels, not bytes.
    region.bufferRowLength = (uint32_t)(bufferRowBytes/bytesPerBlock);
    region.bufferImageHeight = 0; // Tightly packed
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, /*mipLevel=*/0, 0, 1 };
    region.imageOffset = { srcRect.left(), srcRect.top(), /*z=*/0 };
    region.imageExtent = { (uint32_t)srcRect.width(), (uint32_t)srcRect.height(), /*depth=*/1 };

    // Enable editing of the source texture so we can change its layout so it can be copied from.
    const_cast<VulkanTexture*>(srcTexture)->setImageLayout(this,
                                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                           VK_ACCESS_TRANSFER_READ_BIT,
                                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                           false);

    VULKAN_CALL(fSharedContext->interface(),
                CmdCopyImageToBuffer(fPrimaryCommandBuffer,
                                     srcTexture->vkImage(),
                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     dstBuffer,
                                     /*regionCount=*/1,
                                     &region));
    return true;
}

bool VulkanCommandBuffer::onCopyBufferToTexture(const Buffer* buffer,
                                                const Texture* texture,
                                                const BufferTextureCopyData* copyData,
                                                int count) {
    this->submitPipelineBarriers();

    VkBuffer srcBuffer = static_cast<const VulkanBuffer*>(buffer)->vkBuffer();
    const VulkanTexture* dstTexture = static_cast<const VulkanTexture*>(texture);

    // Obtain the VkFormat of the destination texture so we can determine bytes per block.
    VulkanTextureInfo dstTextureInfo;
    dstTexture->textureInfo().getVulkanTextureInfo(&dstTextureInfo);
    size_t bytesPerBlock = VkFormatBytesPerBlock(dstTextureInfo.fFormat);

    // Set up copy regions.
    SkTArray<VkBufferImageCopy> regions(count);
    for (int i = 0; i < count; ++i) {
        VkBufferImageCopy& region = regions.push_back();
        memset(&region, 0, sizeof(VkBufferImageCopy));
        region.bufferOffset = copyData[i].fBufferOffset;
        // copyData provides row length in bytes, but Vulkan expects bufferRowLength in texels.
        region.bufferRowLength = (uint32_t)(copyData[i].fBufferRowBytes/bytesPerBlock);
        region.bufferImageHeight = 0; // Tightly packed
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, copyData[i].fMipLevel, 0, 1 };
        region.imageOffset = { copyData[i].fRect.left(),
                               copyData[i].fRect.top(),
                               /*z=*/0 };
        region.imageExtent = { (uint32_t)copyData[i].fRect.width(),
                               (uint32_t)copyData[i].fRect.height(),
                               /*depth=*/1 };
    }

    // Enable editing of the destination texture so we can change its layout so it can be copied to.
    const_cast<VulkanTexture*>(dstTexture)->setImageLayout(this,
                                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                           VK_ACCESS_TRANSFER_WRITE_BIT,
                                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                           false);

    VULKAN_CALL(fSharedContext->interface(),
            CmdCopyBufferToImage(fPrimaryCommandBuffer,
                                 srcBuffer,
                                 dstTexture->vkImage(),
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 regions.size(),
                                 regions.begin()));
    return true;
}

bool VulkanCommandBuffer::onCopyTextureToTexture(const Texture* src,
                                                 SkIRect srcRect,
                                                 const Texture* dst,
                                                 SkIPoint dstPoint) {
    return false;
}

bool VulkanCommandBuffer::onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) {
    return false;
}

bool VulkanCommandBuffer::onClearBuffer(const Buffer*, size_t offset, size_t size) {
    return false;
}

#ifdef SK_ENABLE_PIET_GPU
void VulkanCommandBuffer::onRenderPietScene(const skgpu::piet::Scene& scene,
                                            const Texture* target) {}
#endif

void VulkanCommandBuffer::addBufferMemoryBarrier(const Resource* resource,
                                                 VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask,
                                                 bool byRegion,
                                                 VkBufferMemoryBarrier* barrier) {
    SkASSERT(resource);
    this->pipelineBarrier(resource,
                          srcStageMask,
                          dstStageMask,
                          byRegion,
                          kBufferMemory_BarrierType,
                          barrier);
}

void VulkanCommandBuffer::addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask,
                                                 bool byRegion,
                                                 VkBufferMemoryBarrier* barrier) {
    // We don't pass in a resource here to the command buffer. The command buffer only is using it
    // to hold a ref, but every place where we add a buffer memory barrier we are doing some other
    // command with the buffer on the command buffer. Thus those other commands will already cause
    // the command buffer to be holding a ref to the buffer.
    this->pipelineBarrier(/*resource=*/nullptr,
                          srcStageMask,
                          dstStageMask,
                          byRegion,
                          kBufferMemory_BarrierType,
                          barrier);
}

void VulkanCommandBuffer::addImageMemoryBarrier(const Resource* resource,
                                                VkPipelineStageFlags srcStageMask,
                                                VkPipelineStageFlags dstStageMask,
                                                bool byRegion,
                                                VkImageMemoryBarrier* barrier) {
    SkASSERT(resource);
    this->pipelineBarrier(resource,
                          srcStageMask,
                          dstStageMask,
                          byRegion,
                          kImageMemory_BarrierType,
                          barrier);
}

void VulkanCommandBuffer::pipelineBarrier(const Resource* resource,
                                          VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask,
                                          bool byRegion,
                                          BarrierType barrierType,
                                          void* barrier) {
    // TODO: Do we need to handle wrapped command buffers?
    // SkASSERT(!this->isWrapped());
    SkASSERT(fActive);
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
                    this->submitPipelineBarriers();
                    break;
                }
            }
        }
        fImageBarriers.push_back(*barrierPtr);
    }
    fBarriersByRegion |= byRegion;
    fSrcStageMask = fSrcStageMask | srcStageMask;
    fDstStageMask = fDstStageMask | dstStageMask;

    if (resource) {
        this->trackResource(sk_ref_sp(resource));
    }
    if (fActiveRenderPass) {
        this->submitPipelineBarriers(true);
    }
}

void VulkanCommandBuffer::submitPipelineBarriers(bool forSelfDependency) {
    SkASSERT(fActive);

    // TODO: Do we need to handle SecondaryCommandBuffers as well?

    // Currently we never submit a pipeline barrier without at least one buffer or image barrier.
    if (fBufferBarriers.size() || fImageBarriers.size()) {
        // For images we can have barriers inside of render passes but they require us to add more
        // support in subpasses which need self dependencies to have barriers inside them. Also, we
        // can never have buffer barriers inside of a render pass. For now we will just assert that
        // we are not in a render pass.
        SkASSERT(!fActiveRenderPass || forSelfDependency);
        // TODO: Do we need to handle wrapped CommandBuffers?
        //  SkASSERT(!this->isWrapped());
        SkASSERT(fSrcStageMask && fDstStageMask);

        VkDependencyFlags dependencyFlags = fBarriersByRegion ? VK_DEPENDENCY_BY_REGION_BIT : 0;
        VULKAN_CALL(fSharedContext->interface(),
                    CmdPipelineBarrier(fPrimaryCommandBuffer, fSrcStageMask, fDstStageMask,
                                       dependencyFlags,
                                       /*memoryBarrierCount=*/0, /*pMemoryBarrier=*/nullptr,
                                       fBufferBarriers.size(), fBufferBarriers.begin(),
                                       fImageBarriers.size(), fImageBarriers.begin()));
        fBufferBarriers.clear();
        fImageBarriers.clear();
        fBarriersByRegion = false;
        fSrcStageMask = 0;
        fDstStageMask = 0;
    }
    SkASSERT(!fBufferBarriers.size());
    SkASSERT(!fImageBarriers.size());
    SkASSERT(!fBarriersByRegion);
    SkASSERT(!fSrcStageMask);
    SkASSERT(!fDstStageMask);
}


} // namespace skgpu::graphite

