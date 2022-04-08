/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlCommandBuffer_DEFINED
#define skgpu_graphite_MtlCommandBuffer_DEFINED

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/Log.h"

#include <memory>

#include "include/core/SkTypes.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {
class MtlBlitCommandEncoder;
class MtlGpu;
class MtlRenderCommandEncoder;

class MtlCommandBuffer final : public CommandBuffer {
public:
    static sk_sp<MtlCommandBuffer> Make(const MtlGpu*);
    ~MtlCommandBuffer() override;

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
            SKGPU_LOG_E("Unfinished command buffer status: %d",
                        (int)(*fCommandBuffer).status);
            SkASSERT(false);
        }
    }
    bool commit();

private:
    MtlCommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer, const MtlGpu* gpu);

    bool onBeginRenderPass(const RenderPassDesc&,
                           const Texture* colorTexture,
                           const Texture* resolveTexture,
                           const Texture* depthStencilTexture) override;
    void endRenderPass() override;

    void onBindGraphicsPipeline(const GraphicsPipeline*) override;
    void onBindUniformBuffer(UniformSlot, const Buffer*, size_t offset) override;
    void onBindVertexBuffers(const Buffer* vertexBuffer, size_t vertexOffset,
                             const Buffer* instanceBuffer, size_t instanceOffset) override;
    void onBindIndexBuffer(const Buffer* indexBuffer, size_t offset) override;


    void onBindTextureAndSampler(sk_sp<Texture>,
                                 sk_sp<Sampler>,
                                 unsigned int bindIndex) override;

    void onSetScissor(unsigned int left, unsigned int top,
                      unsigned int width, unsigned int height) override;
    void onSetViewport(float x, float y, float width, float height,
                       float minDepth, float maxDepth) override;
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

    bool onCopyTextureToBuffer(const Texture*,
                               SkIRect srcRect,
                               const Buffer*,
                               size_t bufferOffset,
                               size_t bufferRowBytes) override;
    bool onCopyBufferToTexture(const Buffer*,
                               const Texture*,
                               const BufferTextureCopyData* copyData,
                               int count) override;

    MtlBlitCommandEncoder* getBlitCommandEncoder();
    void endBlitCommandEncoder();

    sk_cfp<id<MTLCommandBuffer>> fCommandBuffer;
    sk_sp<MtlRenderCommandEncoder> fActiveRenderCommandEncoder;
    sk_sp<MtlBlitCommandEncoder> fActiveBlitCommandEncoder;

    size_t fCurrentVertexStride = 0;
    size_t fCurrentInstanceStride = 0;
    id<MTLBuffer> fCurrentIndexBuffer;
    size_t fCurrentIndexBufferOffset = 0;

    const MtlGpu* fGpu;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlCommandBuffer_DEFINED
