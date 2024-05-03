/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/UploadTask.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAlign.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/DataUtils.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UploadBufferManager.h"

using namespace skia_private;

namespace skgpu::graphite {

UploadInstance::UploadInstance() = default;
UploadInstance::UploadInstance(UploadInstance&&) = default;
UploadInstance& UploadInstance::operator=(UploadInstance&&) = default;
UploadInstance::~UploadInstance() = default;

UploadInstance::UploadInstance(const Buffer* buffer,
                               size_t bytesPerPixel,
                               sk_sp<TextureProxy> textureProxy,
                               std::unique_ptr<ConditionalUploadContext> condContext)
        : fBuffer(buffer)
        , fBytesPerPixel(bytesPerPixel)
        , fTextureProxy(textureProxy)
        , fConditionalContext(std::move(condContext)) {}

// Returns total buffer size to allocate, and required offset alignment of that allocation.
// Updates 'levelOffsetsAndRowBytes' with offsets relative to start of the allocation, as well as
// the aligned destination rowBytes for each level.
std::pair<size_t, size_t> compute_combined_buffer_size(
        const Caps* caps,
        int mipLevelCount,
        size_t bytesPerBlock,
        const SkISize& baseDimensions,
        SkTextureCompressionType compressionType,
        TArray<std::pair<size_t, size_t>>* levelOffsetsAndRowBytes) {
    SkASSERT(levelOffsetsAndRowBytes && !levelOffsetsAndRowBytes->size());
    SkASSERT(mipLevelCount >= 1);

    SkISize compressedBlockDimensions = CompressedDimensionsInBlocks(compressionType,
                                                                     baseDimensions);

    size_t minTransferBufferAlignment =
            std::max(bytesPerBlock, caps->requiredTransferBufferAlignment());
    size_t alignedBytesPerRow =
            caps->getAlignedTextureDataRowBytes(compressedBlockDimensions.width() * bytesPerBlock);

    levelOffsetsAndRowBytes->push_back({0, alignedBytesPerRow});
    size_t combinedBufferSize = SkAlignTo(alignedBytesPerRow * baseDimensions.height(),
                                          minTransferBufferAlignment);
    SkISize levelDimensions = baseDimensions;

    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        levelDimensions = {std::max(1, levelDimensions.width() / 2),
                           std::max(1, levelDimensions.height() / 2)};
        compressedBlockDimensions = CompressedDimensionsInBlocks(compressionType, levelDimensions);
        alignedBytesPerRow = caps->getAlignedTextureDataRowBytes(
                compressedBlockDimensions.width() * bytesPerBlock);
        size_t alignedSize = SkAlignTo(alignedBytesPerRow * compressedBlockDimensions.height(),
                                       minTransferBufferAlignment);
        SkASSERT(combinedBufferSize % minTransferBufferAlignment == 0);

        levelOffsetsAndRowBytes->push_back({combinedBufferSize, alignedBytesPerRow});
        combinedBufferSize += alignedSize;
    }

    SkASSERT(levelOffsetsAndRowBytes->size() == mipLevelCount);
    SkASSERT(combinedBufferSize % minTransferBufferAlignment == 0);
    return {combinedBufferSize, minTransferBufferAlignment};
}

