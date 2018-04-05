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
    , fData(nullptr) {
    nxt::BufferUsageBit usage = GrBufferTypeToNXTUsageBit(type);
    fBuffer = getNXTGpu()->device().CreateBufferBuilder()
        .SetAllowedUsage(usage | nxt::BufferUsageBit::TransferDst)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .SetSize(sizeInBytes)
        .GetResult();
    this->registerWithCache(SkBudgeted::kYes);
}

GrNXTBuffer::~GrNXTBuffer() {
    delete[] fData;
}

void GrNXTBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }
    fData = new char[sizeInBytes()];
    fMapPtr = fData;
}

void GrNXTBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }
    fBuffer.TransitionUsage(nxt::BufferUsageBit::TransferDst);
    fBuffer.SetSubData(0, sizeInBytes(), reinterpret_cast<const uint32_t*>(fData));
    delete[] fData;
    fData = nullptr;
}

bool GrNXTBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (this->wasDestroyed()) {
        return false;
    }
    fBuffer.TransitionUsage(nxt::BufferUsageBit::TransferDst);
    fBuffer.SetSubData(0, srcSizeInBytes, static_cast<const uint32_t*>(src));
    return true;
}

GrNXTGpu* GrNXTBuffer::getNXTGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrNXTGpu*>(this->getGpu());
}
