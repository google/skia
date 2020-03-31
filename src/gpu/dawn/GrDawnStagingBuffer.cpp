/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnStagingBuffer.h"

#include "src/core/SkMathPriv.h"

static void callback(WGPUBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                     void* userData) {
    GrDawnStagingBuffer* buffer = static_cast<GrDawnStagingBuffer*>(userData);
    buffer->markReady(data);
}

void GrDawnStagingBuffer::markReady(void* data) {
    fData = data;
    fOffset = 0;
    fBusy = false;
    if (fGpu) {
        fGpu->markStagingBufferReady();
    }
}

void GrDawnStagingBuffer::mapAsync() {
    fBuffer.MapWriteAsync(callback, this);
    fBusy = true;
}

GrDawnStagingBuffer::Slice GrDawnStagingBuffer::allocate(size_t size) {
    size_t offset = fOffset;
    fOffset += size;
    char* data = static_cast<char*>(fData) + offset;
    return Slice(fBuffer, offset, data);
}