UploadInstance UploadInstance::Make(Recorder* recorder,
                                    sk_sp<TextureProxy> textureProxy,
                                    const SkColorInfo& srcColorInfo,
                                    const SkColorInfo& dstColorInfo,
                                    SkSpan<const MipLevel> levels,
                                    const SkIRect& dstRect,
                                    std::unique_ptr<ConditionalUploadContext> condContext) {
    const Caps* caps = recorder->priv().caps();
    SkASSERT(caps->isTexturable(textureProxy->textureInfo()));
    SkASSERT(caps->areColorTypeAndTextureInfoCompatible(dstColorInfo.colorType(),
                                                        textureProxy->textureInfo()));

    unsigned int mipLevelCount = levels.size();
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(mipLevelCount == 1 || dstRect == SkIRect::MakeSize(textureProxy->dimensions()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
#ifdef SK_DEBUG
    unsigned int numExpectedLevels = 1;
    if (textureProxy->textureInfo().mipmapped() == Mipmapped::kYes) {
        numExpectedLevels = SkMipmap::ComputeLevelCount(textureProxy->dimensions().width(),
                                                        textureProxy->dimensions().height()) + 1;
    }
    SkASSERT(mipLevelCount == 1 || mipLevelCount == numExpectedLevels);
#endif

    if (dstRect.isEmpty()) {
        return Invalid();
    }

    if (mipLevelCount == 1 && !levels[0].fPixels) {
        return Invalid();   // no data to upload
    }

    for (unsigned int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!levels[i].fPixels) {
            return Invalid();
        }
    }

    SkColorType supportedColorType;
    bool isRGB888Format;
    std::tie(supportedColorType, isRGB888Format) =
            caps->supportedWritePixelsColorType(dstColorInfo.colorType(),
                                                textureProxy->textureInfo(),
                                                srcColorInfo.colorType());
    if (supportedColorType == kUnknown_SkColorType) {
        return Invalid();
    }

    const size_t bpp = isRGB888Format ? 3 : SkColorTypeBytesPerPixel(supportedColorType);
    TArray<std::pair<size_t, size_t>> levelOffsetsAndRowBytes(mipLevelCount);

    auto [combinedBufferSize, minAlignment] = compute_combined_buffer_size(
            caps,
            mipLevelCount,
            bpp,
            dstRect.size(),
            SkTextureCompressionType::kNone,
            &levelOffsetsAndRowBytes);
    SkASSERT(combinedBufferSize);

    UploadBufferManager* bufferMgr = recorder->priv().uploadBufferManager();
    auto [writer, bufferInfo] = bufferMgr->getTextureUploadWriter(combinedBufferSize, minAlignment);
    if (!writer) {
        SKGPU_LOG_W("Failed to get write-mapped buffer for texture upload of size %zu",
                    combinedBufferSize);
        return Invalid();
    }

    UploadInstance upload{bufferInfo.fBuffer, bpp, std::move(textureProxy), std::move(condContext)};

    // Fill in copy data
    int32_t currentWidth = dstRect.width();
    int32_t currentHeight = dstRect.height();
    bool needsConversion = (srcColorInfo != dstColorInfo);
    for (unsigned int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        const size_t trimRowBytes = currentWidth * bpp;
        const size_t srcRowBytes = levels[currentMipLevel].fRowBytes;
        const auto [mipOffset, dstRowBytes] = levelOffsetsAndRowBytes[currentMipLevel];

        // copy data into the buffer, skipping any trailing bytes
        const char* src = (const char*)levels[currentMipLevel].fPixels;

        if (isRGB888Format) {
            SkASSERT(supportedColorType == kRGB_888x_SkColorType &&
                     dstColorInfo.colorType() == kRGB_888x_SkColorType);
            SkISize dims = {currentWidth, currentHeight};
            SkImageInfo srcImageInfo = SkImageInfo::Make(dims, srcColorInfo);
            SkImageInfo dstImageInfo = SkImageInfo::Make(dims, dstColorInfo);

            const void* rgbConvertSrc = src;
            size_t rgbSrcRowBytes = srcRowBytes;
            SkAutoPixmapStorage temp;
            if (needsConversion) {
                temp.alloc(dstImageInfo);
                SkAssertResult(SkConvertPixels(dstImageInfo,
                                               temp.writable_addr(),
                                               temp.rowBytes(),
                                               srcImageInfo,
                                               src,
                                               srcRowBytes));
                rgbConvertSrc = temp.addr();
                rgbSrcRowBytes = temp.rowBytes();
            }
            writer.writeRGBFromRGBx(mipOffset,
                                    rgbConvertSrc,
                                    rgbSrcRowBytes,
                                    dstRowBytes,
                                    currentWidth,
                                    currentHeight);
        } else if (needsConversion) {
            SkISize dims = {currentWidth, currentHeight};
            SkImageInfo srcImageInfo = SkImageInfo::Make(dims, srcColorInfo);
            SkImageInfo dstImageInfo = SkImageInfo::Make(dims, dstColorInfo);

            writer.convertAndWrite(
                    mipOffset, srcImageInfo, src, srcRowBytes, dstImageInfo, dstRowBytes);
        } else {
            writer.write(mipOffset, src, srcRowBytes, dstRowBytes, trimRowBytes, currentHeight);
        }

        // For mipped data, the dstRect is always the full texture so we don't need to worry about
        // modifying the TL coord as it will always be 0,0,for all levels.
        upload.fCopyData.push_back({
            /*fBufferOffset=*/bufferInfo.fOffset + mipOffset,
            /*fBufferRowBytes=*/dstRowBytes,
            /*fRect=*/SkIRect::MakeXYWH(dstRect.left(), dstRect.top(), currentWidth, currentHeight),
            /*fMipmapLevel=*/currentMipLevel
        });

        currentWidth = std::max(1, currentWidth / 2);
        currentHeight = std::max(1, currentHeight / 2);
    }

    ATRACE_ANDROID_FRAMEWORK("Upload %sTexture [%dx%d]",
                             mipLevelCount > 1 ? "MipMap " : "",
                             dstRect.width(), dstRect.height());

    return upload;
}

UploadInstance UploadInstance::MakeCompressed(Recorder* recorder,
                                              sk_sp<TextureProxy> textureProxy,
                                              const void* data,
                                              size_t dataSize) {
    if (!data) {
        return Invalid();   // no data to upload
    }

    const TextureInfo& texInfo = textureProxy->textureInfo();

    const Caps* caps = recorder->priv().caps();
    SkASSERT(caps->isTexturable(texInfo));

    SkTextureCompressionType compression = texInfo.compressionType();
    if (compression == SkTextureCompressionType::kNone) {
        return Invalid();
    }

    // Create a transfer buffer and fill with data.
    const SkISize dimensions = textureProxy->dimensions();
    skia_private::STArray<16, size_t> srcMipOffsets;
    SkDEBUGCODE(size_t computedSize =) SkCompressedDataSize(compression,
                                                            dimensions,
                                                            &srcMipOffsets,
                                                            texInfo.mipmapped() == Mipmapped::kYes);
    SkASSERT(computedSize == dataSize);

    unsigned int mipLevelCount = srcMipOffsets.size();
    size_t bytesPerBlock = SkCompressedBlockSize(compression);
    TArray<std::pair<size_t, size_t>> levelOffsetsAndRowBytes(mipLevelCount);
    auto [combinedBufferSize, minAlignment] = compute_combined_buffer_size(
            caps,
            mipLevelCount,
            bytesPerBlock,
            dimensions,
            compression,
            &levelOffsetsAndRowBytes);
    SkASSERT(combinedBufferSize);

    UploadBufferManager* bufferMgr = recorder->priv().uploadBufferManager();
    auto [writer, bufferInfo] = bufferMgr->getTextureUploadWriter(combinedBufferSize, minAlignment);

    std::vector<BufferTextureCopyData> copyData(mipLevelCount);

    if (!bufferInfo.fBuffer) {
        SKGPU_LOG_W("Failed to get write-mapped buffer for texture upload of size %zu",
                    combinedBufferSize);
        return Invalid();
    }

    UploadInstance upload{bufferInfo.fBuffer, bytesPerBlock, std::move(textureProxy)};

    // Fill in copy data
    int32_t currentWidth = dimensions.width();
    int32_t currentHeight = dimensions.height();
    for (unsigned int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        SkISize blockDimensions = CompressedDimensionsInBlocks(compression,
                                                               {currentWidth, currentHeight});
        int32_t blockHeight = blockDimensions.height();

        const size_t trimRowBytes = CompressedRowBytes(compression, currentWidth);
        const size_t srcRowBytes = trimRowBytes;
        const auto [dstMipOffset, dstRowBytes] = levelOffsetsAndRowBytes[currentMipLevel];

        // copy data into the buffer, skipping any trailing bytes
        const void* src = SkTAddOffset<const void>(data, srcMipOffsets[currentMipLevel]);

        writer.write(dstMipOffset, src, srcRowBytes, dstRowBytes, trimRowBytes, blockHeight);

        int32_t copyWidth = currentWidth;
        int32_t copyHeight = currentHeight;
        if (caps->fullCompressedUploadSizeMustAlignToBlockDims()) {
            SkISize oneBlockDims = CompressedDimensions(compression, {1, 1});
            copyWidth = SkAlignTo(copyWidth, oneBlockDims.fWidth);
            copyHeight = SkAlignTo(copyHeight, oneBlockDims.fHeight);
        }

        upload.fCopyData.push_back({
            /*fBufferOffset=*/bufferInfo.fOffset + dstMipOffset,
            /*fBufferRowBytes=*/dstRowBytes,
            /*fRect=*/SkIRect::MakeXYWH(0, 0, copyWidth, copyHeight),
            /*fMipLevel=*/currentMipLevel
        });

        currentWidth = std::max(1, currentWidth / 2);
        currentHeight = std::max(1, currentHeight / 2);
    }

    ATRACE_ANDROID_FRAMEWORK("Upload Compressed %sTexture [%dx%d]",
                             mipLevelCount > 1 ? "MipMap " : "",
                             dimensions.width(),
                             dimensions.height());

    return upload;
}

bool UploadInstance::prepareResources(ResourceProvider* resourceProvider) {
    // While most uploads are to already instantiated proxies (e.g. for client-created texture
    // images) it is possible that writePixels() was issued as the first operation on a scratch
    // Device, or that this is the first upload to the raster or text atlas proxies.
    // TODO: Determine how to instantatiate textues in this case; atlas proxies shouldn't really be
    // "scratch" because they aren't going to be reused for anything else in a Recording. At the
    // same time, it could still go through the ScratchResourceManager and just never return them,
    // which is no different from instantiating them directly with the ResourceProvider.
    if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fTextureProxy.get())) {
        SKGPU_LOG_E("Could not instantiate texture proxy for UploadTask!");
        return false;
    }
    return true;
}

