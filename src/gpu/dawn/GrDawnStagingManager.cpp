/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnStagingManager.h"

#include "src/core/SkMathPriv.h"

namespace {
    const size_t kMinSize = 32 * 1024;
}

GrDawnStagingManager::GrDawnStagingManager(wgpu::Device device) : fDevice(device) {
}

GrDawnStagingManager::~GrDawnStagingManager() {
    // Clean up any pending callbacks before destroying the StagingBuffers.
    while (fWaitingCount > 0) {
        fDevice.Tick();
    }
}

GrDawnStagingBuffer* GrDawnStagingManager::find(size_t size) {
    for (const auto &b : fBuffers) {
        if (!b->fBusy && (b->fSize - b->fOffset) >= size) {
            return b.get();
        }
    }
    return nullptr;
}

GrDawnStagingBuffer::Slice GrDawnStagingManager::allocate(size_t size) {
    size_t sizePow2 = GrNextPow2(size);
    GrDawnStagingBuffer* stagingBuffer = find(size);
    if (!stagingBuffer) {
        wgpu::BufferDescriptor desc;
        desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        desc.size = std::max(sizePow2, kMinSize);
        wgpu::CreateBufferMappedResult result = fDevice.CreateBufferMapped(&desc);
        std::unique_ptr<GrDawnStagingBuffer> b(new GrDawnStagingBuffer(
            this, result.buffer, desc.size, result.data));
        stagingBuffer = b.get();
        fBuffers.push_back(std::move(b));
    }
    size_t offset = stagingBuffer->fOffset;
    stagingBuffer->fOffset += size;
    char* data = static_cast<char*>(stagingBuffer->fData) + offset;
    return GrDawnStagingBuffer::Slice(stagingBuffer->fBuffer, offset, data);
}

static void callback(WGPUBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                     void* userData) {
    GrDawnStagingBuffer* buffer = static_cast<GrDawnStagingBuffer*>(userData);
    buffer->fData = data;
    buffer->fOffset = 0;
    if (buffer->fManager) {
        buffer->fManager->addToReadyPool(buffer);
    }
}

void GrDawnStagingManager::flush() {
    // Map all buffers on the busy list for writing. When they're no longer in flight on the GPU,
    // their callback will be called and they'll be moved to the ready pool.
    for (const auto &b : fBuffers) {
        GrDawnStagingBuffer* buffer = b.get();
        if (buffer->fOffset > 0 && !buffer->fBusy) {
            buffer->fBuffer.Unmap();
        }
    }
}

void GrDawnStagingManager::mapBusyList() {
    // Map all buffers on the busy list for writing. When they're no longer in flight on the GPU,
    // their callback will be called and they'll be moved to the ready pool.
    for (const auto &b : fBuffers) {
        GrDawnStagingBuffer* buffer = b.get();
        if (buffer->fOffset > 0 && !buffer->fBusy) {
            buffer->fBuffer.MapWriteAsync(callback, buffer);
            buffer->fBusy = true;
            fWaitingCount++;
        }
    }
}

void GrDawnStagingManager::addToReadyPool(GrDawnStagingBuffer* buffer) {
    fWaitingCount--;
    buffer->fBusy = false;
}
