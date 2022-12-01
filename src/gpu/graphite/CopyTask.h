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
class CommandBuffer;
class Texture;
class TextureProxy;

class CopyBufferToBufferTask final : public Task {
public:
    static sk_sp<CopyBufferToBufferTask> Make(sk_sp<Buffer> srcBuffer,
                                              sk_sp<Buffer> dstBuffer);

    ~CopyBufferToBufferTask() override;

    bool prepareResources(ResourceProvider*, const SkRuntimeEffectDictionary*) override;

    bool addCommands(ResourceProvider*, CommandBuffer*) override;

private:
    CopyBufferToBufferTask(sk_sp<Buffer> srcBuffer,
                           sk_sp<Buffer> dstBuffer);

    sk_sp<Buffer> fSrcBuffer;
    sk_sp<Buffer> fDstBuffer;
};

class CopyTextureToBufferTask final : public Task {
public:
    static sk_sp<CopyTextureToBufferTask> Make(sk_sp<TextureProxy>,
                                               SkIRect srcRect,
                                               sk_sp<Buffer>,
                                               size_t bufferOffset,
                                               size_t bufferRowBytes);

    ~CopyTextureToBufferTask() override;

    bool prepareResources(ResourceProvider*, const SkRuntimeEffectDictionary*) override;

    bool addCommands(ResourceProvider*, CommandBuffer*) override;

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
                                                SkIPoint dstPoint);

    ~CopyTextureToTextureTask() override;

    bool prepareResources(ResourceProvider*, const SkRuntimeEffectDictionary*) override;

    bool addCommands(ResourceProvider*, CommandBuffer*) override;

private:
    CopyTextureToTextureTask(sk_sp<TextureProxy> srcProxy,
                             SkIRect srcRect,
                             sk_sp<TextureProxy> dstProxy,
                             SkIPoint dstPoint);

    sk_sp<TextureProxy> fSrcProxy;
    SkIRect fSrcRect;
    sk_sp<TextureProxy> fDstProxy;
    SkIPoint fDstPoint;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_CopyTask_DEFINED
