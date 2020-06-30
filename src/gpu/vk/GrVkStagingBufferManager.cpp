/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkStagingBufferManager.h"

#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkTransferBuffer.h"

std::unique_ptr<GrVkStagingBuffer> GrVkStagingBuffer::Make(GrVkGpu* gpu, size_t size) {
    sk_sp<GrVkTransferBuffer> buffer =
            GrVkTransferBuffer::Make(gpu, size, GrVkBuffer::kCopyRead_Type);
    if (!buffer) {
        return nullptr;
    }

    void* mapPtr = buffer->map();
    if (!mapPtr) {
        return nullptr;
    }

    return std::unique_ptr<GrVkStagingBuffer>(new GrVkStagingBuffer(gpu->stagingManager(), size,
                                                                    std::move(buffer),
                                                                    mapPtr));
}

GrVkStagingBuffer::GrVkStagingBuffer(GrVkStagingBufferManager* manager, size_t size,
                                     sk_sp<GrVkTransferBuffer> buffer,
                                     void* mapPtr)
        : GrStagingBuffer(size, mapPtr)
        , fBuffer(std::move(buffer)) {}

void GrVkStagingBuffer::recycle() {
    std::unique_ptr<GrStagingBuffer> upThis(this);
    if (void* mapPtr = fBuffer->map()) {
        fManager->recycleStagingBuffer(std::move(upThis), mapPtr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GrVkStagingBufferManager::onDetachStagedBuffers() {
    for (size_t i = 0; i < fStagedBuffers.size(); ++i) {
        GrVkStagingBuffer* buffer = static_cast<GrVkStagingBuffer*>(fStagedBuffers[i].release());
        buffer->buffer()->unmap();
        fGpu->currentCommandBuffer()->addStagingBuffer(buffer);
    }
}

std::unique_ptr<GrStagingBuffer> GrVkStagingBufferManager::allocateStagingBuffer(size_t size) {
    return GrVkStagingBuffer::Make(fGpu, size);
}
