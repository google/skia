/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_CommandBuffer_DEFINED
#define skgpu_graphite_CommandBuffer_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/GpuRefCnt.h"
#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/CommandTypes.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/Resource.h"

namespace skgpu {
class RefCntedCallback;
class MutableTextureState;
}

namespace skgpu::graphite {

class Buffer;
class DispatchGroup;
class DrawPass;
class SharedContext;
class GraphicsPipeline;
class Sampler;
class Texture;
class TextureProxy;

class CommandBuffer {
public:
    using DrawPassList = skia_private::TArray<std::unique_ptr<DrawPass>>;
    using DispatchGroupList = skia_private::TArray<std::unique_ptr<DispatchGroup>>;

    virtual ~CommandBuffer();

#ifdef SK_DEBUG
    bool hasWork() { return fHasWork; }
#endif

    // Takes a Usage ref on the Resource that will be released when the command buffer has finished
    // execution.
    void trackResource(sk_sp<Resource> resource);
    // Takes a CommandBuffer ref on the Resource that will be released when the command buffer has
    // finished execution. This allows a Resource to be returned to ResourceCache for reuse while
    // the CommandBuffer is still executing on the GPU. This is most commonly used for Textures or
    // Buffers which are only accessed via commands on a command buffer.
    void trackCommandBufferResource(sk_sp<Resource> resource);
    // Release all tracked Resources
    void resetCommandBuffer();

    // If any work is needed to create new resources for a fresh command buffer do that here.
    virtual bool setNewCommandBufferResources() = 0;

    void addFinishedProc(sk_sp<RefCntedCallback> finishedProc);
    void callFinishedProcs(bool success);

    virtual void addWaitSemaphores(size_t numWaitSemaphores,
                                   const BackendSemaphore* waitSemaphores) {}
    virtual void addSignalSemaphores(size_t numWaitSemaphores,
                                     const BackendSemaphore* signalSemaphores) {}
    virtual void prepareSurfaceForStateUpdate(SkSurface* targetSurface,
                                              const MutableTextureState* newState) {}

    void addBuffersToAsyncMapOnSubmit(SkSpan<const sk_sp<Buffer>>);
    SkSpan<const sk_sp<Buffer>> buffersToAsyncMapOnSubmit() const;

    bool addRenderPass(const RenderPassDesc&,
                       sk_sp<Texture> colorTexture,
                       sk_sp<Texture> resolveTexture,
                       sk_sp<Texture> depthStencilTexture,
                       SkRect viewport,
                       const DrawPassList& drawPasses);

    bool addComputePass(const DispatchGroupList& dispatchGroups);

    //---------------------------------------------------------------
    // Can only be used outside renderpasses
    //---------------------------------------------------------------
    bool copyBufferToBuffer(const Buffer* srcBuffer,
                            size_t srcOffset,
                            sk_sp<Buffer> dstBuffer,
                            size_t dstOffset,
                            size_t size);
    bool copyTextureToBuffer(sk_sp<Texture>,
                             SkIRect srcRect,
                             sk_sp<Buffer>,
                             size_t bufferOffset,
                             size_t bufferRowBytes);
    bool copyBufferToTexture(const Buffer*,
                             sk_sp<Texture>,
                             const BufferTextureCopyData*,
                             int count);
    bool copyTextureToTexture(sk_sp<Texture> src,
                              SkIRect srcRect,
                              sk_sp<Texture> dst,
                              SkIPoint dstPoint,
                              int mipLevel);
    bool synchronizeBufferToCpu(sk_sp<Buffer>);
    bool clearBuffer(const Buffer* buffer, size_t offset, size_t size);

    // This sets a translation to be applied to any subsequently added command, assuming these
    // commands are part of a translated replay of a Graphite recording.
    void setReplayTranslation(SkIVector translation) { fReplayTranslation = translation; }
    void clearReplayTranslation() { fReplayTranslation = {0, 0}; }

protected:
    CommandBuffer();

    SkISize fRenderPassSize;
    SkIVector fReplayTranslation;

private:
    // Release all tracked Resources
    void releaseResources();

    virtual void onResetCommandBuffer() = 0;

    virtual bool onAddRenderPass(const RenderPassDesc&,
                                 const Texture* colorTexture,
                                 const Texture* resolveTexture,
                                 const Texture* depthStencilTexture,
                                 SkRect viewport,
                                 const DrawPassList& drawPasses) = 0;

    virtual bool onAddComputePass(const DispatchGroupList& dispatchGroups) = 0;

    virtual bool onCopyBufferToBuffer(const Buffer* srcBuffer,
                                      size_t srcOffset,
                                      const Buffer* dstBuffer,
                                      size_t dstOffset,
                                      size_t size) = 0;
    virtual bool onCopyTextureToBuffer(const Texture*,
                                       SkIRect srcRect,
                                       const Buffer*,
                                       size_t bufferOffset,
                                       size_t bufferRowBytes) = 0;
    virtual bool onCopyBufferToTexture(const Buffer*,
                                       const Texture*,
                                       const BufferTextureCopyData*,
                                       int count) = 0;
    virtual bool onCopyTextureToTexture(const Texture* src,
                                        SkIRect srcRect,
                                        const Texture* dst,
                                        SkIPoint dstPoint,
                                        int mipLevel) = 0;
    virtual bool onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) = 0;
    virtual bool onClearBuffer(const Buffer*, size_t offset, size_t size) = 0;

#ifdef SK_DEBUG
    bool fHasWork = false;
#endif
    inline static constexpr int kInitialTrackedResourcesCount = 32;
    template <typename T>
    using TrackedResourceArray = skia_private::STArray<kInitialTrackedResourcesCount, T>;
    TrackedResourceArray<sk_sp<Resource>> fTrackedUsageResources;
    TrackedResourceArray<gr_cb<Resource>> fCommandBufferResources;
    skia_private::TArray<sk_sp<RefCntedCallback>> fFinishedProcs;
    skia_private::TArray<sk_sp<Buffer>> fBuffersToAsyncMap;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_CommandBuffer_DEFINED
