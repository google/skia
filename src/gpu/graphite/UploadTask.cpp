/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/UploadTask.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAlign.h"
#include "src/core/SkMipmap.h"
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

using namespace skia_private;

namespace skgpu::graphite {

UploadInstance::UploadInstance() = default;
UploadInstance::UploadInstance(UploadInstance&&) = default;
UploadInstance& UploadInstance::operator=(UploadInstance&&) = default;
UploadInstance::~UploadInstance() = default;

UploadInstance::UploadInstance(const Buffer* buffer,
                               size_t bytesPerPixel,
                               sk_sp<TextureProxy> textureProxy,
                               std::vector<BufferTextureCopyData> copyData,
                               std::unique_ptr<ConditionalUploadContext> condContext)
        : fBuffer(buffer)
        , fBytesPerPixel(bytesPerPixel)
        , fTextureProxy(textureProxy)
        , fCopyData(copyData)
        , fConditionalContext(std::move(condContext)) {}

// Returns total buffer size to allocate, and required offset alignment of that allocation.
// Updates 'levelOffsetsAndRowBytes' with offsets relative to start of the allocation, as well as
// the aligned destination rowBytes for each level.
std::pair<size_t, size_t> compute_combined_buffer_size(
        const Caps* caps,
        int mipLevelCount,
        size_t bytesPerPixel,
        const SkISize& baseDimensions,
        TArray<std::pair<size_t, size_t>>* levelOffsetsAndRowBytes) {
    SkASSERT(levelOffsetsAndRowBytes && !levelOffsetsAndRowBytes->size());
    SkASSERT(mipLevelCount >= 1);

    size_t minTransferBufferAlignment =
            std::max(bytesPerPixel, caps->requiredTransferBufferAlignment());
    size_t alignedBytesPerRow =
            caps->getAlignedTextureDataRowBytes(baseDimensions.width() * bytesPerPixel);

    levelOffsetsAndRowBytes->push_back({0, alignedBytesPerRow});
    size_t combinedBufferSize = SkAlignTo(alignedBytesPerRow * baseDimensions.height(),
                                          minTransferBufferAlignment);
    SkISize levelDimensions = baseDimensions;

    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        levelDimensions = {std::max(1, levelDimensions.width() / 2),
                           std::max(1, levelDimensions.height() / 2)};
        alignedBytesPerRow =
                caps->getAlignedTextureDataRowBytes(levelDimensions.width() * bytesPerPixel);
        size_t alignedSize = SkAlignTo(alignedBytesPerRow * levelDimensions.height(),
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
                                    const std::vector<MipLevel>& levels,
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
        return {};
    }

    if (mipLevelCount == 1 && !levels[0].fPixels) {
        return {};   // no data to upload
    }

    for (unsigned int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!levels[i].fPixels) {
            return {};
        }
    }

    const size_t bpp = dstColorInfo.bytesPerPixel();
    TArray<std::pair<size_t, size_t>> levelOffsetsAndRowBytes(mipLevelCount);

    auto [combinedBufferSize, minAlignment] = compute_combined_buffer_size(
            caps, mipLevelCount, bpp, dstRect.size(), &levelOffsetsAndRowBytes);
    SkASSERT(combinedBufferSize);

    UploadBufferManager* bufferMgr = recorder->priv().uploadBufferManager();
    auto [writer, bufferInfo] = bufferMgr->getTextureUploadWriter(combinedBufferSize, minAlignment);

    std::vector<BufferTextureCopyData> copyData(mipLevelCount);

    if (!bufferInfo.fBuffer) {
        return {};
    }
    size_t baseOffset = bufferInfo.fOffset;

    int32_t currentWidth = dstRect.width();
    int32_t currentHeight = dstRect.height();
    bool needsConversion = (srcColorInfo != dstColorInfo);
    for (unsigned int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        const size_t trimRowBytes = currentWidth * bpp;
        const size_t srcRowBytes = levels[currentMipLevel].fRowBytes;
        const auto [mipOffset, dstRowBytes] = levelOffsetsAndRowBytes[currentMipLevel];

        // copy data into the buffer, skipping any trailing bytes
        const char* src = (const char*)levels[currentMipLevel].fPixels;
        if (needsConversion) {
            SkISize dims = {currentWidth, currentHeight};
            SkImageInfo srcImageInfo = SkImageInfo::Make(dims, srcColorInfo);
            SkImageInfo dstImageInfo = SkImageInfo::Make(dims, dstColorInfo);

            writer.convertAndWrite(
                    mipOffset, srcImageInfo, src, srcRowBytes, dstImageInfo, dstRowBytes);
        } else {
            writer.write(mipOffset, src, srcRowBytes, dstRowBytes, trimRowBytes, currentHeight);
        }

        copyData[currentMipLevel].fBufferOffset = baseOffset + mipOffset;
        copyData[currentMipLevel].fBufferRowBytes = dstRowBytes;
        copyData[currentMipLevel].fRect = {
            dstRect.left(), dstRect.top(), // TODO: can we recompute this for mips?
            dstRect.left() + currentWidth, dstRect.top() + currentHeight
        };
        copyData[currentMipLevel].fMipLevel = currentMipLevel;

        currentWidth = std::max(1, currentWidth / 2);
        currentHeight = std::max(1, currentHeight / 2);
    }

