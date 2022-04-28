/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/UploadTask.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UploadBufferManager.h"

namespace skgpu::graphite {

UploadInstance::UploadInstance(const Buffer* buffer,
                               sk_sp<TextureProxy> textureProxy,
                               std::vector<BufferTextureCopyData> copyData)
        : fBuffer(buffer), fTextureProxy(textureProxy), fCopyData(copyData) {}

size_t compute_combined_buffer_size(int mipLevelCount,
                                    size_t bytesPerPixel,
                                    size_t minTransferBufferAlignment,
                                    const SkISize& baseDimensions,
                                    SkTArray<size_t>* individualMipOffsets) {
    SkASSERT(individualMipOffsets && !individualMipOffsets->count());
    SkASSERT(mipLevelCount >= 1);

    individualMipOffsets->push_back(0);

    size_t combinedBufferSize = baseDimensions.width() * bytesPerPixel * baseDimensions.height();
    SkISize levelDimensions = baseDimensions;

    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        levelDimensions = {std::max(1, levelDimensions.width() /2),
                           std::max(1, levelDimensions.height()/2)};

        size_t trimmedSize = levelDimensions.area() * bytesPerPixel;
        combinedBufferSize = SkAlignTo(combinedBufferSize, minTransferBufferAlignment);
        SkASSERT((0 == combinedBufferSize % 4) && (0 == combinedBufferSize % bytesPerPixel));

        individualMipOffsets->push_back(combinedBufferSize);
        combinedBufferSize += trimmedSize;
    }

    SkASSERT(individualMipOffsets->count() == mipLevelCount);
    return combinedBufferSize;
}

UploadInstance UploadInstance::Make(Recorder* recorder,
                                    sk_sp<TextureProxy> textureProxy,
                                    SkColorType dataColorType,
                                    const std::vector<MipLevel>& levels,
                                    const SkIRect& dstRect) {
    const Caps* caps = recorder->priv().caps();
    SkASSERT(caps->isTexturable(textureProxy->textureInfo()));

    unsigned int mipLevelCount = levels.size();
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(mipLevelCount == 1 || dstRect == SkIRect::MakeSize(textureProxy->dimensions()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(mipLevelCount == 1 || mipLevelCount == textureProxy->textureInfo().numMipLevels());

    if (dstRect.isEmpty()) {
        return {};
    }

    SkASSERT(caps->areColorTypeAndTextureInfoCompatible(dataColorType,
                                                        textureProxy->textureInfo()));

    if (mipLevelCount == 1 && !levels[0].fPixels) {
        return {};   // no data to upload
    }

    for (unsigned int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!levels[i].fPixels) {
            return {};
        }
    }

    size_t bpp = SkColorTypeBytesPerPixel(dataColorType);
    size_t minAlignment = caps->getTransferBufferAlignment(bpp);
    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    size_t combinedBufferSize = compute_combined_buffer_size(mipLevelCount, bpp, minAlignment,
                                                             dstRect.size(), &individualMipOffsets);
    SkASSERT(combinedBufferSize);

    UploadBufferManager* bufferMgr = recorder->priv().uploadBufferManager();
    auto [writer, bufferInfo] = bufferMgr->getUploadWriter(combinedBufferSize, minAlignment);

    std::vector<BufferTextureCopyData> copyData(mipLevelCount);

    if (!bufferInfo.fBuffer) {
        return {};
    }
    size_t baseOffset = bufferInfo.fOffset;

    int currentWidth = dstRect.width();
    int currentHeight = dstRect.height();
    for (unsigned int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        const size_t trimRowBytes = currentWidth * bpp;
        const size_t rowBytes = levels[currentMipLevel].fRowBytes;
        const size_t mipOffset = individualMipOffsets[currentMipLevel];

        // copy data into the buffer, skipping any trailing bytes
        const char* src = (const char*)levels[currentMipLevel].fPixels;
        writer.write(mipOffset, src, rowBytes, trimRowBytes, currentHeight);

        copyData[currentMipLevel].fBufferOffset = baseOffset + mipOffset;
        copyData[currentMipLevel].fBufferRowBytes = trimRowBytes;
        copyData[currentMipLevel].fRect = {
            dstRect.left(), dstRect.top(), // TODO: can we recompute this for mips?
            dstRect.left() + currentWidth, dstRect.top() + currentHeight
        };
        copyData[currentMipLevel].fMipLevel = currentMipLevel;

        currentWidth = std::max(1, currentWidth/2);
        currentHeight = std::max(1, currentHeight/2);
    }

    ATRACE_ANDROID_FRAMEWORK("Upload %sTexture [%ux%u]",
                             mipLevelCount > 1 ? "MipMap " : "",
                             dstRect.width(), dstRect.height());

    return {bufferInfo.fBuffer, std::move(textureProxy), std::move(copyData)};
}

void UploadInstance::addCommand(ResourceProvider* resourceProvider,
                                CommandBuffer* commandBuffer) const {
    if (!fTextureProxy) {
        SKGPU_LOG_E("No texture proxy specified for UploadTask");
        return;
    }
    if (!fTextureProxy->instantiate(resourceProvider)) {
        SKGPU_LOG_E("Could not instantiate texture proxy for UploadTask!");
        return;
    }

    // The CommandBuffer doesn't take ownership of the upload buffer here; it's owned by
    // UploadBufferManager, which will transfer ownership in transferToCommandBuffer.
    commandBuffer->copyBufferToTexture(
            fBuffer, fTextureProxy->refTexture(), fCopyData.data(), fCopyData.size());
}

//---------------------------------------------------------------------------

bool UploadList::recordUpload(Recorder* recorder,
                              sk_sp<TextureProxy> textureProxy,
                              SkColorType dataColorType,
                              const std::vector<MipLevel>& levels,
                              const SkIRect& dstRect) {
    UploadInstance instance = UploadInstance::Make(recorder, std::move(textureProxy), dataColorType,
                                                   levels, dstRect);
    if (!instance.isValid()) {
        return false;
    }

    fInstances.push_back(instance);
    return true;
}

//---------------------------------------------------------------------------

sk_sp<UploadTask> UploadTask::Make(UploadList* uploadList) {
    SkASSERT(uploadList && uploadList->fInstances.size() > 0);
    return sk_sp<UploadTask>(new UploadTask(std::move(uploadList->fInstances)));
}

sk_sp<UploadTask> UploadTask::Make(const UploadInstance& instance) {
    if (!instance.isValid()) {
        return nullptr;
    }
    return sk_sp<UploadTask>(new UploadTask(instance));
}

UploadTask::UploadTask(std::vector<UploadInstance> instances) : fInstances(std::move(instances)) {}

UploadTask::UploadTask(const UploadInstance& instance) {
    fInstances.push_back(instance);
}

UploadTask::~UploadTask() {}

bool UploadTask::addCommands(ResourceProvider* resourceProvider,
                             CommandBuffer* commandBuffer) {
    for (unsigned int i = 0; i < fInstances.size(); ++i) {
        fInstances[i].addCommand(resourceProvider, commandBuffer);
    }

    return true;
}

} // namespace skgpu::graphite
