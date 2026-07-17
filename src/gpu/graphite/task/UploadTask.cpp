/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/UploadTask.h"

#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/SkAlign.h"
#include "include/private/SkAssert.h"
#include "include/private/SkDebug.h"
#include "include/private/SkLog.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkSafeMath.h"
#include "src/core/SkTraceEvent.h"  // IWYU pragma: keep
#include "src/gpu/DataUtils.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Texture.h"  // IWYU pragma: keep
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/UploadBufferManager.h"

#include <algorithm>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

using namespace skia_private;

namespace skgpu::graphite {

// Returns total buffer size to allocate, and required offset alignment of that allocation.
// Updates 'levelOffsetsAndRowBytes' with offsets relative to start of the allocation, as well as
// the aligned destination rowBytes for each level.
std::pair<size_t, size_t> compute_combined_buffer_size(
        const Caps* caps,
        TextureFormat format,
        int mipLevelCount,
        const SkISize& baseDimensions,
        TArray<std::pair<size_t, size_t>>* levelOffsetsAndRowBytes) {
    SkASSERT(levelOffsetsAndRowBytes && levelOffsetsAndRowBytes->empty());
    SkASSERT(mipLevelCount >= 1);

    const size_t bytesPerBlock = TextureFormatBytesPerBlock(format);
    const SkTextureCompressionType compressionType = TextureFormatCompressionType(format);

    SkISize compressedBlockDimensions = CompressedDimensionsInBlocks(compressionType,
                                                                     baseDimensions);

    size_t minTransferBufferAlignment =
            std::max(bytesPerBlock, caps->requiredTransferBufferAlignment());

    SkSafeMath safe;
    size_t rowBytes = safe.mul(compressedBlockDimensions.width(), bytesPerBlock);
    size_t alignedBytesPerRow = caps->getAlignedTextureDataRowBytes(rowBytes, bytesPerBlock);

    levelOffsetsAndRowBytes->push_back({0, alignedBytesPerRow});
    size_t combinedBufferSize =
            safe.alignUpNonPow2(safe.mul(alignedBytesPerRow, baseDimensions.height()),
                                minTransferBufferAlignment);
    SkISize levelDimensions = baseDimensions;

    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        levelDimensions = {std::max(1, levelDimensions.width() / 2),
                           std::max(1, levelDimensions.height() / 2)};
        compressedBlockDimensions = CompressedDimensionsInBlocks(compressionType, levelDimensions);
        rowBytes = safe.mul(compressedBlockDimensions.width(), bytesPerBlock);
        alignedBytesPerRow = caps->getAlignedTextureDataRowBytes(rowBytes, bytesPerBlock);
        size_t size = safe.mul(alignedBytesPerRow, compressedBlockDimensions.height());
        size_t alignedSize = safe.alignUpNonPow2(size, minTransferBufferAlignment);

        levelOffsetsAndRowBytes->push_back({combinedBufferSize, alignedBytesPerRow});
        combinedBufferSize = safe.add(combinedBufferSize, alignedSize);
    }

    SkASSERT(levelOffsetsAndRowBytes->size() == mipLevelCount);
    if (!safe.ok()) {
        levelOffsetsAndRowBytes->clear();
        return {0, minTransferBufferAlignment};
    }
    SkASSERT(combinedBufferSize % minTransferBufferAlignment == 0);
    return {combinedBufferSize, minTransferBufferAlignment};
}

UploadSource::UploadSource(TextureProxyView view) : fView(std::move(view)) {}
UploadSource::UploadSource(UploadSource&&) = default;
UploadSource& UploadSource::operator=(UploadSource&&) = default;
UploadSource::~UploadSource() = default;

UploadSource UploadSource::Make(const Caps* caps,
                                const TextureProxyView& dstView,
                                const SkColorInfo& srcColorInfo,
                                const SkColorInfo& dstColorInfo,
                                SkSpan<const MipLevel> levels,
                                const SkIRect& dstRect) {
    // No data to upload
    if (dstRect.isEmpty()) {
        return Invalid();
    }
    // Ensure data would fit into the texture
    if (!dstView.proxy()->isFullyLazy() &&
        !SkIRect::MakeSize(dstView.dimensions()).contains(dstRect)) {
        return Invalid();
    }

    unsigned int mipLevelCount = levels.size();
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    if (mipLevelCount != 1 && dstRect != SkIRect::MakeSize(dstView.dimensions())) {
        return Invalid();
    }

    // We assume that if the texture has mips, we either upload to all the levels or just the first.
    unsigned int numExpectedLevels = dstView.mipmapped() == Mipmapped::kYes ?
            SkMipmap::ComputeLevelCount(dstView.dimensions()) + 1 : 1;
    if (numExpectedLevels != mipLevelCount) {
        return Invalid();
    }

    SkColorSpaceXformSteps csSteps{srcColorInfo.colorSpace(), srcColorInfo.alphaType(),
                                   dstColorInfo.colorSpace(), dstColorInfo.alphaType()};
    auto xferFn = TextureFormatXferFn::MakeCpuToGpu(srcColorInfo.colorType(),
                                                    csSteps,
                                                    dstView.proxy()->format(),
                                                    dstView.swizzle());
    if (!xferFn) {
        return Invalid();
    }

    UploadSource source{std::move(dstView)};
    for (unsigned int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!levels[i].fPixels) {
            return Invalid();
        }
        source.fLevels.push_back(levels[i]);
    }

    source.fDstRect = dstRect;
    source.fXferFn = xferFn;
    return source;
}

