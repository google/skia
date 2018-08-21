/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTRingBuffer.h"
#include "GrNXTGpu.h"

namespace {
    const int kDefaultSize = 512 * 1024;
}

GrNXTRingBuffer::GrNXTRingBuffer(GrNXTGpu* gpu, dawn::BufferUsageBit usage)
    : fGpu(gpu) , fUsage(usage) {
}

GrNXTRingBuffer::~GrNXTRingBuffer() {
}

GrNXTRingBuffer::Slice GrNXTRingBuffer::allocate(int size) {
    if (!fBuffer || fOffset + size > kDefaultSize) {
        fBuffer = fGpu->device().CreateBufferBuilder()
            .SetAllowedUsage(fUsage | dawn::BufferUsageBit::TransferDst)
            .SetSize(kDefaultSize)
            .GetResult();
        fOffset = 0;
    }
    int offset = fOffset;
    fOffset += size;
    fOffset = (fOffset + 255) & ~255;
    return Slice(fBuffer.Clone(), offset);
}
