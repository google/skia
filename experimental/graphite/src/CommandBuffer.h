/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

#include "experimental/graphite/src/DrawTypes.h"
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

struct SkIRect;

namespace skgpu {
class Buffer;
class Gpu;
class GraphicsPipeline;
class Texture;
class TextureProxy;

struct AttachmentDesc {
    sk_sp<TextureProxy> fTextureProxy;
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
    void bindGraphicsPipeline(sk_sp<GraphicsPipeline> graphicsPipeline);
    void bindUniformBuffer(sk_sp<Buffer>, size_t bufferOffset);
    void bindVertexBuffers(sk_sp<Buffer> vertexBuffer, size_t vertexOffset,
                           sk_sp<Buffer> instanceBuffer, size_t instanceOffset);
    void bindIndexBuffer(sk_sp<Buffer> indexBuffer, size_t bufferOffset);

    void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) {
        this->onDraw(type, baseVertex, vertexCount);
        fHasWork = true;
    }
    void drawIndexed(PrimitiveType type, unsigned int baseIndex, unsigned int indexCount,
                     unsigned int baseVertex) {
        this->onDrawIndexed(type, baseIndex, indexCount, baseVertex);
        fHasWork = true;
    }
    void drawInstanced(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount,
                       unsigned int baseInstance, unsigned int instanceCount) {
        this->onDrawInstanced(type, baseVertex, vertexCount, baseInstance, instanceCount);
        fHasWork = true;
    }
    void drawIndexedInstanced(PrimitiveType type, unsigned int baseIndex, unsigned int indexCount,
                              unsigned int baseVertex, unsigned int baseInstance,
                              unsigned int instanceCount) {
        this->onDrawIndexedInstanced(type, baseIndex, indexCount, baseVertex, baseInstance,
                                     instanceCount);
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

    virtual void onBindGraphicsPipeline(const GraphicsPipeline*) = 0;
    virtual void onBindUniformBuffer(const Buffer*, size_t bufferOffset) = 0;
    virtual void onBindVertexBuffers(const Buffer* vertexBuffer, size_t vertexOffset,
                                     const Buffer* instanceBuffer, size_t instanceOffset) = 0;
    virtual void onBindIndexBuffer(const Buffer* indexBuffer, size_t bufferOffset) = 0;

    virtual void onDraw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) = 0;
    virtual void onDrawIndexed(PrimitiveType type, unsigned int baseIndex, unsigned int indexCount,
                               unsigned int baseVertex) = 0;
    virtual void onDrawInstanced(PrimitiveType type,
                                 unsigned int baseVertex, unsigned int vertexCount,
                                 unsigned int baseInstance, unsigned int instanceCount) = 0;
    virtual void onDrawIndexedInstanced(PrimitiveType type, unsigned int baseIndex,
                                        unsigned int indexCount, unsigned int baseVertex,
                                        unsigned int baseInstance, unsigned int instanceCount) = 0;

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
