/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkCommandPool.h"

#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkGpu.h"

GrVkCommandPool* GrVkCommandPool::Create(GrVkGpu* gpu) {
    VkCommandPoolCreateFlags cmdPoolCreateFlags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (gpu->protectedContext()) {
        cmdPoolCreateFlags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    }

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
        nullptr,                                     // pNext
        cmdPoolCreateFlags,                          // CmdPoolCreateFlags
        gpu->queueIndex(),                           // queueFamilyIndex
    };
    VkResult result;
    VkCommandPool pool;
    GR_VK_CALL_RESULT(gpu, result, CreateCommandPool(gpu->device(), &cmdPoolInfo, nullptr, &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    GrVkPrimaryCommandBuffer* primaryCmdBuffer = GrVkPrimaryCommandBuffer::Create(gpu, pool);
    if (!primaryCmdBuffer) {
        GR_VK_CALL(gpu->vkInterface(), DestroyCommandPool(gpu->device(), pool, nullptr));
        return nullptr;
    }

    return new GrVkCommandPool(gpu, pool, primaryCmdBuffer);
}

GrVkCommandPool::GrVkCommandPool(GrVkGpu* gpu, VkCommandPool commandPool,
                                 GrVkPrimaryCommandBuffer* primaryCmdBuffer)
        : GrVkManagedResource(gpu)
        , fCommandPool(commandPool)
        , fPrimaryCommandBuffer(primaryCmdBuffer)
        , fMaxCachedSecondaryCommandBuffers(
                gpu->vkCaps().maxPerPoolCachedSecondaryCommandBuffers()) {
}

std::unique_ptr<GrVkSecondaryCommandBuffer> GrVkCommandPool::findOrCreateSecondaryCommandBuffer(
        GrVkGpu* gpu) {
    std::unique_ptr<GrVkSecondaryCommandBuffer> result;
    if (fAvailableSecondaryBuffers.count()) {
        result = std::move(fAvailableSecondaryBuffers.back());
        fAvailableSecondaryBuffers.pop_back();
    } else{
        result.reset(GrVkSecondaryCommandBuffer::Create(gpu, this));
    }
    return result;
}

void GrVkCommandPool::recycleSecondaryCommandBuffer(GrVkSecondaryCommandBuffer* buffer) {
    std::unique_ptr<GrVkSecondaryCommandBuffer> scb(buffer);
    if (fAvailableSecondaryBuffers.count() < fMaxCachedSecondaryCommandBuffers) {
        fAvailableSecondaryBuffers.push_back(std::move(scb));
    } else {
        VkCommandBuffer vkBuffer = buffer->vkCommandBuffer();
        GR_VK_CALL(fGpu->vkInterface(),
                   FreeCommandBuffers(fGpu->device(), fCommandPool, 1, &vkBuffer));
    }
}

void GrVkCommandPool::close() {
    fOpen = false;
}

void GrVkCommandPool::reset(GrVkGpu* gpu) {
    SkASSERT(!fOpen);
    fOpen = true;
    // We can't use the normal result macro calls here because we may call reset on a different
    // thread and we can't be modifying the lost state on the GrVkGpu. We just call
    // vkResetCommandPool and assume the "next" vulkan call will catch the lost device.
    SkDEBUGCODE(VkResult result = )GR_VK_CALL(gpu->vkInterface(),
                                              ResetCommandPool(gpu->device(), fCommandPool, 0));
    SkASSERT(result == VK_SUCCESS || result == VK_ERROR_DEVICE_LOST);
}

void GrVkCommandPool::releaseResources() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(!fOpen);
    fPrimaryCommandBuffer->releaseResources();
    fPrimaryCommandBuffer->recycleSecondaryCommandBuffers(this);
}

void GrVkCommandPool::freeGPUData() const {
    // TODO: having freeGPUData virtual on GrManagedResource be const seems like a bad restriction since
    // we are changing the internal objects of these classes when it is called. We should go back a
    // revisit how much of a headache it would be to make this function non-const
    GrVkCommandPool* nonConstThis = const_cast<GrVkCommandPool*>(this);
    nonConstThis->close();
    nonConstThis->releaseResources();
    fPrimaryCommandBuffer->freeGPUData(fGpu, fCommandPool);
    for (const auto& buffer : fAvailableSecondaryBuffers) {
        buffer->freeGPUData(fGpu, fCommandPool);
    }
    if (fCommandPool != VK_NULL_HANDLE) {
        GR_VK_CALL(fGpu->vkInterface(),
                   DestroyCommandPool(fGpu->device(), fCommandPool, nullptr));
    }
}