Task::Status UploadInstance::addCommand(Context* context,
                                        CommandBuffer* commandBuffer,
                                        Task::ReplayTargetData replayData) const {
    using Status = Task::Status;
    SkASSERT(fTextureProxy && fTextureProxy->isInstantiated());

    if (fConditionalContext && !fConditionalContext->needsUpload(context)) {
        // Assume that if a conditional context says to dynamically not upload that another
        // time through the tasks should try to upload again.
        return Status::kSuccess;
    }

    if (fTextureProxy->texture() != replayData.fTarget) {
        // The CommandBuffer doesn't take ownership of the upload buffer here; it's owned by
        // UploadBufferManager, which will transfer ownership in transferToCommandBuffer.
        if (!commandBuffer->copyBufferToTexture(fBuffer,
                                                fTextureProxy->refTexture(),
                                                fCopyData.data(),
                                                fCopyData.size())) {
            return Status::kFail;
        }
    } else {
        // Here we assume that multiple copies in a single UploadInstance are always used for
        // mipmaps of a single image, and that we won't ever copy to a replay target with mipmaps.
        SkASSERT(fCopyData.size() == 1);
        const BufferTextureCopyData& copyData = fCopyData[0];
        SkIRect dstRect = copyData.fRect;
        dstRect.offset(replayData.fTranslation);
        SkIRect croppedDstRect = dstRect;
        if (!croppedDstRect.intersect(SkIRect::MakeSize(fTextureProxy->dimensions()))) {
            // The replay translation can change on each insert, so subsequent replays may
            // actually intersect the copy rect.
            return Status::kSuccess;
        }

        BufferTextureCopyData transformedCopyData = copyData;
        transformedCopyData.fBufferOffset +=
                (croppedDstRect.y() - dstRect.y()) * copyData.fBufferRowBytes +
                (croppedDstRect.x() - dstRect.x()) * fBytesPerPixel;
        transformedCopyData.fRect = croppedDstRect;

        if (!commandBuffer->copyBufferToTexture(fBuffer,
                                                fTextureProxy->refTexture(),
                                                &transformedCopyData, 1)) {
            return Status::kFail;
        }
    }

    // The conditional context will return false if the upload should not happen anymore. If there's
    // no context assume that the upload should always be executed on replay.
    if (!fConditionalContext || fConditionalContext->uploadSubmitted()) {
        return Status::kSuccess;
    } else {
        return Status::kDiscard;
    }
}

