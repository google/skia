/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrStagingBufferManager.h"

#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrResourceProvider.h"

#include <algorithm>

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
        size_t minSize = fGpu->getContext()->priv().options().fMinimumStagingBufferSize;
        size_t bufferSize = std::max(size, minSize);
        sk_sp<GrGpuBuffer> newBuffer = resourceProvider->createBuffer(
                bufferSize,
                GrGpuBufferType::kXferCpuToGpu,
                kDynamic_GrAccessPattern,
                GrResourceProvider::ZeroInit::kNo);
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
