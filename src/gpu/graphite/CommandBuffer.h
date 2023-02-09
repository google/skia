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
#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/CommandTypes.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"

namespace skgpu {
class RefCntedCallback;
}

#ifdef SK_ENABLE_PIET_GPU
namespace skgpu::piet {
class Scene;
}
#endif

namespace skgpu::graphite {

class Buffer;
class ComputePipeline;
class DrawPass;
class SharedContext;
class GraphicsPipeline;
class Resource;
class Sampler;
class Texture;
class TextureProxy;

class CommandBuffer {
public:
    virtual ~CommandBuffer();

#ifdef SK_DEBUG
    bool hasWork() { return fHasWork; }
#endif

    void trackResource(sk_sp<Resource> resource);
    // Release all tracked Resources
    void resetCommandBuffer();

    // If any work is needed to create new resources for a fresh command buffer do that here.
    virtual bool setNewCommandBufferResources() = 0;

    void addFinishedProc(sk_sp<RefCntedCallback> finishedProc);
    void callFinishedProcs(bool success);

    bool addRenderPass(const RenderPassDesc&,
                       sk_sp<Texture> colorTexture,
                       sk_sp<Texture> resolveTexture,
                       sk_sp<Texture> depthStencilTexture,
                       SkRect viewport,
                       const std::vector<std::unique_ptr<DrawPass>>& drawPasses);

    bool addComputePass(const ComputePassDesc&,
                        sk_sp<ComputePipeline> pipeline,
                        const std::vector<ResourceBinding>& bindings);

    //---------------------------------------------------------------
    // Can only be used outside renderpasses
    //---------------------------------------------------------------
    bool copyBufferToBuffer(sk_sp<Buffer> srcBuffer,
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
                              SkIPoint dstPoint);
    bool synchronizeBufferToCpu(sk_sp<Buffer>);
    bool clearBuffer(const Buffer* buffer, size_t offset, size_t size);

#ifdef SK_ENABLE_PIET_GPU
    void renderPietScene(const skgpu::piet::Scene& scene, sk_sp<Texture> target);
#endif

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
                                 const std::vector<std::unique_ptr<DrawPass>>& drawPasses) = 0;

    virtual bool onAddComputePass(const ComputePassDesc&,
                                  const ComputePipeline*,
                                  const std::vector<ResourceBinding>& bindings) = 0;

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
                                        SkIPoint dstPoint) = 0;
    virtual bool onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) = 0;
    virtual bool onClearBuffer(const Buffer*, size_t offset, size_t size) = 0;

#ifdef SK_ENABLE_PIET_GPU
    virtual void onRenderPietScene(const skgpu::piet::Scene& scene, const Texture* target) = 0;
#endif

#ifdef SK_DEBUG
    bool fHasWork = false;
#endif

    inline static constexpr int kInitialTrackedResourcesCount = 32;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<Resource>> fTrackedResources;
    SkTArray<sk_sp<RefCntedCallback>> fFinishedProcs;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_CommandBuffer_DEFINED
