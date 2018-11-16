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
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkCommandPool", "findOrCreatePrimary", gpu->getContext());
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
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkCommandPool", "findOrCreateSecondary", gpu->getContext());
    std::unique_lock<std::recursive_mutex> lock(fMutex);
    SkASSERT(fOpen);
    if (fAvailableSecondaryBuffers.count()) {
        GrVkSecondaryCommandBuffer* result = fAvailableSecondaryBuffers.back();
        fAvailableSecondaryBuffers.pop_back();
        return result;
    }
    else {
        return GrVkSecondaryCommandBuffer::Create(gpu, this);
    }
}

void GrVkCommandPool::recyclePrimaryCommandBuffer(GrVkGpu* gpu,
                                                  GrVkPrimaryCommandBuffer* buffer) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkCommandPool", "recyclePrimary", gpu->getContext());
    std::unique_lock<std::recursive_mutex> lock(fMutex);
    buffer->reset(gpu);
    fAvailablePrimaryBuffers.push_back(buffer);
}

void GrVkCommandPool::recycleSecondaryCommandBuffer(GrVkGpu* gpu,
                                                    GrVkSecondaryCommandBuffer* buffer) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkCommandPool", "recycleSecondary", gpu->getContext());
    std::unique_lock<std::recursive_mutex> lock(fMutex);
    buffer->reset(gpu);
    fAvailableSecondaryBuffers.push_back(buffer);
}

void GrVkCommandPool::checkCommandBuffers(GrVkResourceProvider* provider, GrVkGpu* gpu) {
    for (int i = fActivePrimaryBuffers.count() - 1; i >= 0; --i) {
        if (fActivePrimaryBuffers[i]->finished(gpu)) {
            GrVkPrimaryCommandBuffer* cmdBuffer = fActivePrimaryBuffers[i];
            provider->backgroundReset(cmdBuffer);
            fActivePrimaryBuffers.removeShuffle(i);
        }
    }
}

void GrVkCommandPool::close() {
    fOpen = false;
}

bool GrVkCommandPool::finished() const {
    return !fOpen && fActivePrimaryBuffers.count() == 0;
}

void GrVkCommandPool::reset(GrVkGpu* gpu) {
    SkASSERT(this->finished());
    fOpen = true;
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkCommandPool", "reset", gpu->getContext());
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), ResetCommandPool(gpu->device(), fCommandPool, 0));
}

void GrVkCommandPool::releaseIdleBuffers(GrVkResourceProvider* provider, GrVkGpu* gpu) {
    this->checkCommandBuffers(provider, gpu);
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

void GrVkCommandPool::abandonGPUData() const {
    for (GrVkPrimaryCommandBuffer* buffer : fActivePrimaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unrefAndAbandon();
    }
    fActivePrimaryBuffers.reset();

    for (GrVkPrimaryCommandBuffer* buffer : fAvailablePrimaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unrefAndAbandon();
    }
    fAvailablePrimaryBuffers.reset();

    for (GrVkSecondaryCommandBuffer* buffer : fAvailableSecondaryBuffers) {
        SkASSERT(buffer->unique());
        buffer->unrefAndAbandon();
    }
    fAvailableSecondaryBuffers.reset();

    fCommandPool = VK_NULL_HANDLE;
}

void GrVkCommandPool::freeGPUData(GrVkGpu* gpu) const {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrVkCommandPool", "freeGPUData", gpu->getContext());
    SkASSERT(fCommandPool != VK_NULL_HANDLE);
    SkASSERT(fActivePrimaryBuffers.count() == 0);
    SkASSERT(fAvailablePrimaryBuffers.count() == 0);
    SkASSERT(fAvailableSecondaryBuffers.count() == 0);

    if (fCommandPool != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyCommandPool(gpu->device(), fCommandPool, nullptr));
        fCommandPool = VK_NULL_HANDLE;
    }
}
