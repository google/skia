/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlCommandBuffer_DEFINED
#define skgpu_MtlCommandBuffer_DEFINED

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GpuWorkSubmission.h"

#include <memory>

#include "include/core/SkTypes.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {
class BlitCommandEncoder;
class Gpu;
class RenderCommandEncoder;

class CommandBuffer final : public skgpu::CommandBuffer {
public:
    static sk_sp<CommandBuffer> Make(const Gpu*);
    ~CommandBuffer() override;

    bool isFinished() {
        return (*fCommandBuffer).status == MTLCommandBufferStatusCompleted ||
               (*fCommandBuffer).status == MTLCommandBufferStatusError;

    }
    void waitUntilFinished() {
        // TODO: it's not clear what do to if status is Enqueued. Commit and then wait?
        if ((*fCommandBuffer).status == MTLCommandBufferStatusScheduled ||
            (*fCommandBuffer).status == MTLCommandBufferStatusCommitted) {
            [(*fCommandBuffer) waitUntilCompleted];
        }
        if (!this->isFinished()) {
            SkDebugf("Unfinished command buffer status: %d\n",
                     (int)(*fCommandBuffer).status);
            SkASSERT(false);
        }
    }
    bool commit();

private:
    CommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer, const Gpu* gpu);

    void onBeginRenderPass(const RenderPassDesc&,
                           const skgpu::Texture* colorTexture,
                           const skgpu::Texture* resolveTexture,
                           const skgpu::Texture* depthStencilTexture) override;
    void endRenderPass() override;

    void onBindGraphicsPipeline(const skgpu::GraphicsPipeline*) override;
    void onBindUniformBuffer(UniformSlot, const skgpu::Buffer*, size_t offset) override;
    void onBindVertexBuffers(const skgpu::Buffer* vertexBuffer, size_t vertexOffset,
                             const skgpu::Buffer* instanceBuffer, size_t instanceOffset) override;
    void onBindIndexBuffer(const skgpu::Buffer* indexBuffer, size_t offset) override;

    void onSetScissor(unsigned int left, unsigned int top,
                      unsigned int width, unsigned int height) override;
    void onSetViewport(float x, float y, float width, float height,
                       float minDepth, float maxDepth) override;
    void onSetStencilReference(unsigned int referenceValue) override;
    void onSetBlendConstants(std::array<float, 4> blendConstants) override;

    void onDraw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) override;
    void onDrawIndexed(PrimitiveType type, unsigned int baseIndex, unsigned int indexCount,
                       unsigned int baseVertex) override;
    void onDrawInstanced(PrimitiveType type,
                         unsigned int baseVertex, unsigned int vertexCount,
                         unsigned int baseInstance, unsigned int instanceCount) override;
    void onDrawIndexedInstanced(PrimitiveType type, unsigned int baseIndex,
                                unsigned int indexCount, unsigned int baseVertex,
                                unsigned int baseInstance, unsigned int instanceCount) override;

    void onCopyTextureToBuffer(const skgpu::Texture*,
                               SkIRect srcRect,
                               const skgpu::Buffer*,
                               size_t bufferOffset,
                               size_t bufferRowBytes) override;

    BlitCommandEncoder* getBlitCommandEncoder();
    void endBlitCommandEncoder();

    sk_cfp<id<MTLCommandBuffer>> fCommandBuffer;
    sk_sp<RenderCommandEncoder> fActiveRenderCommandEncoder;
    sk_sp<BlitCommandEncoder> fActiveBlitCommandEncoder;

    size_t fCurrentVertexStride = 0;
    size_t fCurrentInstanceStride = 0;
    id<MTLBuffer> fCurrentIndexBuffer;
    size_t fCurrentIndexBufferOffset = 0;

    const Gpu* fGpu;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlCommandBuffer_DEFINED
