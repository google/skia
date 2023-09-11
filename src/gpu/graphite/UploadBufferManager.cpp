/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/UploadBufferManager.h"

#include "include/gpu/graphite/Recording.h"
#include "include/private/base/SkAlign.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

static constexpr size_t kReusedBufferSize = 64 << 10;  // 64 KB

UploadBufferManager::UploadBufferManager(ResourceProvider* resourceProvider,
                                         const Caps* caps)
        : fResourceProvider(resourceProvider)
        , fMinAlignment(caps->requiredTransferBufferAlignment()) {}

UploadBufferManager::~UploadBufferManager() {}

std::tuple<UploadWriter, BindBufferInfo> UploadBufferManager::getUploadWriter(
        size_t requiredBytes, size_t requiredAlignment) {
    if (!requiredBytes) {
        return {UploadWriter(), BindBufferInfo()};
    }

    requiredAlignment = std::max(requiredAlignment, fMinAlignment);
    requiredBytes = SkAlignTo(requiredBytes, requiredAlignment);
    if (requiredBytes > kReusedBufferSize) {
        // Create a dedicated buffer for this request.
        sk_sp<Buffer> buffer = fResourceProvider->findOrCreateBuffer(
                requiredBytes, BufferType::kXferCpuToGpu, AccessPattern::kHostVisible);

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
    }

    if (!fReusedBuffer) {
        fReusedBuffer = fResourceProvider->findOrCreateBuffer(
                kReusedBufferSize, BufferType::kXferCpuToGpu, AccessPattern::kHostVisible);
        fReusedBufferOffset = 0;
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

void UploadBufferManager::transferToRecording(Recording* recording) {
    for (sk_sp<Buffer>& buffer : fUsedBuffers) {
        buffer->unmap();
        recording->priv().addResourceRef(std::move(buffer));
    }
    fUsedBuffers.clear();

    if (fReusedBuffer) {
        fReusedBuffer->unmap();
        recording->priv().addResourceRef(std::move(fReusedBuffer));
    }
}

}  // namespace skgpu::graphite
