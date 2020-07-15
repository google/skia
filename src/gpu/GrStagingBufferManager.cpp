/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrStagingBufferManager.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
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
    }

    SkASSERT(buffer);
    SkASSERT(buffer->remaining() >= size);

    size_t sliceOffset = buffer->fOffset;
    buffer->fOffset += size;
    char* offsetMapPtr = static_cast<char*>(buffer->fMapPtr) + sliceOffset;
    return {buffer->fBuffer.get(), sliceOffset, offsetMapPtr};
}

void GrStagingBufferManager::detachBuffers() {
    for (size_t i = 0; i < fBuffers.size(); ++i) {
        fBuffers[i].fBuffer->unmap();
        fGpu->takeOwnershipOfStagingBuffer(std::move(fBuffers[i].fBuffer));
    }
    fBuffers.clear();
}