//---------------------------------------------------------------------------

bool UploadList::recordUpload(Recorder* recorder,
                              sk_sp<TextureProxy> textureProxy,
                              const SkColorInfo& srcColorInfo,
                              const SkColorInfo& dstColorInfo,
                              SkSpan<const MipLevel> levels,
                              const SkIRect& dstRect,
                              std::unique_ptr<ConditionalUploadContext> condContext) {
    UploadInstance instance = UploadInstance::Make(recorder, std::move(textureProxy),
                                                   srcColorInfo, dstColorInfo,
                                                   levels, dstRect, std::move(condContext));
    if (!instance.isValid()) {
        return false;
    }

    fInstances.emplace_back(std::move(instance));
    return true;
}

//---------------------------------------------------------------------------

sk_sp<UploadTask> UploadTask::Make(UploadList* uploadList) {
    SkASSERT(uploadList);
    if (!uploadList->size()) {
        return nullptr;
    }
    return sk_sp<UploadTask>(new UploadTask(std::move(uploadList->fInstances)));
}

sk_sp<UploadTask> UploadTask::Make(UploadInstance instance) {
    if (!instance.isValid()) {
        return nullptr;
    }
    return sk_sp<UploadTask>(new UploadTask(std::move(instance)));
}

UploadTask::UploadTask(skia_private::TArray<UploadInstance>&& instances)
        : fInstances(std::move(instances)) {}

