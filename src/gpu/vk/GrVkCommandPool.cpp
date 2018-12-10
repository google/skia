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

GrVkCommandPool::GrVkCommandPool(GrVkGpu* gpu, VkCommandPool commandPool)
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

void GrVkCommandPool::recycleSecondaryCommandBuffer(GrVkGpu* gpu,
                                                    GrVkSecondaryCommandBuffer* buffer) {
    SkASSERT(buffer->commandPool() == this);
    buffer->reset(gpu);
    fAvailableSecondaryBuffers.push_back(buffer);
}

GrVkGpuRTCommandBuffer* GrVkCommandPool::getCommandBuffer(
                                 GrVkGpu* gpu,
                                 GrRenderTarget* rt,
                                 GrSurfaceOrigin origin,
                                 const SkRect& bounds,
                                 const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
                                 const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    if (!fCachedRTCommandBuffer) {
        fCachedRTCommandBuffer.reset(new GrVkGpuRTCommandBuffer(gpu));
    }

    fCachedRTCommandBuffer->set(rt, origin, colorInfo, stencilInfo);
    return fCachedRTCommandBuffer.get();
}

GrVkGpuRTCommandBuffer* GrVkCommandPool::getCachedRTCommandBuffer() {
    return fCachedRTCommandBuffer.get();
}

GrVkGpuTextureCommandBuffer* GrVkCommandPool::getCommandBuffer(GrVkGpu* gpu, GrTexture* texture,
                                                               GrSurfaceOrigin origin) {
    if (!fCachedTexCommandBuffer) {
        fCachedTexCommandBuffer.reset(new GrVkGpuTextureCommandBuffer(gpu));
    }

    fCachedTexCommandBuffer->set(texture, origin);
    return fCachedTexCommandBuffer.get();
}

GrVkGpuTextureCommandBuffer* GrVkCommandPool::getCachedTextureCommandBuffer() {
    return fCachedTexCommandBuffer.get();
}

void GrVkCommandPool::close() {
    SkASSERT(!fPrimaryCommandBuffer->fIsActive);
    fOpen = false;
}

void GrVkCommandPool::reset(GrVkGpu* gpu) {
    SkASSERT(!fOpen);
    fOpen = true;
    fPrimaryCommandBuffer->reset(gpu);
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), ResetCommandPool(gpu->device(), fCommandPool, 0));

    for (GrVkSecondaryCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unref(gpu);
    }
    fAvailableSecondaryBuffers.reset();

}

void GrVkCommandPool::releaseResources(GrVkGpu* gpu) {
    SkASSERT(!fOpen);
    fPrimaryCommandBuffer->releaseResources(gpu);
    for (GrVkCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        buffer->releaseResources(gpu);
    }
}

void GrVkCommandPool::abandonGPUData() const {
    fPrimaryCommandBuffer->unrefAndAbandon();
    for (GrVkCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unrefAndAbandon();
    }
}

void GrVkCommandPool::freeGPUData(GrVkGpu* gpu) const {
    fPrimaryCommandBuffer->unref(gpu);
    if (fCommandPool != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyCommandPool(gpu->device(), fCommandPool, nullptr));
    }
}
