/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrStagingBufferManager.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrResourceProvider.h"

GrStagingBufferManager::Slice GrStagingBufferManager::allocateStagingBufferSlice(
        size_t size, size_t requiredAlignment) {
    StagingBuffer* buffer = nullptr;
    size_t offset = 0;
    for (size_t i = 0; i < fBuffers.size(); ++i) {
        size_t totalBufferSize = fBuffers[i].fBuffer->size();
        size_t currentOffset = fBuffers[i].fOffset;
        offset = ((currentOffset + requiredAlignment - 1)/requiredAlignment)*requiredAlignment;
        if (totalBufferSize - offset >= size) {
            buffer = &fBuffers[i];
            break;
        }
    }

    if (!buffer) {
        GrResourceProvider* resourceProvider = fGpu->getContext()->priv().resourceProvider();
        size_t bufferSize = std::max(size, kMinStagingBufferSize);
        sk_sp<GrGpuBuffer> newBuffer = resourceProvider->createBuffer(
            bufferSize, GrGpuBufferType::kXferCpuToGpu, kDynamic_GrAccessPattern, nullptr);
        if (!newBuffer) {
            return {}; // invalid slice
        }
        void* mapPtr = newBuffer->map();
        if (!mapPtr) {
            return {}; // invalid slice
        }
        fBuffers.emplace_back(std::move(newBuffer), mapPtr);
        buffer = &fBuffers.back();
        offset = 0;
    }

    SkASSERT(buffer);
    SkASSERT(buffer->remaining() >= size);

    buffer->fOffset = offset + size;
    char* offsetMapPtr = static_cast<char*>(buffer->fMapPtr) + offset;
    return {buffer->fBuffer.get(), offset, offsetMapPtr};
}

void GrStagingBufferManager::detachBuffers() {
    for (size_t i = 0; i < fBuffers.size(); ++i) {
        fBuffers[i].fBuffer->unmap();
        fGpu->takeOwnershipOfBuffer(std::move(fBuffers[i].fBuffer));
    }
    fBuffers.clear();
}