UploadTask::UploadTask(UploadInstance instance) {
    fInstances.emplace_back(std::move(instance));
}

UploadTask::~UploadTask() {}

Task::Status UploadTask::prepareResources(ResourceProvider* resourceProvider,
                                          ScratchResourceManager*,
                                          const RuntimeEffectDictionary*) {
    for (int i = 0; i < fInstances.size(); ++i) {
        // No upload should be invalidated before prepareResources() is called.
        SkASSERT(fInstances[i].isValid());
        if (!fInstances[i].prepareResources(resourceProvider)) {
            return Status::kFail;
        }
    }

    return Status::kSuccess;
}

Task::Status UploadTask::addCommands(Context* context,
                                     CommandBuffer* commandBuffer,
                                     ReplayTargetData replayData) {
    int discardCount = 0;
    for (int i = 0; i < fInstances.size(); ++i) {
        if (!fInstances[i].isValid()) {
            discardCount++;
            continue;
        }
        Status status = fInstances[i].addCommand(context, commandBuffer, replayData);
        if (status == Status::kFail) {
            return Status::kFail;
        } else if (status == Status::kDiscard) {
            fInstances[i] = UploadInstance::Invalid();
            discardCount++;
        }
    }

    if (discardCount == fInstances.size()) {
        return Status::kDiscard;
    } else {
        return Status::kSuccess;
    }
}

} // namespace skgpu::graphite
