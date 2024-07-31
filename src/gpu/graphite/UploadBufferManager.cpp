/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/UploadBufferManager.h"

#include "include/gpu/graphite/Recording.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkTFitsIn.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

static constexpr size_t kReusedBufferSize = 64 << 10;  // 64 KB

UploadBufferManager::UploadBufferManager(ResourceProvider* resourceProvider,
                                         const Caps* caps)
        : fResourceProvider(resourceProvider)
        , fMinAlignment(SkTo<uint32_t>(caps->requiredTransferBufferAlignment())) {}

UploadBufferManager::~UploadBufferManager() {}

std::tuple<TextureUploadWriter, BindBufferInfo> UploadBufferManager::getTextureUploadWriter(
        size_t requiredBytes, size_t requiredAlignment) {
    auto[bufferMapPtr, bindInfo] = this->makeBindInfo(requiredBytes,
                                                      requiredAlignment,
                                                      "TextureUploadBuffer");
    if (!bufferMapPtr) {
        return {TextureUploadWriter(), BindBufferInfo()};
    }

    return {TextureUploadWriter(bufferMapPtr, requiredBytes), bindInfo};
}

std::tuple<void* /*mappedPtr*/, BindBufferInfo> UploadBufferManager::makeBindInfo(
        size_t requiredBytes, size_t requiredAlignment, std::string_view label) {
    if (!SkTFitsIn<uint32_t>(requiredBytes)) {
        return {nullptr, BindBufferInfo()};
    }

    uint32_t requiredAlignment32 = std::max(SkTo<uint32_t>(requiredAlignment), fMinAlignment);
    uint32_t requiredBytes32 = SkAlignTo(SkTo<uint32_t>(requiredBytes), requiredAlignment32);
    if (requiredBytes32 > kReusedBufferSize) {
        // Create a dedicated buffer for this request.
        sk_sp<Buffer> buffer = fResourceProvider->findOrCreateBuffer(requiredBytes32,
                                                                     BufferType::kXferCpuToGpu,
                                                                     AccessPattern::kHostVisible,
                                                                     std::move(label));
        void* bufferMapPtr = buffer ? buffer->map() : nullptr;
        if (!bufferMapPtr) {
            // Unlike [Draw|Static]BufferManager, the UploadManager does not track if any buffer
            // mapping has failed. This is because it's common for uploads to be scoped to a
            // specific image creation. In that case, the image can be returned as null to signal a
            // very isolated failure instead of taking down the entire Recording. For the other
            // managers, failures to map buffers creates unrecoverable scenarios.
            return {nullptr, BindBufferInfo()};
        }

        BindBufferInfo bindInfo;
        bindInfo.fBuffer = buffer.get();
        bindInfo.fOffset = 0;
        bindInfo.fSize = requiredBytes32;

        fUsedBuffers.push_back(std::move(buffer));
        return {bufferMapPtr, bindInfo};
    }

    // Try to reuse an already-allocated buffer.
    fReusedBufferOffset = SkAlignTo(fReusedBufferOffset, requiredAlignment32);
    if (fReusedBuffer && requiredBytes32 > fReusedBuffer->size() - fReusedBufferOffset) {
        fUsedBuffers.push_back(std::move(fReusedBuffer));
    }

    if (!fReusedBuffer) {
        fReusedBuffer = fResourceProvider->findOrCreateBuffer(kReusedBufferSize,
                                                              BufferType::kXferCpuToGpu,
                                                              AccessPattern::kHostVisible,
                                                              std::move(label));
        fReusedBufferOffset = 0;
        if (!fReusedBuffer || !fReusedBuffer->map()) {
            fReusedBuffer = nullptr;
            return {nullptr, BindBufferInfo()};
        }
    }

    BindBufferInfo bindInfo;
    bindInfo.fBuffer = fReusedBuffer.get();
    bindInfo.fOffset = fReusedBufferOffset;
    bindInfo.fSize = requiredBytes32;

    void* bufferMapPtr = fReusedBuffer->map();
    SkASSERT(bufferMapPtr); // Should have been validated when it was created
    bufferMapPtr = SkTAddOffset<void>(bufferMapPtr, fReusedBufferOffset);

    fReusedBufferOffset += requiredBytes32;

    return {bufferMapPtr, bindInfo};
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