UploadSource UploadSource::MakeCompressed(const Caps* caps,
                                          sk_sp<TextureProxy> textureProxy,
                                          const void* data,
                                          size_t dataSize) {
    if (!data) {
        return Invalid();  // no data to upload
    }
    SkTextureCompressionType compression = TextureFormatCompressionType(textureProxy->format());
    if (compression == SkTextureCompressionType::kNone) {
        return Invalid();
    }
    // Create a transfer buffer and fill with data.
    const SkISize dimensions = textureProxy->dimensions();
    STArray<16, size_t> srcMipOffsets;
    size_t computedSize = SkCompressedDataSize(
            compression, dimensions, &srcMipOffsets, textureProxy->mipmapped() == Mipmapped::kYes);
    if (computedSize != dataSize) {
        return Invalid();
    }

    auto xferFn = TextureFormatXferFn::MakeIdentity(textureProxy->format());
    SkASSERT(xferFn.has_value());

    const unsigned int mipLevelCount = srcMipOffsets.size();

    UploadSource source{TextureProxyView(textureProxy)};
    source.fLevels.resize(mipLevelCount);
    int currentWidth = textureProxy->dimensions().width();
    for (unsigned int i = 0; i < mipLevelCount; ++i) {
        source.fLevels[i].fPixels = SkTAddOffset<const void>(data, srcMipOffsets[i]);
        // Assume the source data is tightly packed.
        source.fLevels[i].fRowBytes = CompressedRowBytes(compression, currentWidth);
        currentWidth = std::max(1, currentWidth / 2);
    }

    source.fDstRect = SkIRect::MakeSize(dimensions);
    source.fXferFn = xferFn;
    return source;
}

bool UploadSource::attemptUploadOnhost() const {
    SkASSERT(this->isValid());

    // Don't upload on the host if we need to perform conversions that could be done directly into
    // a mapped GPU buffer.
    if (!fXferFn->isIdentity() ||
        !fView.proxy()->isInstantiated() ||
        !fView.proxy()->texture()->canUploadOnHost()) {
        return false;
    }

    // Don't upload on the host if the UploadSource doesn't have a unique hold on the TextureProxy
    // (otherwise some other thread could trigger GPU work while this thread was modifying the
    // underlying Texture resource).
    if (!fView.proxy()->unique()) {
        return false;
    }
    return fView.proxy()->texture()->uploadDataOnHost(*this);
}

UploadInstance::UploadInstance() = default;
UploadInstance::UploadInstance(UploadInstance&&) = default;
UploadInstance& UploadInstance::operator=(UploadInstance&&) = default;
UploadInstance::~UploadInstance() = default;

UploadInstance::UploadInstance(const Buffer* buffer,
                               sk_sp<TextureProxy> textureProxy,
                               std::unique_ptr<ConditionalUploadContext> condContext)
        : fBuffer(buffer)
        , fTextureProxy(textureProxy)
        , fConditionalContext(std::move(condContext)) {}

