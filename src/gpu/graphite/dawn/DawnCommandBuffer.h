/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnCommandBuffer_DEFINED
#define skgpu_graphite_DawnCommandBuffer_DEFINED

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include <optional>

namespace skgpu::graphite {
class ComputePipeline;
class DawnBuffer;
class DawnComputePipeline;
class DawnQueueManager;
class DawnSharedContext;
class DawnTexture;
class DispatchGroup;
struct WorkgroupSize;

class DawnCommandBuffer final : public CommandBuffer {
public:
    static std::unique_ptr<DawnCommandBuffer> Make(const DawnSharedContext*, DawnResourceProvider*);
    ~DawnCommandBuffer() override;

    wgpu::CommandBuffer finishEncoding();

#if defined(SK_DEBUG)
    bool hasActivePassEncoder() const {
        return fActiveRenderPassEncoder || fActiveComputePassEncoder;
    }
#endif

private:
    DawnCommandBuffer(const DawnSharedContext* sharedContext,
                      DawnResourceProvider* resourceProvider);

    ResourceProvider* resourceProvider() const override { return fResourceProvider; }

    void onResetCommandBuffer() override;
    bool setNewCommandBufferResources() override;

    bool onAddRenderPass(const RenderPassDesc&,
                         SkIRect renderPassBounds,
                         const Texture* colorTexture,
                         const Texture* resolveTexture,
                         const Texture* depthStencilTexture,
                         SkIRect viewport,
                         const DrawPassList&) override;
    bool onAddComputePass(DispatchGroupSpan) override;

    // Methods for populating a Dawn RenderPassEncoder:
    bool beginRenderPass(const RenderPassDesc&,
                         SkIRect renderPassBounds,
                         const Texture* colorTexture,
                         const Texture* resolveTexture,
                         const Texture* depthStencilTexture);
    bool loadMSAAFromResolveAndBeginRenderPassEncoder(
            const RenderPassDesc& frontendRenderPassDesc,
            const wgpu::RenderPassDescriptor& wgpuRenderPassDesc,
            const DawnTexture* msaaTexture);
    bool doBlitWithDraw(const wgpu::RenderPassEncoder& renderEncoder,
                        const RenderPassDesc& frontendRenderPassDesc,
                        const wgpu::TextureView& sourceTextureView,
                        int width,
                        int height);
    void endRenderPass();

    bool addDrawPass(const DrawPass*);

    bool bindGraphicsPipeline(const GraphicsPipeline*);
    void setBlendConstants(float* blendConstants);

    void bindUniformBuffer(const BindBufferInfo& info, UniformSlot);
    void bindDrawBuffers(const BindBufferInfo& vertices,
                         const BindBufferInfo& instances,
                         const BindBufferInfo& indices,
                         const BindBufferInfo& indirect);

    void bindTextureAndSamplers(const DrawPass& drawPass,
                                const DrawPassCommands::BindTexturesAndSamplers& command);

    void setScissor(unsigned int left, unsigned int top, unsigned int width, unsigned int height);
    bool updateIntrinsicUniforms(SkIRect viewport);
    void setViewport(SkIRect viewport);

    void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount);
    void drawIndexed(PrimitiveType type,
                     unsigned int baseIndex,
                     unsigned int indexCount,
                     unsigned int baseVertex);
    void drawInstanced(PrimitiveType type,
                       unsigned int baseVertex,
                       unsigned int vertexCount,
                       unsigned int baseInstance,
                       unsigned int instanceCount);
    void drawIndexedInstanced(PrimitiveType type,
                              unsigned int baseIndex,
                              unsigned int indexCount,
                              unsigned int baseVertex,
                              unsigned int baseInstance,
                              unsigned int instanceCount);
    void drawIndirect(PrimitiveType type);
    void drawIndexedIndirect(PrimitiveType type);

    // Methods for populating a Dawn ComputePassEncoder:
    void beginComputePass();
    void bindComputePipeline(const ComputePipeline*);
    void bindDispatchResources(const DispatchGroup&, const DispatchGroup::Dispatch&);
    void dispatchWorkgroups(const WorkgroupSize& globalSize);
    void dispatchWorkgroupsIndirect(const Buffer* indirectBuffer, size_t indirectBufferOffset);
    void endComputePass();

    // Methods for doing texture/buffer to texture/buffer copying:
    bool onCopyBufferToBuffer(const Buffer* srcBuffer,
                              size_t srcOffset,
                              const Buffer* dstBuffer,
                              size_t dstOffset,
                              size_t size) override;
    bool onCopyTextureToBuffer(const Texture*,
                               SkIRect srcRect,
                               const Buffer*,
                               size_t bufferOffset,
                               size_t bufferRowBytes) override;
    bool onCopyBufferToTexture(const Buffer*,
                               const Texture*,
                               const BufferTextureCopyData* copyData,
                               int count) override;
    bool onCopyTextureToTexture(const Texture* src,
                                SkIRect srcRect,
                                const Texture* dst,
                                SkIPoint dstPoint,
                                int mipLevel) override;
    bool onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) override;
    bool onClearBuffer(const Buffer*, size_t offset, size_t size) override;

    // Commiting uniform buffers' changes if any before drawing
    void syncUniformBuffers();

    bool fBoundUniformBuffersDirty = false;

    std::array<BindBufferInfo, DawnGraphicsPipeline::kNumUniformBuffers> fBoundUniforms;

    wgpu::CommandEncoder fCommandEncoder;
    wgpu::RenderPassEncoder fActiveRenderPassEncoder;
    wgpu::ComputePassEncoder fActiveComputePassEncoder;

    wgpu::Buffer fCurrentIndirectBuffer;
    size_t fCurrentIndirectBufferOffset = 0;

    const DawnGraphicsPipeline* fActiveGraphicsPipeline = nullptr;
    const DawnComputePipeline* fActiveComputePipeline = nullptr;
    const DawnSharedContext* fSharedContext;
    DawnResourceProvider* fResourceProvider;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnCommandBuffer_DEFINED
