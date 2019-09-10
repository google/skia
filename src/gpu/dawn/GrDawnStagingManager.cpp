/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnStagingManager.h"

#include "src/core/SkMathPriv.h"

GrDawnStagingManager::GrDawnStagingManager(dawn::Device device) : fDevice(device) {
}

GrDawnStagingManager::~GrDawnStagingManager() {
    // Clean up any pending callbacks before destroying the StagingBuffers.
    while (fWaitingCount > 0) {
        fDevice.Tick();
    }
}

GrDawnStagingBuffer* GrDawnStagingManager::findOrCreateStagingBuffer(size_t size) {
    size_t sizePow2 = GrNextPow2(size);
    GrDawnStagingBuffer* stagingBuffer;
    auto i = fReadyPool.find(sizePow2);
    if (i != fReadyPool.end()) {
        stagingBuffer = i->second;
        fReadyPool.erase(i);
    } else {
        dawn::BufferDescriptor desc;
        desc.usage = dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::CopySrc;
        desc.size = sizePow2;
        dawn::CreateBufferMappedResult result = fDevice.CreateBufferMapped(&desc);
        std::unique_ptr<GrDawnStagingBuffer> b(new GrDawnStagingBuffer(
            this, result.buffer, sizePow2, result.data));
        stagingBuffer = b.get();
        fBuffers.push_back(std::move(b));
    }
    fBusyList.push_back(stagingBuffer);
    return stagingBuffer;
}

static void callback(DawnBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                     void* userData) {
    GrDawnStagingBuffer* buffer = static_cast<GrDawnStagingBuffer*>(userData);
    buffer->fData = data;
    if (buffer->fManager) {
        buffer->fManager->addToReadyPool(buffer);
    }
}

void GrDawnStagingManager::mapBusyList() {
    // Map all buffers on the busy list for writing. When they're no longer in flight on the GPU,
    // their callback will be called and they'll be moved to the ready pool.
    for (GrDawnStagingBuffer* buffer : fBusyList) {
        buffer->fBuffer.MapWriteAsync(callback, buffer);
        fWaitingCount++;
    }
    fBusyList.clear();
}

void GrDawnStagingManager::addToReadyPool(GrDawnStagingBuffer* buffer) {
    fWaitingCount--;
    fReadyPool.insert(std::pair<size_t, GrDawnStagingBuffer*>(buffer->fSize, buffer));
}
