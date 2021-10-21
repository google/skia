/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

struct SkIRect;

namespace skgpu {
class Buffer;
class Gpu;
class RenderPipeline;
class Texture;

struct AttachmentDesc {
    sk_sp<Texture> fTexture; // the ref on this will be taken by the command buffer
    LoadOp fLoadOp;
    StoreOp fStoreOp;
};

struct RenderPassDesc {
    AttachmentDesc fColorAttachment;
    std::array<float, 4> fClearColor;
    AttachmentDesc fColorResolveAttachment;

    AttachmentDesc fStencilDepthAttachment;
    uint32_t fClearStencil;
    float fClearDepth;

    // TODO:
    // * bounds (TBD whether exact bounds vs. granular)
    // * input attachments
};

class CommandBuffer : public SkRefCnt {
public:
    ~CommandBuffer() override {
        this->releaseResources();
    }

    bool hasWork() { return fHasWork; }

    virtual void beginRenderPass(const RenderPassDesc&) = 0;
    virtual void endRenderPass() = 0;

    void setRenderPipeline(sk_sp<RenderPipeline> renderPipeline);

    virtual void copyTextureToBuffer(sk_sp<Texture>,
                                     SkIRect srcRect,
                                     sk_sp<Buffer>,
                                     size_t bufferOffset,
                                     size_t bufferRowBytes) = 0;

    void draw(PrimitiveType type, unsigned int vertexStart, unsigned int vertexCount) {
        this->onDraw(type, vertexStart, vertexCount);
        fHasWork = true;
    }

protected:
    CommandBuffer();

    void trackResource(sk_sp<SkRefCnt> resource) {
        fTrackedResources.push_back(std::move(resource));
    }
    void releaseResources();

    virtual void onSetRenderPipeline(sk_sp<RenderPipeline>&) = 0;

    virtual void onDraw(PrimitiveType type, unsigned int vertexStart, unsigned int vertexCount) = 0;

    bool fHasWork = false;

private:
    inline static constexpr int kInitialTrackedResourcesCount = 32;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<SkRefCnt>> fTrackedResources;
};

} // namespace skgpu

#endif // skgpu_CommandBuffer_DEFINED
