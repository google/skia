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
#include "src/gpu/graphite/CommandTypes.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/Resource.h"

#include <optional>

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
struct RenderPassDesc;
class ResourceProvider;
class Sampler;
class Texture;
class TextureProxy;

class CommandBuffer {
public:
    using DrawPassList = skia_private::TArray<std::unique_ptr<DrawPass>>;
    using DispatchGroupSpan = SkSpan<const std::unique_ptr<DispatchGroup>>;

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

    virtual bool startTimerQuery() { SK_ABORT("Timer query unsupported."); }
    virtual void endTimerQuery() { SK_ABORT("Timer query unsupported."); }
    virtual std::optional<GpuStats> gpuStats() { return {}; }

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

    // If any recorded draw requires a dst texture copy for blending, that texture must be provided
    // in `dstCopy`; otherwise it should be null. The `dstReadBounds` are in the same coordinate
    // space of the logical viewport *before* any replay translation is applied.
    //
    // The logical viewport is always (0,0,viewportDims) and matches the "device" coordinate space
    // of the higher-level SkDevices that recorded the rendering operations. The actual viewport
    // is automatically adjusted by the replay translation.
    //
    // If the RenderPassTask allocates a smaller color texture than the resolve texture, it can pass
    // a non-zero `resolveOffset` which is the the offset for resolving:
    // - The color texture's (0, 0, w, h) region.
    // - And store in the resolve texture's (resolveOffset.x, resolveOffset.y, w, h) region.
    bool addRenderPass(const RenderPassDesc&,
                       sk_sp<Texture> colorTexture,
                       sk_sp<Texture> resolveTexture,
                       sk_sp<Texture> depthStencilTexture,
                       const Texture* dstCopy,
                       SkIRect dstReadBounds,
                       SkIPoint resolveOffset,
                       SkISize viewportDims,
                       const DrawPassList& drawPasses);

    bool addComputePass(DispatchGroupSpan dispatchGroups);

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

    // This sets a translation and clip to be applied to any subsequently added command, assuming
    // these commands are part of a transformed replay of a Graphite recording. Returns whether the
    // clip and render target bounds have an intersection; if not, no draws need be replayed.
    bool setReplayTranslationAndClip(const SkIVector& translation,
                                     const SkIRect& clip,
                                     const SkIRect& renderTargetBounds);

    Protected isProtected() const { return fIsProtected; }

protected:
    CommandBuffer(Protected);

    // These are the color attachment bounds, intersected with any clip provided on replay.
    SkIRect fRenderPassBounds;
    // This is also the origin of the logical viewport relative to the target texture's (0,0) pixel.
    SkIVector fReplayTranslation;

    // The texture to use for implementing DstReadStrategy::kTextureCopy for the current render
    // pass. This is a bare pointer since the CopyTask that initializes the texture's contents
    // will have tracked the resource on the CommandBuffer already.
    std::pair<const Texture*, const Sampler*> fDstCopy;
    // Already includes replay translation and respects final color attachment bounds, but with
    // dimensions that equal fDstCopy's width and height.
    SkIRect fDstReadBounds;

    Protected fIsProtected;

private:
    // Release all tracked Resources
    void releaseResources();

    // Subclasses will hold their backend-specific ResourceProvider directly to avoid virtual calls
    // and access backend-specific behavior, but they can reflect it back to the base CommandBuffer
    // if it needs to make generic resources.
    virtual ResourceProvider* resourceProvider() const = 0;

    virtual void onResetCommandBuffer() = 0;

    // Renderpass, viewport bounds have already been adjusted by the replay translation. The render
    // pass bounds has been intersected with the color attachment bounds.
    virtual bool onAddRenderPass(const RenderPassDesc&,
                                 SkIRect renderPassBounds,
                                 const Texture* colorTexture,
                                 const Texture* resolveTexture,
                                 const Texture* depthStencilTexture,
                                 SkIPoint resolveOffset,
                                 SkIRect viewport,
                                 const DrawPassList& drawPasses) = 0;

    virtual bool onAddComputePass(DispatchGroupSpan dispatchGroups) = 0;

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
