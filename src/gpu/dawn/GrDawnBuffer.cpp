/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnBuffer.h"
#include "src/gpu/dawn/GrDawnStagingBuffer.h"

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

    if (bufferDesc.usage & wgpu::BufferUsage::MapRead) {
        SkASSERT(!SkToBool(bufferDesc.usage & wgpu::BufferUsage::MapWrite));
        fMappable = Mappable::kReadOnly;
    } else if (bufferDesc.usage & wgpu::BufferUsage::MapWrite) {
        fMappable = Mappable::kWriteOnly;
    }

    this->registerWithCache(SkBudgeted::kYes);
}

GrDawnBuffer::~GrDawnBuffer() {
}

static void callback_read(WGPUBufferMapAsyncStatus status, const void* data, uint64_t dataLength,
                     void* userData) {
    (*reinterpret_cast<const void**>(userData)) = data;
}

static void callback_write(WGPUBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                           void* userData) {
    (*reinterpret_cast<const void**>(userData)) = data;
}

void GrDawnBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }
    if (fMappable == Mappable::kNot) {
        GrStagingBufferManager::Slice slice =
            this->getDawnGpu()->stagingBufferManager()->allocateStagingBufferSlice(this->size());
        fStagingBuffer = static_cast<GrDawnBuffer*>(slice.fBuffer)->get();
        fStagingOffset = slice.fOffset;
        fMapPtr = slice.fOffsetMapPtr;
    } else {
        SkASSERT(!fStagingBuffer);
        SkASSERT(!fMapPtr);

        if (fMappable == Mappable::kReadOnly) {
            fBuffer.MapReadAsync(callback_read, &fMapPtr);
        } else {
            fBuffer.MapWriteAsync(callback_write, &fMapPtr);
        }
        // We shouldn't be sitting in this loop for long since we will rarely if ever even try to
        // map a GrGpuBuffer that is still in use on the GPU.
        while (!fMapPtr) {
            this->getDawnGpu()->device().Tick();
        }
        fStagingOffset = 0;
    }
}

void GrDawnBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }
    fMapPtr = nullptr;
    if (fMappable == Mappable::kNot) {
        this->getDawnGpu()->getCopyEncoder()
            .CopyBufferToBuffer(fStagingBuffer, fStagingOffset, fBuffer, 0, this->size());
    } else {
        fBuffer.Unmap();
    }
}

bool GrDawnBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (this->wasDestroyed()) {
        return false;
    }
    this->onMap();
    memcpy(fMapPtr, src, srcSizeInBytes);
    this->onUnmap();
    return true;
}

GrDawnGpu* GrDawnBuffer::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}
