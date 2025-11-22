/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_CopyTask_DEFINED
#define skgpu_graphite_task_CopyTask_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/task/Task.h"

#include <cstddef>
#include <functional>

namespace skgpu::graphite {

class Buffer;
class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;
class TextureProxy;

class CopyBufferToBufferTask final : public Task {
public:
    // The srcBuffer for this Task is always a transfer buffer which is owned by the
    // UploadBufferManager. Thus we don't have to take a ref to it as the UploadBufferManager will
    // handle its refs and passing them to the Recording.
    static sk_sp<CopyBufferToBufferTask> Make(const Buffer* srcBuffer,
                                              size_t srcOffset,
                                              sk_sp<Buffer> dstBuffer,
                                              size_t dstOffset,
                                              size_t size);

    ~CopyBufferToBufferTask() override;

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            sk_sp<const RuntimeEffectDictionary>) override;

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

#if defined(SK_DUMP_TASKS)
    void dump(int index, const char* prefix) const override {
        if (index >= 0) {
            SkDebugf("%s%d: Copy BtoB Task: Src=%p Dst=%p\n", prefix, index, fSrcBuffer,
                fDstBuffer.get());
        } else {
            SkDebugf("%sCopy BtoB Task: Src=%p Dst=%p\n", prefix, fSrcBuffer, fDstBuffer.get());
        }
    }
#endif

private:
    CopyBufferToBufferTask(const Buffer* srcBuffer,
                           size_t srcOffset,
                           sk_sp<Buffer> dstBuffer,
                           size_t dstOffset,
                           size_t size);

    const Buffer* fSrcBuffer;
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

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            sk_sp<const RuntimeEffectDictionary>) override;

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

    bool visitProxies(const std::function<bool(const TextureProxy*)>& visitor,
                      bool readsOnly) override {
        // The texture is the source of the copy, so it's always read
        return visitor(fTextureProxy.get());
    }

#if defined(SK_DUMP_TASKS)
    void dump(int index, const char* prefix) const override {
        if (index >= 0) {
            SkDebugf("%s%d: Copy TtoB Task: Texture=%p Buffer=%p\n", prefix, index,
                fTextureProxy.get(), fBuffer.get());
        } else {
            SkDebugf("%sCopy TtoB Task: Texture=%p Buffer=%p\n", prefix, fTextureProxy.get(),
                fBuffer.get());
        }
    }
#endif

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

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            sk_sp<const RuntimeEffectDictionary>) override;

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

    bool visitProxies(const std::function<bool(const TextureProxy*)>& visitor,
                      bool readsOnly) override {
        // Only visit fDstProxy if readsOnly is false; fSrcProxy is the only texture being read.
        return visitor(fSrcProxy.get()) && (readsOnly || visitor(fDstProxy.get()));
    }

#if defined(SK_DUMP_TASKS)
    void dump(int index, const char* prefix) const override {
        if (index >= 0) {
            SkDebugf("%s%d: Copy TtoT Task: Src=%p Dst=%p\n", prefix, index, fSrcProxy.get(),
                                                              fDstProxy.get());
        } else {
            SkDebugf("%sCopy TtoT Task: Src=%p Dst=%p\n", prefix, fSrcProxy.get(), fDstProxy.get());
        }
    }
#endif

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

#endif // skgpu_graphite_task_CopyTask_DEFINED
