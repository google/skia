/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrStagingBufferManager.h"

#include "src/gpu/GrResourceProvider.h"

GrStagingBufferManager::Slice GrStagingBufferManager::allocateStagingBufferSlice(size_t size) {
    StagingBuffer* buffer = nullptr;
    for (size_t i = 0; i < fBuffers.size(); ++i) {
        if (fBuffers[i].remaining() >= size) {
            buffer = &fBuffers[i];
            break;
        }
    }

    if (!buffer) {
        sk_sp<GrGpuBuffer> newBuffer = fResourceProvider->createBuffer(
            size, GrGpuBufferType::kXferCpuToGpu, kDynamic_GrAccessPattern, nullptr);
        if (!newBuffer) {
            return {}; // invalid slice
        }
        void* mapPtr = newBuffer->map();
        if (!mapPtr) {
            return {}; // invalid slice
        }
        fBuffers.emplace_back(std::move(newBuffer), mapPtr);
        buffer = &fBuffers.back();
    }
    SkASSERT(buffer);

    SkASSERT(buffer);
    SkASSERT(buffer->remaining() >= size);

    size_t sliceOffset = buffer->fOffset;
    buffer->fOffset += size;
    return {buffer->fBuffer.get(), sliceOffset, buffer->fMapPtr};
}

void GrStagingBufferManager::detachBuffers(const DetachBufferFunc& func) {
    for (size_t i = 0; i < fBuffers.size(); ++i) {
        func(std::move(fBuffers[i].fBuffer));
    }
    fBuffers.clear();
}
