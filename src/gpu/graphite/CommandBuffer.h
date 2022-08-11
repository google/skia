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
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/CommandTypes.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"

namespace skgpu {
class RefCntedCallback;
}

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

class CommandBuffer : public SkRefCnt {
public:
    ~CommandBuffer() override;

#ifdef SK_DEBUG
    bool hasWork() { return fHasWork; }
#endif

    void trackResource(sk_sp<Resource> resource);
    // Release all tracked Resources
    void releaseResources();

    void addFinishedProc(sk_sp<RefCntedCallback> finishedProc);
    void callFinishedProcs(bool success);

    bool addRenderPass(const RenderPassDesc&,
                       sk_sp<Texture> colorTexture,
                       sk_sp<Texture> resolveTexture,
                       sk_sp<Texture> depthStencilTexture,
                       const std::vector<std::unique_ptr<DrawPass>>& drawPasses);

    bool addComputePass(const ComputePassDesc&,
                        sk_sp<ComputePipeline> pipeline,
                        const std::vector<ResourceBinding>& bindings);

    //---------------------------------------------------------------
    // Can only be used outside renderpasses
    //---------------------------------------------------------------
    bool copyTextureToBuffer(sk_sp<Texture>,
                             SkIRect srcRect,
                             sk_sp<Buffer>,
                             size_t bufferOffset,
                             size_t bufferRowBytes);
    bool copyBufferToTexture(const Buffer*,
                             sk_sp<Texture>,
                             const BufferTextureCopyData*,
                             int count);

protected:
    CommandBuffer();

private:
    virtual bool onAddRenderPass(const RenderPassDesc&,
                                 const Texture* colorTexture,
                                 const Texture* resolveTexture,
                                 const Texture* depthStencilTexture,
                                 const std::vector<std::unique_ptr<DrawPass>>& drawPasses) = 0;

    virtual bool onAddComputePass(const ComputePassDesc&,
                                  const ComputePipeline*,
                                  const std::vector<ResourceBinding>& bindings) = 0;

    virtual bool onCopyTextureToBuffer(const Texture*,
                                       SkIRect srcRect,
                                       const Buffer*,
                                       size_t bufferOffset,
                                       size_t bufferRowBytes) = 0;
    virtual bool onCopyBufferToTexture(const Buffer*,
                                       const Texture*,
                                       const BufferTextureCopyData*,
                                       int count) = 0;

#ifdef SK_DEBUG
    bool fHasWork = false;
#endif

    inline static constexpr int kInitialTrackedResourcesCount = 32;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<Resource>> fTrackedResources;
    SkTArray<sk_sp<RefCntedCallback>> fFinishedProcs;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_CommandBuffer_DEFINED
