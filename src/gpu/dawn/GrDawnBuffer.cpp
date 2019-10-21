/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnBuffer.h"

#include "src/gpu/dawn/GrDawnGpu.h"

namespace {
    dawn::BufferUsage GrGpuBufferTypeToDawnUsageBit(GrGpuBufferType type) {
        switch (type) {
            case GrGpuBufferType::kVertex:
                return dawn::BufferUsage::Vertex;
            case GrGpuBufferType::kIndex:
                return dawn::BufferUsage::Index;
            case GrGpuBufferType::kXferCpuToGpu:
                return dawn::BufferUsage::CopySrc;
            case GrGpuBufferType::kXferGpuToCpu:
                return dawn::BufferUsage::CopyDst;
            default:
                SkASSERT(!"buffer type not supported by Dawn");
                return dawn::BufferUsage::Vertex;
        }
    }
}

GrDawnBuffer::GrDawnBuffer(GrDawnGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                           GrAccessPattern pattern)
    : INHERITED(gpu, sizeInBytes, type, pattern)
    , fStagingBuffer(nullptr) {
    dawn::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeInBytes;
    bufferDesc.usage = GrGpuBufferTypeToDawnUsageBit(type) | dawn::BufferUsage::CopyDst;
    fBuffer = this->getDawnGpu()->device().CreateBuffer(&bufferDesc);
    this->registerWithCache(SkBudgeted::kYes);
}

GrDawnBuffer::~GrDawnBuffer() {
}

void GrDawnBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }
    fStagingBuffer = getDawnGpu()->getStagingBuffer(this->size());
    fMapPtr = fStagingBuffer->fData;
}

void GrDawnBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }
    fStagingBuffer->fBuffer.Unmap();
    fMapPtr = nullptr;
    getDawnGpu()->getCopyEncoder()
        .CopyBufferToBuffer(fStagingBuffer->fBuffer, 0, fBuffer, 0, this->size());
}

bool GrDawnBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (this->wasDestroyed()) {
        return false;
    }
    this->onMap();
    memcpy(fStagingBuffer->fData, src, srcSizeInBytes);
    this->onUnmap();
    return true;
}

GrDawnGpu* GrDawnBuffer::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}
