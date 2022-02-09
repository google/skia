/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UploadTask.h"

#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Log.h"
#include "experimental/graphite/src/RecorderPriv.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "src/core/SkConvertPixels.h"

namespace skgpu {

void UploadCommand::addCommand(ResourceProvider* resourceProvider,
                               CommandBuffer* commandBuffer) const {
    if (!fTextureProxy) {
        SKGPU_LOG_E("No texture proxy specified for UploadTask");
        return;
    }
    if (!fTextureProxy->instantiate(resourceProvider)) {
        SKGPU_LOG_E("Could not instantiate texture proxy for UploadTask!");
        return;
    }

    commandBuffer->copyBufferToTexture(std::move(fBuffer),
                                       fTextureProxy->refTexture(),
                                       fCopyData.data(),
                                       fCopyData.size());
}

//---------------------------------------------------------------------------

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

bool UploadList::appendUpload(Recorder* recorder,
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
        return false;
    }

    SkASSERT(caps->areColorTypeAndTextureInfoCompatible(dataColorType,
                                                        textureProxy->textureInfo()));

    if (mipLevelCount == 1 && !levels[0].fPixels) {
        return true;   // no data to upload
    }

    for (unsigned int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!levels[i].fPixels) {
            return false;
        }
    }

    size_t bpp = SkColorTypeBytesPerPixel(dataColorType);
    size_t minAlignment = caps->getTransferBufferAlignment(bpp);
    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    size_t combinedBufferSize = compute_combined_buffer_size(mipLevelCount, bpp, minAlignment,
                                                             dstRect.size(), &individualMipOffsets);
    SkASSERT(combinedBufferSize);

    // TODO: get staging buffer or {void* offset, sk_sp<Buffer> buffer} pair.
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    sk_sp<Buffer> buffer = resourceProvider->findOrCreateBuffer(combinedBufferSize,
                                                                BufferType::kXferCpuToGpu,
                                                                PrioritizeGpuReads::kNo);

    std::vector<BufferTextureCopyData> copyData(mipLevelCount);

    if (!buffer) {
        return false;
    }
    char* bufferData = (char*) buffer->map(); // TODO: get from staging buffer instead
    size_t baseOffset = 0;

    int currentWidth = dstRect.width();
    int currentHeight = dstRect.height();
    for (unsigned int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        const size_t trimRowBytes = currentWidth * bpp;
        const size_t rowBytes = levels[currentMipLevel].fRowBytes;

        // copy data into the buffer, skipping any trailing bytes
        char* dst = bufferData + individualMipOffsets[currentMipLevel];
        const char* src = (const char*)levels[currentMipLevel].fPixels;
        SkRectMemcpy(dst, trimRowBytes, src, rowBytes, trimRowBytes, currentHeight);

        copyData[currentMipLevel].fBufferOffset =
                baseOffset + individualMipOffsets[currentMipLevel];
        copyData[currentMipLevel].fBufferRowBytes = trimRowBytes;
        copyData[currentMipLevel].fRect = {
            dstRect.left(), dstRect.top(), // TODO: can we recompute this for mips?
            currentWidth, currentHeight
        };
        copyData[currentMipLevel].fMipLevel = currentMipLevel;

        currentWidth = std::max(1, currentWidth/2);
        currentHeight = std::max(1, currentHeight/2);
    }

    buffer->unmap();

    fCommands.push_back({std::move(buffer), std::move(textureProxy), std::move(copyData)});

    return true;
}

//---------------------------------------------------------------------------

sk_sp<UploadTask> UploadTask::Make(UploadList* uploadList) {
    return sk_sp<UploadTask>(new UploadTask(std::move(uploadList->fCommands)));
}

UploadTask::UploadTask(std::vector<UploadCommand> commands) : fCommands(std::move(commands)) {}

UploadTask::~UploadTask() {}

void UploadTask::addCommands(ResourceProvider* resourceProvider,
                             CommandBuffer* commandBuffer) {
    for (unsigned int i = 0; i < fCommands.size(); ++i) {
        fCommands[i].addCommand(resourceProvider, commandBuffer);
    }
}

} // namespace skgpu
