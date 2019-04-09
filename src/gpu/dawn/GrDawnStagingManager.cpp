/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnStagingManager.h"

GrDawnStagingManager::GrDawnStagingManager(dawn::Device device) : fDevice(device) {
}

GrDawnStagingManager::~GrDawnStagingManager() {
    // Clean up any pending callbacks before destroying the StagingBuffers.
    while (fWaitingCount > 0) {
        fDevice.Tick();
    }
}

static void init_callback(DawnBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                          DawnCallbackUserdata userData) {
    auto stagingBuffer = reinterpret_cast<GrDawnStagingBuffer*>(userData);
    stagingBuffer->fData = data;
}

GrDawnStagingBuffer* GrDawnStagingManager::findOrCreateStagingBuffer(size_t size,
                                                                   dawn::BufferUsageBit usage) {
    size_t sizePow2 = 1;
    while (sizePow2 < size) {
        sizePow2 <<= 1;
    }
    GrDawnStagingBuffer* stagingBuffer;
    ReadyKey key(sizePow2, usage);
    auto i = fReadyPool.find(key);
    if (i != fReadyPool.end()) {
        stagingBuffer = i->second;
        fReadyPool.erase(i);
    } else {
        dawn::BufferDescriptor desc;
        desc.usage = dawn::BufferUsageBit::MapWrite | usage;
        desc.size = sizePow2;
        dawn::Buffer buffer = fDevice.CreateBuffer(&desc);
        std::unique_ptr<GrDawnStagingBuffer> b(new GrDawnStagingBuffer(this, buffer, sizePow2, usage));
        stagingBuffer = b.get();
        fBuffers.push_back(std::move(b));
        buffer.MapWriteAsync(init_callback, reinterpret_cast<dawn::CallbackUserdata>(stagingBuffer));
        while (!stagingBuffer->fData) {
            fDevice.Tick();
        }
    }
    fBusyList.push_back(stagingBuffer);
    return stagingBuffer;
}

static void callback(DawnBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                     DawnCallbackUserdata userData) {
    GrDawnStagingBuffer* buffer = reinterpret_cast<GrDawnStagingBuffer*>(userData);
    buffer->fData = data;
    if (buffer->fManager) {
        buffer->fManager->addToReadyPool(buffer);
    }
}

void GrDawnStagingManager::mapBusyList() {
    // Map all buffers on the busy list for writing. When they're no longer in flight on the GPU,
    // their callback will be called and they'll be moved to the ready pool.
    for (GrDawnStagingBuffer* buffer : fBusyList) {
        buffer->fBuffer.MapWriteAsync(callback, reinterpret_cast<dawn::CallbackUserdata>(buffer));
        fWaitingCount++;
    }
    fBusyList.clear();
}

void GrDawnStagingManager::addToReadyPool(GrDawnStagingBuffer* buffer) {
    fWaitingCount--;
    ReadyKey key(buffer->fSize, buffer->fUsage);
    fReadyPool.insert(std::pair<ReadyKey, GrDawnStagingBuffer*>(key, buffer));
}
