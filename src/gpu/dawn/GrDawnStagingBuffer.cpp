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
    buffer->markAvailable(data);
}

GrDawnGpu* GrDawnStagingBuffer::getDawnGpu() const {
    return static_cast<GrDawnGpu*>(this->getGpu());
}

void GrDawnStagingBuffer::mapAsync() {
    fBuffer.MapWriteAsync(callback, this);
}

void GrDawnStagingBuffer::onUnmap() {
    fBuffer.Unmap();
}
