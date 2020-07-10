/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnBuffer.h"

#include "src/gpu/dawn/GrDawnGpu.h"

namespace {
    wgpu::BufferUsage GrGpuBufferTypeToDawnUsageBit(GrGpuBufferType type) {
        switch (type) {
            case GrGpuBufferType::kVertex:
                return wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
            case GrGpuBufferType::kIndex:
                return wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
            case GrGpuBufferType::kXferCpuToGpu:
                return wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
            case GrGpuBufferType::kXferGpuToCpu:
                return wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
            default:
                SkASSERT(!"buffer type not supported by Dawn");
                return wgpu::BufferUsage::Vertex;
        }
    }
}

GrDawnBuffer::GrDawnBuffer(GrDawnGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                           GrAccessPattern pattern)
    : INHERITED(gpu, sizeInBytes, type, pattern) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeInBytes;
    bufferDesc.usage = GrGpuBufferTypeToDawnUsageBit(type);
    fBuffer = this->getDawnGpu()->device().CreateBuffer(&bufferDesc);

    this->registerWithCache(SkBudgeted::kYes);
}

GrDawnBuffer::~GrDawnBuffer() {
}

bool GrDawnBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (this->wasDestroyed()) {
        return false;
    }
    this->map();
    memcpy(fMapPtr, src, srcSizeInBytes);
    this->unmap();
    return true;
}

GrDawnGpu* GrDawnBuffer::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GrDawnGpuBuffer::GrDawnGpuBuffer(GrDawnGpu* gpu,
                                 size_t sizeInBytes,
                                 GrGpuBufferType type,
                                 GrAccessPattern pattern)
        : GrDawnBuffer(gpu, sizeInBytes, type, pattern) {
    SkASSERT(type != GrGpuBufferType::kXferCpuToGpu && type != GrGpuBufferType::kXferGpuToCpu);
}

void GrDawnGpuBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }

    GrStagingBufferManager::Slice slice =
            this->getDawnGpu()->stagingBufferManager()->allocateStagingBufferSlice(this->size());
    fStagingBuffer = static_cast<GrDawnBuffer*>(slice.fBuffer)->get();
    fStagingOffset = slice.fOffset;
    fMapPtr = slice.fOffsetMapPtr;
}

void GrDawnGpuBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }
    this->getDawnGpu()->getCopyEncoder().CopyBufferToBuffer(fStagingBuffer, fStagingOffset, fBuffer,
                                                            0, this->size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GrDawnTransferBuffer::GrDawnTransferBuffer(GrDawnGpu* gpu,
                                           size_t sizeInBytes,
                                           GrGpuBufferType type,
                                           GrAccessPattern pattern)
        : GrDawnBuffer(gpu, sizeInBytes, type, pattern) {
    SkASSERT(type == GrGpuBufferType::kXferCpuToGpu || type == GrGpuBufferType::kXferGpuToCpu);
}

void GrDawnTransferBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }

    SkASSERT(fMappedState != MappedState::kMapped && !fMapPtr);
    if (fMappedState == MappedState::kNotMapped) {
        if (this->intendedType() == GrGpuBufferType::kXferGpuToCpu) {
            this->mapReadAsync();
        } else {
            SkASSERT(this->intendedType() == GrGpuBufferType::kXferCpuToGpu);
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

void GrDawnTransferBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }
    fBuffer.Unmap();
}

static void callback_read(WGPUBufferMapAsyncStatus status,
                          const void* data,
                          uint64_t dataLength,
                          void* userData) {
    GrDawnTransferBuffer* buffer = reinterpret_cast<GrDawnTransferBuffer*>(userData);
    buffer->setMapPtr(const_cast<void*>(data));
}

static void callback_write(WGPUBufferMapAsyncStatus status,
                           void* data,
                           uint64_t dataLength,
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
