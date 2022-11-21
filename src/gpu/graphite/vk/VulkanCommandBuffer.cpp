/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

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

    // TODO: Remove these lines. They are only here to hide compiler warnings/errors about unused
    // member variables.
    (void) fPool;
    (void) fPrimaryCommandBuffer;
    (void) fSharedContext;
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

bool VulkanCommandBuffer::onCopyTextureToBuffer(const Texture*,
                                                SkIRect srcRect,
                                                const Buffer*,
                                                size_t bufferOffset,
                                                size_t bufferRowBytes) {
    return false;
}

bool VulkanCommandBuffer::onCopyBufferToTexture(const Buffer*,
                                                const Texture*,
                                                const BufferTextureCopyData* copyData,
                                                int count) {
    return false;
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

#ifdef SK_ENABLE_PIET_GPU
void VulkanCommandBuffer::onRenderPietScene(const skgpu::piet::Scene& scene,
                                            const Texture* target) {}
#endif

} // namespace skgpu::graphite

