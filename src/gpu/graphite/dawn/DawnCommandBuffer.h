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

#include "webgpu/webgpu_cpp.h"

#include <memory>

#include "include/core/SkTypes.h"

namespace skgpu::graphite {
class DawnBlitCommandEncoder;
class DawnComputeCommandEncoder;
class DawnRenderCommandEncoder;
class DawnResourceProvider;
class DawnSharedContext;

class DawnCommandBuffer final : public CommandBuffer {
public:
    static sk_sp<DawnCommandBuffer> Make(wgpu::CommandBuffer cmdBuffer,
                                         const DawnSharedContext*,
                                         DawnResourceProvider*);
    ~DawnCommandBuffer() override;

    bool isFinished() {
        // TODO
        return false;
    }

    void waitUntilFinished(const SharedContext*) {
        // TODO
    }

    bool commit();


private:
    DawnCommandBuffer(wgpu::CommandBuffer cmdBuffer,
                     const DawnSharedContext* sharedContext,
                     DawnResourceProvider* resourceProvider);

    bool onAddRenderPass(const RenderPassDesc&,
                         const Texture* colorTexture,
                         const Texture* resolveTexture,
                         const Texture* depthStencilTexture,
                         const std::vector<std::unique_ptr<DrawPass>>& drawPasses) override;
    bool onAddComputePass(const ComputePassDesc&,
                          const ComputePipeline*,
                          const std::vector<ResourceBinding>& bindings) override;
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
                                SkIPoint dstPoint) override;
    bool onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) override;
    void onResetCommandBuffer() override;

    wgpu::CommandBuffer fCommandBuffer;
    wgpu::Buffer fCurrentIndexBuffer;
    const DawnSharedContext* fSharedContext;
    DawnResourceProvider* fResourceProvider;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnCommandBuffer_DEFINED
