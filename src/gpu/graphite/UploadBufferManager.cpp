/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/UploadBufferManager.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

static constexpr size_t kReusedBufferSize = 64 << 10;  // 64 KB

UploadBufferManager::UploadBufferManager(ResourceProvider* resourceProvider)
        : fResourceProvider(resourceProvider) {}

UploadBufferManager::~UploadBufferManager() {}

std::tuple<UploadWriter, BindBufferInfo> UploadBufferManager::getUploadWriter(
        size_t requiredBytes, size_t requiredAlignment) {
    if (!requiredBytes) {
        return {UploadWriter(), BindBufferInfo()};
    }

    if (requiredBytes > kReusedBufferSize) {
        // Create a dedicated buffer for this request.
        sk_sp<Buffer> buffer = fResourceProvider->findOrCreateBuffer(
                requiredBytes, BufferType::kXferCpuToGpu, PrioritizeGpuReads::kNo);

        BindBufferInfo bindInfo;
        bindInfo.fBuffer = buffer.get();
        bindInfo.fOffset = 0;

        void* bufferMapPtr = buffer->map();
        fUsedBuffers.push_back(std::move(buffer));
        return {UploadWriter(bufferMapPtr, requiredBytes), bindInfo};
    }

    // Try to reuse an already-allocated buffer.
    fReusedBufferOffset = SkAlignTo(fReusedBufferOffset, requiredAlignment);
    if (fReusedBuffer && requiredBytes > fReusedBuffer->size() - fReusedBufferOffset) {
        fUsedBuffers.push_back(std::move(fReusedBuffer));
        fReusedBufferOffset = 0;
    }

    if (!fReusedBuffer) {
        fReusedBuffer = fResourceProvider->findOrCreateBuffer(
                kReusedBufferSize, BufferType::kXferCpuToGpu, PrioritizeGpuReads::kNo);
        if (!fReusedBuffer) {
            return {UploadWriter(), BindBufferInfo()};
        }
    }

    BindBufferInfo bindInfo;
    bindInfo.fBuffer = fReusedBuffer.get();
    bindInfo.fOffset = fReusedBufferOffset;

    void* bufferMapPtr = fReusedBuffer->map();
    bufferMapPtr = SkTAddOffset<void>(bufferMapPtr, fReusedBufferOffset);

    fReusedBufferOffset += requiredBytes;
    return {UploadWriter(bufferMapPtr, requiredBytes), bindInfo};
}

void UploadBufferManager::transferToCommandBuffer(CommandBuffer* commandBuffer) {
    for (sk_sp<Buffer>& buffer : fUsedBuffers) {
        buffer->unmap();
        commandBuffer->trackResource(std::move(buffer));
    }
    fUsedBuffers.clear();

    if (fReusedBuffer) {
        fReusedBuffer->unmap();
        commandBuffer->trackResource(std::move(fReusedBuffer));
    }
}

}  // namespace skgpu::graphite
