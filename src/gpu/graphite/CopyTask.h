/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_CopyTask_DEFINED
#define skgpu_graphite_CopyTask_DEFINED

#include "src/gpu/graphite/Task.h"

#include "include/core/SkRect.h"

namespace skgpu::graphite {

class Buffer;
class Texture;
class TextureProxy;

class CopyBufferToBufferTask final : public Task {
public:
    static sk_sp<CopyBufferToBufferTask> Make(sk_sp<Buffer> srcBuffer,
                                              sk_sp<Buffer> dstBuffer);

    static sk_sp<CopyBufferToBufferTask> Make(sk_sp<Buffer> srcBuffer, size_t srcOffset,
                                              sk_sp<Buffer> dstBuffer, size_t dstOffset,
                                              size_t size);

    ~CopyBufferToBufferTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    CopyBufferToBufferTask(sk_sp<Buffer> srcBuffer, size_t srcOffset,
                           sk_sp<Buffer> dstBuffer, size_t dstOffset,
                           size_t size);

    sk_sp<Buffer> fSrcBuffer;
    size_t        fSrcOffset;
    sk_sp<Buffer> fDstBuffer;
    size_t        fDstOffset;
    size_t        fSize;
};

class CopyTextureToBufferTask final : public Task {
public:
    static sk_sp<CopyTextureToBufferTask> Make(sk_sp<TextureProxy>,
                                               SkIRect srcRect,
                                               sk_sp<Buffer>,
                                               size_t bufferOffset,
                                               size_t bufferRowBytes);

    ~CopyTextureToBufferTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    CopyTextureToBufferTask(sk_sp<TextureProxy>,
                            SkIRect srcRect,
                            sk_sp<Buffer>,
                            size_t bufferOffset,
                            size_t bufferRowBytes);

    sk_sp<TextureProxy> fTextureProxy;
    SkIRect fSrcRect;
    sk_sp<Buffer> fBuffer;
    size_t fBufferOffset;
    size_t fBufferRowBytes;
};

class CopyTextureToTextureTask final : public Task {
public:
    static sk_sp<CopyTextureToTextureTask> Make(sk_sp<TextureProxy> srcProxy,
                                                SkIRect srcRect,
                                                sk_sp<TextureProxy> dstProxy,
                                                SkIPoint dstPoint,
                                                int dstLevel = 0);

    ~CopyTextureToTextureTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    CopyTextureToTextureTask(sk_sp<TextureProxy> srcProxy,
                             SkIRect srcRect,
                             sk_sp<TextureProxy> dstProxy,
                             SkIPoint dstPoint,
                             int dstLevel);

    sk_sp<TextureProxy> fSrcProxy;
    SkIRect fSrcRect;
    sk_sp<TextureProxy> fDstProxy;
    SkIPoint fDstPoint;
    int fDstLevel;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_CopyTask_DEFINED
