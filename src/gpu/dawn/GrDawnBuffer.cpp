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
                return wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite |
                       wgpu::BufferUsage::CopySrc;
            case GrGpuBufferType::kXferGpuToCpu:
                return wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite |
                       wgpu::BufferUsage::CopyDst;
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
    bufferDesc.usage = GrGpuBufferTypeToDawnUsageBit(type) | wgpu::BufferUsage::CopyDst;
    fBuffer = this->getDawnGpu()->device().CreateBuffer(&bufferDesc);

    if (bufferDesc.usage & wgpu::BufferUsage::MapRead) {
        // Currently everything we create that is mappable is both ReandAndWrite supported cause we
        // don't have a way to enforce GrGpuBuffer::map call to not allow writing.
        // TODO: At least for GrGpuBufferType::kXferGpuToCpu it may be safe to assume there will
        // never be writing to the mapped buffer, but we still can't enforce that.
        fMappable = bufferDesc.usage & wgpu::BufferUsage::MapWrite ? Mappable::kReadOnly
                                                                   : Mappable::kReadAndWrite;
    }

    this->registerWithCache(SkBudgeted::kYes);
}

GrDawnBuffer::~GrDawnBuffer() {
}

static void callback(WGPUBufferMapAsyncStatus status, const void* data, uint64_t dataLength,
                     void* userdata) {
    (*reinterpret_cast<const void**>(userdata)) = data;
}

void GrDawnBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }
    if (fMappable == Mappable::kNot) {
        GrStagingBuffer::Slice slice = getGpu()->allocateStagingBufferSlice(this->size());
        fStagingBuffer = static_cast<GrDawnStagingBuffer*>(slice.fBuffer)->buffer();
        fStagingOffset = slice.fOffset;
        fMapPtr = slice.fData;
    } else {
        SkASSERT(!fStagingBuffer);
        SkASSERT(!fMapPtr);

        fBuffer.MapReadAsync(callback, &fMapPtr);
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
