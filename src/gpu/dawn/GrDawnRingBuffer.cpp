/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnRingBuffer.h"

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnUtil.h"

namespace {
    const int kDefaultSize = 512 * 1024;
}

GrDawnRingBuffer::GrDawnRingBuffer(GrDawnGpu* gpu, dawn::BufferUsageBit usage)
    : fGpu(gpu) , fUsage(usage) {
}

GrDawnRingBuffer::~GrDawnRingBuffer() {
}

GrDawnRingBuffer::Slice GrDawnRingBuffer::allocate(int size) {
    if (!fBuffer || fOffset + size > kDefaultSize) {
        dawn::BufferDescriptor desc;
        desc.usage = fUsage | dawn::BufferUsageBit::CopyDst;
        desc.size = kDefaultSize;
        fBuffer = fGpu->device().CreateBuffer(&desc);
        fOffset = 0;
    }
    int offset = fOffset;
    fOffset += size;
    fOffset = GrDawnRoundRowBytes(fOffset);
    return Slice(fBuffer, offset);
}
