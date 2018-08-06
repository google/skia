/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTStagingManager.h"

GrNXTStagingManager::GrNXTStagingManager(nxt::Device device) : fDevice(device.Clone()) {
}

GrNXTStagingManager::~GrNXTStagingManager() {
    // Clean up any pending callbacks before destroying the StagingBuffers.
    while (fWaitingCount > 0) {
        fDevice.Tick();
    }
}

static void init_callback(nxtBufferMapAsyncStatus status, void* data, nxtCallbackUserdata userData) {
    auto stagingBuffer = reinterpret_cast<GrNXTStagingBuffer*>(userData);
    stagingBuffer->fData = data;
}

GrNXTStagingBuffer* GrNXTStagingManager::findOrCreateStagingBuffer(size_t size,
                                                                   nxt::BufferUsageBit usage) {
    size_t sizePow2 = 1;
    while (sizePow2 < size) {
        sizePow2 <<= 1;
    }
    GrNXTStagingBuffer* stagingBuffer;
    ReadyKey key(sizePow2, usage);
    auto i = fReadyPool.find(key);
    if (i != fReadyPool.end()) {
        stagingBuffer = i->second;
        fReadyPool.erase(i);
    } else {
        nxt::Buffer buffer = fDevice.CreateBufferBuilder()
            .SetSize(sizePow2)
            .SetAllowedUsage(nxt::BufferUsageBit::MapWrite | usage)
            .GetResult();
        std::unique_ptr<GrNXTStagingBuffer> b(new GrNXTStagingBuffer(this, buffer.Clone(), sizePow2, usage));
        stagingBuffer = b.get();
        fBuffers.push_back(std::move(b));
        buffer.MapWriteAsync(0, sizePow2, init_callback, reinterpret_cast<nxt::CallbackUserdata>(stagingBuffer));
        while (!stagingBuffer->fData) {
            fDevice.Tick();
        }
    }
    fBusyList.push_back(stagingBuffer);
    return stagingBuffer;
}

static void callback(nxtBufferMapAsyncStatus status, void* data, nxtCallbackUserdata userData) {
    GrNXTStagingBuffer* buffer = reinterpret_cast<GrNXTStagingBuffer*>(userData);
    buffer->fData = data;
    if (buffer->fManager) {
        buffer->fManager->addToReadyPool(buffer);
    }
}

void GrNXTStagingManager::mapBusyList() {
    // Map all buffers on the busy list for writing. When they're no longer in flight on the GPU,
    // their callback will be called and they'll be moved to the ready pool.
    for (GrNXTStagingBuffer* buffer : fBusyList) {
        buffer->fBuffer.MapWriteAsync(0, buffer->fSize, callback,
                                      reinterpret_cast<nxt::CallbackUserdata>(buffer));
        fWaitingCount++;
    }
    fBusyList.clear();
}

void GrNXTStagingManager::addToReadyPool(GrNXTStagingBuffer* buffer) {
    fWaitingCount--;
    ReadyKey key(buffer->fSize, buffer->fUsage);
    fReadyPool.insert(std::pair<ReadyKey, GrNXTStagingBuffer*>(key, buffer));
}