    ATRACE_ANDROID_FRAMEWORK("Upload %sTexture [%ux%u]",
                             mipLevelCount > 1 ? "MipMap " : "",
                             dstRect.width(), dstRect.height());

    return {bufferInfo.fBuffer, bpp, std::move(textureProxy), std::move(copyData),
            std::move(condContext)};
}

bool UploadInstance::prepareResources(ResourceProvider* resourceProvider) {
    if (!fTextureProxy) {
        SKGPU_LOG_E("No texture proxy specified for UploadTask");
        return false;
    }
    if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fTextureProxy.get())) {
        SKGPU_LOG_E("Could not instantiate texture proxy for UploadTask!");
        return false;
    }
    return true;
}

void UploadInstance::addCommand(Context* context,
                                CommandBuffer* commandBuffer,
                                Task::ReplayTargetData replayData) const {
    SkASSERT(fTextureProxy && fTextureProxy->isInstantiated());

    if (fConditionalContext && !fConditionalContext->needsUpload(context)) {
        return;
    }

    if (fTextureProxy->texture() != replayData.fTarget) {
        // The CommandBuffer doesn't take ownership of the upload buffer here; it's owned by
        // UploadBufferManager, which will transfer ownership in transferToCommandBuffer.
        commandBuffer->copyBufferToTexture(
                fBuffer, fTextureProxy->refTexture(), fCopyData.data(), fCopyData.size());

    } else {
        // Here we assume that multiple copies in a single UploadInstance are always used for
        // mipmaps of a single image, and that we won't ever copy to a replay target with mipmaps.
        SkASSERT(fCopyData.size() == 1);
        const BufferTextureCopyData& copyData = fCopyData[0];
        SkIRect dstRect = copyData.fRect;
        dstRect.offset(replayData.fTranslation);
        SkIRect croppedDstRect = dstRect;
        if (!croppedDstRect.intersect(SkIRect::MakeSize(fTextureProxy->dimensions()))) {
            return;
        }

        BufferTextureCopyData transformedCopyData = copyData;
        transformedCopyData.fBufferOffset +=
                (croppedDstRect.y() - dstRect.y()) * copyData.fBufferRowBytes +
                (croppedDstRect.x() - dstRect.x()) * fBytesPerPixel;
        transformedCopyData.fRect = croppedDstRect;

        commandBuffer->copyBufferToTexture(
                fBuffer, fTextureProxy->refTexture(), &transformedCopyData, 1);
    }

    if (fConditionalContext) {
        fConditionalContext->uploadSubmitted();
    }
}

//---------------------------------------------------------------------------

bool UploadList::recordUpload(Recorder* recorder,
                              sk_sp<TextureProxy> textureProxy,
                              const SkColorInfo& srcColorInfo,
                              const SkColorInfo& dstColorInfo,
                              const std::vector<MipLevel>& levels,
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
    SkASSERT(uploadList && uploadList->fInstances.size() > 0);
    return sk_sp<UploadTask>(new UploadTask(std::move(uploadList->fInstances)));
}

sk_sp<UploadTask> UploadTask::Make(UploadInstance instance) {
    if (!instance.isValid()) {
        return nullptr;
    }
    return sk_sp<UploadTask>(new UploadTask(std::move(instance)));
}

UploadTask::UploadTask(std::vector<UploadInstance> instances) : fInstances(std::move(instances)) {}

UploadTask::UploadTask(UploadInstance instance) {
    fInstances.emplace_back(std::move(instance));
}

UploadTask::~UploadTask() {}

bool UploadTask::prepareResources(ResourceProvider* resourceProvider,
                                  const RuntimeEffectDictionary*) {
    for (unsigned int i = 0; i < fInstances.size(); ++i) {
        if (!fInstances[i].prepareResources(resourceProvider)) {
            return false;
        }
    }

    return true;
}

bool UploadTask::addCommands(Context* context,
                             CommandBuffer* commandBuffer,
                             ReplayTargetData replayData) {
    for (unsigned int i = 0; i < fInstances.size(); ++i) {
        fInstances[i].addCommand(context, commandBuffer, replayData);
    }

    return true;
}

} // namespace skgpu::graphite