UploadInstance UploadInstance::Make(Recorder* recorder,
                                    const UploadSource& source,
                                    std::unique_ptr<ConditionalUploadContext> condContext) {
    sk_sp<TextureProxy> textureProxy = source.view().refProxy();
    const SkIRect dstRect = source.dstRect();
    const SkTextureCompressionType compression =
            TextureFormatCompressionType(textureProxy->format());

    const Caps* caps = recorder->priv().caps();
    SkSpan<const MipLevel> levels = source.levels();
    uint32_t mipLevelCount = static_cast<uint32_t>(levels.size());

    STArray<16, std::pair<size_t, size_t>> levelOffsetsAndRowBytes(mipLevelCount);
    auto [combinedBufferSize, minAlignment] =
            compute_combined_buffer_size(caps,
                                         textureProxy->format(),
                                         mipLevelCount,
                                         dstRect.size(),
                                         &levelOffsetsAndRowBytes);
    if (combinedBufferSize == 0) {
        return Invalid();
    }

    UploadBufferManager* bufferMgr = recorder->priv().uploadBufferManager();
    auto [writer, bufferInfo] = bufferMgr->getTextureUploadWriter(combinedBufferSize, minAlignment);

    if (!bufferInfo.fBuffer) {
        SKIA_LOG_W("Failed to get write-mapped buffer for texture upload of size %zu",
                    combinedBufferSize);
        return Invalid();
    }

    ATRACE_ANDROID_FRAMEWORK("Upload %s %sTexture [%dx%d]",
                             TextureFormatName(textureProxy->format()),
                             mipLevelCount > 1 ? "MipMap " : "",
                             dstRect.width(),
                             dstRect.height());

    UploadInstance upload{bufferInfo.fBuffer, std::move(textureProxy), std::move(condContext)};

    // Fill in copy data
    int32_t currentWidth = dstRect.width();
    int32_t currentHeight = dstRect.height();
    for (uint32_t currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        // NOTE: When not compressed, this function automatically returns currentWidth and height,
        // e.g. uncompressed blocks are the same as texels.
        SkISize blockDimensions =
                CompressedDimensionsInBlocks(compression, {currentWidth, currentHeight});

        const size_t srcRowBytes = levels[currentMipLevel].fRowBytes;
        const auto [dstMipOffset, dstRowBytes] = levelOffsetsAndRowBytes[currentMipLevel];

        // copy data into the buffer, skipping any trailing bytes
        const void* src = levels[currentMipLevel].fPixels;
        writer.convert(dstMipOffset, blockDimensions.width(), blockDimensions.height(),
                       src, srcRowBytes, source.formatXferFn(), dstRowBytes);

        int32_t copyWidth = currentWidth;
        int32_t copyHeight = currentHeight;
        if (compression != SkTextureCompressionType::kNone &&
            caps->fullCompressedUploadSizeMustAlignToBlockDims()) {
            SkISize oneBlockDims = CompressedDimensions(compression, {1, 1});
            copyWidth = SkAlignTo(copyWidth, oneBlockDims.fWidth);
            copyHeight = SkAlignTo(copyHeight, oneBlockDims.fHeight);
        }

        // For compressed and mipped data, the dstRect is always the full texture so we don't need
        // to worry about modifying the TL coord as it will always be 0,0,for all levels.
        SkASSERT((dstRect.left() == 0 && dstRect.top() == 0) ||
                 (mipLevelCount == 1 && compression == SkTextureCompressionType::kNone));
        upload.fCopyData.push_back({
            /*fBufferOffset=*/bufferInfo.fOffset + dstMipOffset,
            /*fBufferRowBytes=*/dstRowBytes,
            /*fRect=*/SkIRect::MakeXYWH(dstRect.left(), dstRect.top(), copyWidth, copyHeight),
            /*fMipLevel=*/currentMipLevel
        });

        currentWidth = std::max(1, currentWidth / 2);
        currentHeight = std::max(1, currentHeight / 2);
    }

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
        SKIA_LOG_E("Could not instantiate texture proxy for UploadTask!");
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
        // mipmaps of a single image, and that we won't ever upload to a replay target's mipmaps
        // directly.
        SkASSERT(fCopyData.size() == 1);
        const BufferTextureCopyData& copyData = fCopyData[0];
        SkIRect dstRect = copyData.fRect;
        dstRect.offset(replayData.fTranslation);
        SkIRect croppedDstRect = dstRect;

        if (!replayData.fClip.isEmpty()) {
            SkIRect dstClip = replayData.fClip;
            dstClip.offset(replayData.fTranslation);
            if (!croppedDstRect.intersect(dstClip)) {
                // The replay clip can change on each insert, so subsequent replays may actually
                // intersect the copy rect.
                return Status::kSuccess;
            }
        }

        if (!croppedDstRect.intersect(SkIRect::MakeSize(fTextureProxy->dimensions()))) {
            // The replay translation can change on each insert, so subsequent replays may
            // actually intersect the copy rect.
            return Status::kSuccess;
        }

        const int bpp = TextureFormatBytesPerBlock(fTextureProxy->format());
        BufferTextureCopyData transformedCopyData = copyData;
        transformedCopyData.fBufferOffset +=
                (croppedDstRect.y() - dstRect.y()) * copyData.fBufferRowBytes +
                (croppedDstRect.x() - dstRect.x()) * bpp;
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
                              const UploadSource& source,
                              std::unique_ptr<ConditionalUploadContext> condContext) {
    UploadInstance instance = UploadInstance::Make(recorder,
                                                   source,
                                                   std::move(condContext));
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
                                          sk_sp<const RuntimeEffectDictionary>) {
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
