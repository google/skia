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

    if (bufferDesc.usage & wgpu::BufferUsage::MapRead) {
        SkASSERT(!SkToBool(bufferDesc.usage & wgpu::BufferUsage::MapWrite));
        fMappable = Mappable::kReadOnly;
    } else if (bufferDesc.usage & wgpu::BufferUsage::MapWrite) {
        fMappable = Mappable::kWriteOnly;
    }

    if (fMappable == Mappable::kNot || fMappable == Mappable::kReadOnly) {
        fBuffer = this->getDawnGpu()->device().CreateBuffer(&bufferDesc);
    } else {
        bufferDesc.mappedAtCreation = true;
        fBuffer = this->getDawnGpu()->device().CreateBuffer(&bufferDesc);
        fMapPtr = fBuffer.GetMappedRange();
    }

    this->registerWithCache(SkBudgeted::kYes);
}

GrDawnBuffer::~GrDawnBuffer() {
}

void GrDawnBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }

    if (fMappable == Mappable::kNot) {
        GrStagingBufferManager::Slice slice =
                this->getDawnGpu()->stagingBufferManager()->allocateStagingBufferSlice(
                        this->size());
        fStagingBuffer = static_cast<GrDawnBuffer*>(slice.fBuffer)->get();
        fStagingOffset = slice.fOffset;
        fMapPtr = slice.fOffsetMapPtr;
    } else {
        // We always create this buffers mapped or if they've been used on the gpu before we use the
        // async map callback to know when it is safe to reuse them. Thus by the time we get here
        // the buffer should always be mapped.
        SkASSERT(this->isMapped());
    }
}

void GrDawnBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }

    if (fMappable == Mappable::kNot) {
        this->getDawnGpu()->getCopyEncoder().CopyBufferToBuffer(fStagingBuffer, fStagingOffset,
                                                                fBuffer, 0, this->size());
    } else {
        fBuffer.Unmap();
    }
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

static void callback_read(WGPUBufferMapAsyncStatus status, void* userData) {
    auto buffer = static_cast<GrDawnBuffer*>(userData);
    buffer->setMapPtr(const_cast<void*>(buffer->get().GetConstMappedRange()));
}

static void callback_write(WGPUBufferMapAsyncStatus status, void* userData) {
    auto buffer = static_cast<GrDawnBuffer*>(userData);
    buffer->setMapPtr(buffer->get().GetMappedRange());
}

void GrDawnBuffer::mapWriteAsync() {
    SkASSERT(!this->isMapped());
    fBuffer.MapAsync(wgpu::MapMode::Write, 0, 0, callback_write, this);
}
void GrDawnBuffer::mapReadAsync() {
    SkASSERT(!this->isMapped());
    fBuffer.MapAsync(wgpu::MapMode::Read, 0, 0, callback_read, this);
}
