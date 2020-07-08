/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnStagingBuffer.h"

#include "src/core/SkMathPriv.h"

static void callback(WGPUBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                     void* userData) {
    GrDawnStagingBuffer* buffer = static_cast<GrDawnStagingBuffer*>(userData);
    buffer->markAvailable(data);
}

GrDawnGpu* GrDawnStagingBuffer::getDawnGpu() const {
    return static_cast<GrDawnGpu*>(this->getGpu());
}

void GrDawnStagingBuffer::mapAsync() {
    fBuffer.MapWriteAsync(callback, this);
}

void GrDawnStagingBuffer::onUnmap() {
    fBuffer.Unmap();
}

GrDawnTransferBuffer::GrDawnTransferBuffer(GrDawnGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                                           GrAccessPattern pattern)
        : GrDawnBuffer(gpu, sizeInBytes, type, pattern) {
    SkASSERT(fMappable != Mappable::kNot);
}

void GrDawnTransferBuffer::onMap() {
    SkASSERT(fMappedState != MappedState::kMapped && !fMapPtr);
    if (fMappedState == MappedState::kNotMapped) {
        if (fMappable == Mappable::kReadOnly) {
            this->mapReadAsync();
        } else {
            this->mapWriteAsync();
        }
    }
    // We shouldn't be sitting in this loop for long since we will rarely if ever even try to
    // map a GrGpuBuffer that is still in use on the GPU.
    while (!fMapPtr) {
        SkASSERT(fMappedState == MappedState::kMapPending);
        this->getDawnGpu()->device().Tick();
    }
    SkASSERT(fMappedState == MappedState::kMapped);
}

static void callback_read(WGPUBufferMapAsyncStatus status, const void* data, uint64_t dataLength,
                          void* userData) {
    GrDawnTransferBuffer* buffer = reinterpret_cast<GrDawnTransferBuffer*>(userData);
    buffer->setMapPtr(const_cast<void*>(data));
}

static void callback_write(WGPUBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                           void* userData) {
    GrDawnTransferBuffer* buffer = reinterpret_cast<GrDawnTransferBuffer*>(userData);
    buffer->setMapPtr(data);
}

void GrDawnTransferBuffer::mapWriteAsync() {
    SkASSERT(fMappedState == MappedState::kNotMapped);
    fMappedState = MappedState::kMapPending;
    fBuffer.MapWriteAsync(callback_write, this);
}
void GrDawnTransferBuffer::mapReadAsync() {
    SkASSERT(fMappedState == MappedState::kNotMapped);
    fMappedState = MappedState::kMapPending;
    fBuffer.MapReadAsync(callback_read, this);
}
