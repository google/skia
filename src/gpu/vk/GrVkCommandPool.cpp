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

GrVkPrimaryCommandBuffer* GrVkCommandPool::findOrCreatePrimaryCommandBuffer(GrVkGpu* gpu) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkPrimaryCommandBuffer", "findOrCreatePrimaryCommandBuffer", gpu->getContext());
    std::unique_lock<std::recursive_mutex> lock(fMutex);
    SkASSERT(fOpen);
    GrVkPrimaryCommandBuffer* result;
    if (fAvailablePrimaryBuffers.count()) {
        result = fAvailablePrimaryBuffers.back();
        fAvailablePrimaryBuffers.pop_back();
    }
    else {
        result = GrVkPrimaryCommandBuffer::Create(gpu, this);
    }
    fActivePrimaryBuffers.push_back(result);
    result->ref();
    result->begin(gpu);
    return result;
}

GrVkSecondaryCommandBuffer* GrVkCommandPool::findOrCreateSecondaryCommandBuffer(GrVkGpu* gpu) {
    std::unique_lock<std::recursive_mutex> lock(fMutex);
    if (fAvailableSecondaryBuffers.count()) {
        GrVkSecondaryCommandBuffer* result = fAvailableSecondaryBuffers.back();
        fAvailableSecondaryBuffers.pop_back();
        return result;
    }
    return GrVkSecondaryCommandBuffer::Create(gpu, this);
}

void GrVkCommandPool::recyclePrimaryCommandBuffer(GrVkGpu* gpu,
                                                  GrVkPrimaryCommandBuffer* buffer) {
    std::unique_lock<std::recursive_mutex> lock(fMutex);
    SkASSERT(buffer->commandPool() == this);
    buffer->reset(gpu);
    fAvailablePrimaryBuffers.push_back(buffer);
}

void GrVkCommandPool::recycleSecondaryCommandBuffer(GrVkGpu* gpu,
                                                    GrVkSecondaryCommandBuffer* buffer) {
    std::unique_lock<std::recursive_mutex> lock(fMutex);
    SkASSERT(buffer->commandPool() == this);
    buffer->reset(gpu);
    fAvailableSecondaryBuffers.push_back(buffer);
}

void GrVkCommandPool::checkCommandBuffers(GrVkResourceProvider* provider, GrVkGpu* gpu) {
    for (int i = fActivePrimaryBuffers.count() - 1; i >= 0; --i) {
        if (fActivePrimaryBuffers[i]->finished(gpu)) {
            GrVkPrimaryCommandBuffer* cmdBuffer = fActivePrimaryBuffers[i];
            fActivePrimaryBuffers.removeShuffle(i);
            provider->backgroundReset(cmdBuffer);
        }
    }
}

void GrVkCommandPool::close() {
    fOpen = false;
}

bool GrVkCommandPool::isFinished() const {
    return !fOpen && fActivePrimaryBuffers.count() == 0;
}

void GrVkCommandPool::reset(GrVkGpu* gpu) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkCommandPool", "reset", gpu->getContext());
    SkASSERT(this->isFinished());
    fOpen = true;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), ResetCommandPool(gpu->device(), fCommandPool, 0));

    for (GrVkPrimaryCommandBuffer* buffer : fActivePrimaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unref(gpu);
    }
    fActivePrimaryBuffers.reset();

    for (GrVkPrimaryCommandBuffer* buffer : fAvailablePrimaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unref(gpu);
    }
    fAvailablePrimaryBuffers.reset();

    for (GrVkSecondaryCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unref(gpu);
    }
    fAvailableSecondaryBuffers.reset();

}

void GrVkCommandPool::releaseResources(GrVkGpu* gpu) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkCommandPool", "releaseResources", gpu->getContext());
    SkASSERT(this->isFinished());
    for (GrVkCommandBuffer* buffer : fAvailablePrimaryBuffers) {
        buffer->releaseResources(gpu);
    }
    for (GrVkCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        buffer->releaseResources(gpu);
    }
}

void GrVkCommandPool::abandonGPUData() const {
    for (GrVkPrimaryCommandBuffer* buffer : fActivePrimaryBuffers) {
        buffer->unrefAndAbandon();
    }
    for (GrVkCommandBuffer* buffer : fAvailablePrimaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unrefAndAbandon();
    }
    for (GrVkCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unrefAndAbandon();
    }
}

void GrVkCommandPool::freeGPUData(GrVkGpu* gpu) const {
    SkASSERT(fActivePrimaryBuffers.count() == 0);
    if (fCommandPool != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyCommandPool(gpu->device(), fCommandPool, nullptr));
    }
}
