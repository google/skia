/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTBuffer.h"
#include "GrNXTGpu.h"

namespace {
    nxt::BufferUsageBit GrBufferTypeToNXTUsageBit(GrBufferType type) {
        switch (type) {
            case GrBufferType::kVertex_GrBufferType:
                return nxt::BufferUsageBit::Vertex;
            case GrBufferType::kIndex_GrBufferType:
                return nxt::BufferUsageBit::Index;
            case GrBufferType::kXferCpuToGpu_GrBufferType:
                return nxt::BufferUsageBit::TransferSrc;
            case GrBufferType::kXferGpuToCpu_GrBufferType:
                return nxt::BufferUsageBit::TransferDst;
            case GrBufferType::kDrawIndirect_GrBufferType:
            case GrBufferType::kTexel_GrBufferType:
            default:
                SkASSERT(!"buffer type not supported by NXT");
                return nxt::BufferUsageBit::Vertex;
        }
    }
}

GrNXTBuffer::GrNXTBuffer(GrNXTGpu* gpu, size_t sizeInBytes, GrBufferType type,
                         GrAccessPattern pattern)
    : INHERITED(gpu, sizeInBytes, type, pattern)
    , fStagingBuffer(nullptr) {
    nxt::BufferUsageBit usage = GrBufferTypeToNXTUsageBit(type);
    fBuffer = getNXTGpu()->device().CreateBufferBuilder()
        .SetAllowedUsage(usage | nxt::BufferUsageBit::TransferDst)
        .SetSize(sizeInBytes)
        .GetResult();
    this->registerWithCache(SkBudgeted::kYes);
}

GrNXTBuffer::~GrNXTBuffer() {
}

void GrNXTBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }
    fStagingBuffer = getNXTGpu()->getStagingBuffer(sizeInBytes());
    fMapPtr = fStagingBuffer->fData;
}

void GrNXTBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }
    fStagingBuffer->fBuffer.Unmap();
    fMapPtr = nullptr;
    getNXTGpu()->getCopyBuilder()
        .CopyBufferToBuffer(fStagingBuffer->fBuffer, 0, fBuffer, 0, sizeInBytes());
}

bool GrNXTBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (this->wasDestroyed()) {
        return false;
    }
    this->onMap();
    memcpy(fStagingBuffer->fData, src, srcSizeInBytes);
    this->onUnmap();
    return true;
}

GrNXTGpu* GrNXTBuffer::getNXTGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrNXTGpu*>(this->getGpu());
}
