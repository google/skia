/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrStagingBufferManager.h"

#include "src/core/SkMathPriv.h"

#include <algorithm>

GrStagingBuffer::Slice GrStagingBufferManager::allocateStagingBufferSlice(size_t size) {
    GrStagingBuffer* buffer = nullptr;
    for (size_t i = 0; i < fActiveBuffers.size() && !buffer; ++i) {
        if (fActiveBuffers[i]->remaining() >= size) {
            buffer = fActiveBuffers[i].get();
        }
    }
    if (!buffer) {
        for (size_t i = 0; i < fAvailableBuffers.size() && !buffer; ++i) {
            if (fAvailableBuffers[i]->remaining() >= size) {
                buffer = fAvailableBuffers[i].get();
                fActiveBuffers.push_back(std::move(fAvailableBuffers[i]));
                fAvailableBuffers.erase(fAvailableBuffers.begin() + i);
            }
        }
    }

    if (!buffer) {
        size_t newSize = SkNextPow2(size);
        newSize = std::max(newSize, kMinStagingBufferSize);
        std::unique_ptr<GrStagingBuffer> newBuffer = this->allocateStagingBuffer(newSize);
        if (!newBuffer) {
            return {}; // invalid slice
        }
        buffer = newBuffer.get();
        fActiveBuffers.push_back(std::move(newBuffer));
    }

    SkASSERT(buffer);
    SkASSERT(buffer->remaining() >= size);
    return buffer->allocate(size);
}

void GrStagingBufferManager::release() {
    // TODO: If any backends ever end up have resources on the GrStagingBuffer implementations that
    // need to be involved with release, then we will need to loop through all the GrStagingBuffers
    // here and call release on each one.
    fActiveBuffers.clear();
    fAvailableBuffers.clear();
    this->onRelease();
}

void GrStagingBufferManager::abandon() {
    // TODO: If any backends ever end up have resources on the GrStagingBuffer implementations that
    // need to be involved with abandon, then we will need to loop through all the GrStagingBuffers
    // here and call abandon on each one.
    fActiveBuffers.clear();
    fAvailableBuffers.clear();
    this->onAbandon();
}
