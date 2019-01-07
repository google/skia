/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnRingBuffer.h"
#include "GrDawnGpu.h"

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
        desc.usage = fUsage | dawn::BufferUsageBit::TransferDst;
        desc.size = kDefaultSize;
        fBuffer = fGpu->device().CreateBuffer(&desc);
        fOffset = 0;
    }
    int offset = fOffset;
    fOffset += size;
    fOffset = (fOffset + 255) & ~255;
    return Slice(fBuffer, offset);
}
