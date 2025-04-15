/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/CopyTask.h"

#include "include/private/base/SkAssert.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/Texture.h"  // IWYU pragma: keep
#include "src/gpu/graphite/TextureProxy.h"

#include <utility>

namespace skgpu::graphite {

sk_sp<CopyBufferToBufferTask> CopyBufferToBufferTask::Make(const Buffer* srcBuffer,
                                                           size_t srcOffset,
                                                           sk_sp<Buffer> dstBuffer,
                                                           size_t dstOffset,
                                                           size_t size) {
    SkASSERT(srcBuffer);
    SkASSERT(size <= srcBuffer->size() - srcOffset);
    SkASSERT(dstBuffer);
    SkASSERT(size <= dstBuffer->size() - dstOffset);
    return sk_sp<CopyBufferToBufferTask>(new CopyBufferToBufferTask(srcBuffer,
                                                                    srcOffset,
                                                                    std::move(dstBuffer),
                                                                    dstOffset,
                                                                    size));
}

CopyBufferToBufferTask::CopyBufferToBufferTask(const Buffer* srcBuffer,
                                               size_t srcOffset,
                                               sk_sp<Buffer> dstBuffer,
                                               size_t dstOffset,
                                               size_t size)
        : fSrcBuffer(srcBuffer)
        , fSrcOffset(srcOffset)
        , fDstBuffer(std::move(dstBuffer))
        , fDstOffset(dstOffset)
        , fSize(size) {}

CopyBufferToBufferTask::~CopyBufferToBufferTask() = default;

Task::Status CopyBufferToBufferTask::prepareResources(ResourceProvider*,
                                                      ScratchResourceManager*,
                                                      const RuntimeEffectDictionary*) {
    return Status::kSuccess;
}

Task::Status CopyBufferToBufferTask::addCommands(Context*,
                                                 CommandBuffer* commandBuffer,
                                                 ReplayTargetData) {
    if (commandBuffer->copyBufferToBuffer(fSrcBuffer, fSrcOffset, fDstBuffer, fDstOffset, fSize)) {
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

sk_sp<CopyTextureToBufferTask> CopyTextureToBufferTask::Make(sk_sp<TextureProxy> textureProxy,
                                                             SkIRect srcRect,
                                                             sk_sp<Buffer> buffer,
                                                             size_t bufferOffset,
                                                             size_t bufferRowBytes) {
    if (!textureProxy) {
        return nullptr;
    }
    return sk_sp<CopyTextureToBufferTask>(new CopyTextureToBufferTask(std::move(textureProxy),
                                                                      srcRect,
                                                                      std::move(buffer),
                                                                      bufferOffset,
                                                                      bufferRowBytes));
}

CopyTextureToBufferTask::CopyTextureToBufferTask(sk_sp<TextureProxy> textureProxy,
                                                 SkIRect srcRect,
                                                 sk_sp<Buffer> buffer,
                                                 size_t bufferOffset,
                                                 size_t bufferRowBytes)
        : fTextureProxy(std::move(textureProxy))
        , fSrcRect(srcRect)
        , fBuffer(std::move(buffer))
        , fBufferOffset(bufferOffset)
        , fBufferRowBytes(bufferRowBytes) {
}

CopyTextureToBufferTask::~CopyTextureToBufferTask() {}

Task::Status CopyTextureToBufferTask::prepareResources(ResourceProvider* resourceProvider,
                                                       ScratchResourceManager*,
                                                       const RuntimeEffectDictionary*) {
    // If the source texture hasn't been instantiated yet, it means there was no prior task that
    // could have initialized its contents so a readback to a buffer does not make sense.
    SkASSERT(fTextureProxy->isInstantiated() || fTextureProxy->isLazy());
    // TODO: The copy is also a consumer of the source, so it should participate in returning
    // scratch resources like RenderPassTask does. For now, though, all copy tasks side step reuse
    // entirely and they cannot participate until they've been moved into scoping tasks like
    // DrawTask first.
    return Status::kSuccess;
}

Task::Status CopyTextureToBufferTask::addCommands(Context*,
                                                  CommandBuffer* commandBuffer,
                                                  ReplayTargetData) {
    if (commandBuffer->copyTextureToBuffer(fTextureProxy->refTexture(),
                                           fSrcRect,
                                           std::move(fBuffer),
                                           fBufferOffset,
                                           fBufferRowBytes)) {
        // TODO(b/332681367): CopyTextureToBuffer is currently only used for readback operations,
        // which are a one-time event. Should this just default to returning kDiscard?
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

//--------------------------------------------------------------------------------------------------

sk_sp<CopyTextureToTextureTask> CopyTextureToTextureTask::Make(sk_sp<TextureProxy> srcProxy,
                                                               SkIRect srcRect,
                                                               sk_sp<TextureProxy> dstProxy,
                                                               SkIPoint dstPoint,
                                                               int dstLevel) {
    if (!srcProxy || !dstProxy) {
        return nullptr;
    }
    return sk_sp<CopyTextureToTextureTask>(new CopyTextureToTextureTask(std::move(srcProxy),
                                                                        srcRect,
                                                                        std::move(dstProxy),
                                                                        dstPoint,
                                                                        dstLevel));
}

CopyTextureToTextureTask::CopyTextureToTextureTask(sk_sp<TextureProxy> srcProxy,
                                                   SkIRect srcRect,
                                                   sk_sp<TextureProxy> dstProxy,
                                                   SkIPoint dstPoint,
                                                   int dstLevel)
        : fSrcProxy(std::move(srcProxy))
        , fSrcRect(srcRect)
        , fDstProxy(std::move(dstProxy))
        , fDstPoint(dstPoint)
        , fDstLevel(dstLevel) {}

CopyTextureToTextureTask::~CopyTextureToTextureTask() {}

Task::Status CopyTextureToTextureTask::prepareResources(ResourceProvider* resourceProvider,
                                                        ScratchResourceManager*,
                                                        const RuntimeEffectDictionary*) {
    // Do not instantiate the src proxy. If the source texture hasn't been instantiated yet, it
    // means there was no prior task that could have initialized its contents so propagating the
    // undefined contents to the dst does not make sense.
    // TODO(b/333729316): Assert that fSrcProxy is instantiated or lazy; right now it may not be
    // instantatiated if this is a dst readback copy for a scratch Device. In that case, a
    // RenderPassTask will immediately follow this copy task and instantiate the source proxy so
    // that addCommands() has a texture to operate on. That said, the texture's contents will be
    // undefined when the copy is executed ideally it just shouldn't happen.

    // TODO: The copy is also a consumer of the source, so it should participate in returning
    // scratch resources like RenderPassTask does. For now, though, all copy tasks side step reuse
    // entirely and they cannot participate until they've been moved into scoping tasks like
    // DrawTask first. In particular, for texture-to-texture copies, they should be scoped to not
    // invoke pending listeners for a subsequent RenderPassTask.

    // TODO: Use the scratch resource manager to instantiate fDstProxy, although the details of when
    // that texture can be returned need to be worked out. While brittle, all current use cases
    // of scratch texture-to-texture copies have the dst used immediately by the next task, so it
    // could just add a pending listener that returns the texture w/o any read counting.
    if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fDstProxy.get())) {
        SKGPU_LOG_E("Could not instantiate dst texture proxy for CopyTextureToTextureTask!");
        return Status::kFail;
    }
    return Status::kSuccess;
}

Task::Status CopyTextureToTextureTask::addCommands(Context*,
                                                   CommandBuffer* commandBuffer,
                                                   ReplayTargetData) {
    // prepareResources() doesn't instantiate the source assuming that a prior task will have do so
    // as part of initializing the texture contents.
    SkASSERT(fSrcProxy->isInstantiated());
    if (commandBuffer->copyTextureToTexture(fSrcProxy->refTexture(),
                                            fSrcRect,
                                            fDstProxy->refTexture(),
                                            fDstPoint,
                                            fDstLevel)) {
        // TODO(b/332681367): The calling context should be able to specify whether or not this copy
        // is a repeatable operation (e.g. dst readback copy for blending) or one time (e.g. client
        // asked for a copy of an image or surface).
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

} // namespace skgpu::graphite
