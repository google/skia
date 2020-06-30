/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnStagingBufferManager.h"

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnStagingBuffer.h"

void GrDawnStagingBufferManager::onDetachStagedBuffers() {
    for (size_t i = 0; i < fStagedBuffers.size(); ++i) {
        fBusyBuffers.push_back(std::move(fStagedBuffers[i]));
        // We need to unmap all the buffers.
        static_cast<GrDawnStagingBuffer*>(fBusyBuffers.back().get())->unmap();
    }
}

std::unique_ptr<GrStagingBuffer> GrDawnStagingBufferManager::allocateStagingBuffer(size_t size) {
    return fGpu->createStagingBuffer(size);
}

void GrDawnStagingBufferManager::mapStagingBuffers() {
    // Map all active buffers, so we get a callback when they're done.
    for (std::unique_ptr<GrStagingBuffer>& buffer : fBusyBuffers) {
        static_cast<GrDawnStagingBuffer*>(buffer.get())->mapAsync();
    }
}

std::unique_ptr<GrStagingBuffer> GrDawnStagingBufferManager::removeFromBusy(
        const GrStagingBuffer* buffer) {
    for (auto it = fBusyBuffers.begin(); it != fBusyBuffers.end(); ++it) {
        if (it->get() == buffer) {
            std::unique_ptr<GrStagingBuffer> upBuffer = std::move(*it);
            // Since we are returning after the erase there is no need to make sure the interator
            // is correctly updated to the next value.
            fBusyBuffers.erase(it);
            return upBuffer;
        }
    }
    // We should always find the buffer
    SkASSERT(false);
    return nullptr;
}
