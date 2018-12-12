/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkCommandPool.h"

#include "GrContextPriv.h"
#include "GrVkCommandBuffer.h"
#include "GrVkGpu.h"

GrVkCommandPool* GrVkCommandPool::Create(const GrVkGpu* gpu) {
    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,      // sType
        nullptr,                                         // pNext
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // CmdPoolCreateFlags
        gpu->queueIndex(),                              // queueFamilyIndex
    };
    VkCommandPool pool;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateCommandPool(gpu->device(), &cmdPoolInfo,
                                                               nullptr, &pool));
    return new GrVkCommandPool(gpu, pool);
}

GrVkCommandPool::GrVkCommandPool(const GrVkGpu* gpu, VkCommandPool commandPool)
        : fCommandPool(commandPool) {
    fPrimaryCommandBuffer = GrVkPrimaryCommandBuffer::Create(gpu, this);
}

GrVkSecondaryCommandBuffer* GrVkCommandPool::findOrCreateSecondaryCommandBuffer(GrVkGpu* gpu) {
    if (fAvailableSecondaryBuffers.count()) {
        GrVkSecondaryCommandBuffer* result = fAvailableSecondaryBuffers.back();
        fAvailableSecondaryBuffers.pop_back();
        return result;
    }
    return GrVkSecondaryCommandBuffer::Create(gpu, this);
}

void GrVkCommandPool::recycleSecondaryCommandBuffer(GrVkSecondaryCommandBuffer* buffer) {
    SkASSERT(buffer->commandPool() == this);
    fAvailableSecondaryBuffers.push_back(buffer);
}

void GrVkCommandPool::close() {
    fOpen = false;
}

void GrVkCommandPool::reset(GrVkGpu* gpu) {
    SkASSERT(!fOpen);
    fOpen = true;
    fPrimaryCommandBuffer->recycleSecondaryCommandBuffers();
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), ResetCommandPool(gpu->device(), fCommandPool, 0));
}

void GrVkCommandPool::releaseResources(GrVkGpu* gpu) {
    SkASSERT(!fOpen);
    fPrimaryCommandBuffer->releaseResources(gpu);
    for (GrVkSecondaryCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        buffer->releaseResources(gpu);
    }
}

void GrVkCommandPool::abandonGPUData() const {
    fPrimaryCommandBuffer->unrefAndAbandon();
    for (GrVkSecondaryCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unrefAndAbandon();
    }
}

void GrVkCommandPool::freeGPUData(GrVkGpu* gpu) const {
    fPrimaryCommandBuffer->unref(gpu);
    for (GrVkSecondaryCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unref(gpu);
    }
    if (fCommandPool != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyCommandPool(gpu->device(), fCommandPool, nullptr));
    }
}
