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

void GrDawnStagingBuffer::mapAsync() {
    if (fNeedsToBeMapped) {
        fBuffer.MapWriteAsync(callback, this);
        fNeedsToBeMapped = false;
    }
}

void GrDawnStagingBuffer::unmap() {
    fBuffer.Unmap();
    fNeedsToBeMapped = true;
}

void GrDawnStagingBuffer::markAvailable(void* mapPtr) {
    std::unique_ptr<GrStagingBuffer> buffer = fManager->removeFromBusy(this);
    SkASSERT(buffer);
    fManager->recycleStagingBuffer(std::move(buffer), mapPtr);
}
