/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

std::unique_ptr<DawnCommandBuffer> DawnCommandBuffer::Make(wgpu::CommandBuffer cmdBuffer,
                                                           const DawnSharedContext* sharedContext,
                                                           DawnResourceProvider* resourceProvider) {
    return nullptr;
}

DawnCommandBuffer::DawnCommandBuffer(wgpu::CommandBuffer cmdBuffer,
                                     const DawnSharedContext* sharedContext,
                                     DawnResourceProvider* resourceProvider)
        : fCommandBuffer(std::move(cmdBuffer))
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider) {

}

DawnCommandBuffer::~DawnCommandBuffer() {}

bool DawnCommandBuffer::commit() {
    // TODO
    return false;
}

bool DawnCommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                        const Texture* colorTexture,
                                        const Texture* resolveTexture,
                                        const Texture* depthStencilTexture,
                                        SkRect viewport,
                                        const std::vector<std::unique_ptr<DrawPass>>& drawPasses) {
    // TODO
    return false;
}

bool DawnCommandBuffer::onAddComputePass(const ComputePassDesc& computePassDesc,
                                         const ComputePipeline* pipeline,
                                         const std::vector<ResourceBinding>& bindings) {
    // TODO
    return false;
}

bool DawnCommandBuffer::onCopyTextureToBuffer(const Texture* texture,
                                              SkIRect srcRect,
                                              const Buffer* buffer,
                                              size_t bufferOffset,
                                              size_t bufferRowBytes) {
    return false;
}

bool DawnCommandBuffer::onCopyBufferToTexture(const Buffer* buffer,
                                              const Texture* texture,
                                              const BufferTextureCopyData* copyData,
                                              int count) {
    return false;
}

bool DawnCommandBuffer::onCopyTextureToTexture(const Texture* src,
                                               SkIRect srcRect,
                                               const Texture* dst,
                                               SkIPoint dstPoint) {
    return false;
}

bool DawnCommandBuffer::onSynchronizeBufferToCpu(const Buffer* buffer,
                                                 bool* outDidResultInWork) {
    return false;
}

void DawnCommandBuffer::onResetCommandBuffer() {
}

bool DawnCommandBuffer::setNewCommandBufferResources() {
    return false;
}

} // namespace skgpu::graphite
