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
    const int kDefaultSize = 32 * 1024;
}

GrDawnRingBuffer::GrDawnRingBuffer(GrDawnGpu* gpu, wgpu::BufferUsage usage)
    : fGpu(gpu) , fUsage(usage) {
}

GrDawnRingBuffer::~GrDawnRingBuffer() {
}

GrDawnRingBuffer::Slice GrDawnRingBuffer::allocate(int size) {
    if (!fBuffer || fOffset + size > kDefaultSize) {
        flush();
        wgpu::BufferDescriptor desc;
        desc.usage = fUsage | wgpu::BufferUsage::CopyDst;
        desc.size = kDefaultSize;
        fBuffer = fGpu->device().CreateBuffer(&desc);
        fOffset = 0;
    }

    if (!fStagingBuffer) {
        wgpu::BufferDescriptor desc;
        desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        desc.size = kDefaultSize;
        wgpu::CreateBufferMappedResult result = fGpu->device().CreateBufferMapped(&desc);
        fStagingBuffer = result.buffer;
        fData = result.data;
    }
    int offset = fOffset;
    fOffset += size;
    fOffset = GrDawnRoundRowBytes(fOffset);
    fGpu->getCopyEncoder().CopyBufferToBuffer(fStagingBuffer, offset, fBuffer, offset, size);
    return Slice(fBuffer, offset, static_cast<uint8_t*>(fData) + offset);
}

void GrDawnRingBuffer::flush() {
    if (fStagingBuffer) {
        fStagingBuffer.Unmap();
        fStagingBuffer = nullptr;   // FIXME: reuse staging buffer
        fData = nullptr;
    }
}
