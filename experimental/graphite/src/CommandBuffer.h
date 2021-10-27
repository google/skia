/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "include/core/SkRect.h"
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

    void trackResource(sk_sp<SkRefCnt> resource) {
        fTrackedResources.push_back(std::move(resource));
    }

    void beginRenderPass(const RenderPassDesc&);
    virtual void endRenderPass() = 0;

    //---------------------------------------------------------------
    // Can only be used within renderpasses
    //---------------------------------------------------------------
    void bindRenderPipeline(sk_sp<RenderPipeline> renderPipeline);
    void bindUniformBuffer(sk_sp<Buffer>, size_t bufferOffset);
    void bindVertexBuffers(sk_sp<Buffer> vertexBuffer, sk_sp<Buffer> instanceBuffer);

    void draw(PrimitiveType type, unsigned int vertexStart, unsigned int vertexCount) {
        this->onDraw(type, vertexStart, vertexCount);
        fHasWork = true;
    }

    //---------------------------------------------------------------
    // Can only be used outside renderpasses
    //---------------------------------------------------------------
    void copyTextureToBuffer(sk_sp<Texture>,
                             SkIRect srcRect,
                             sk_sp<Buffer>,
                             size_t bufferOffset,
                             size_t bufferRowBytes);

protected:
    CommandBuffer();

private:
    void releaseResources();

    virtual void onBeginRenderPass(const RenderPassDesc&) = 0;

    virtual void onBindRenderPipeline(const RenderPipeline*) = 0;
    virtual void onBindUniformBuffer(const Buffer*, size_t bufferOffset) = 0;
    virtual void onBindVertexBuffers(const Buffer* vertexBuffer, const Buffer* instanceBuffer) = 0;

    virtual void onDraw(PrimitiveType type, unsigned int vertexStart, unsigned int vertexCount) = 0;

    virtual void onCopyTextureToBuffer(const Texture*,
                                       SkIRect srcRect,
                                       const Buffer*,
                                       size_t bufferOffset,
                                       size_t bufferRowBytes) = 0;

    bool fHasWork = false;

    inline static constexpr int kInitialTrackedResourcesCount = 32;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<SkRefCnt>> fTrackedResources;
};

} // namespace skgpu

#endif // skgpu_CommandBuffer_DEFINED
