/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/CommandBuffer.h"

#include "experimental/graphite/src/GraphicsPipeline.h"
#include "src/core/SkTraceEvent.h"

#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"

namespace skgpu {

CommandBuffer::CommandBuffer() {}

void CommandBuffer::releaseResources() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    fTrackedResources.reset();
}

void CommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc) {
    this->onBeginRenderPass(renderPassDesc);

    auto& colorInfo = renderPassDesc.fColorAttachment;
    if (colorInfo.fTextureProxy) {
        this->trackResource(colorInfo.fTextureProxy->refTexture());
    }
    if (colorInfo.fStoreOp == StoreOp::kStore) {
        fHasWork = true;
    }
}

void CommandBuffer::bindGraphicsPipeline(sk_sp<GraphicsPipeline> graphicsPipeline) {
    this->onBindGraphicsPipeline(graphicsPipeline.get());
    this->trackResource(std::move(graphicsPipeline));
    fHasWork = true;
}

void CommandBuffer::bindUniformBuffer(sk_sp<Buffer> uniformBuffer, size_t offset) {
    this->onBindUniformBuffer(uniformBuffer.get(), offset);
    this->trackResource(std::move(uniformBuffer));
    fHasWork = true;
}

void CommandBuffer::bindVertexBuffers(sk_sp<Buffer> vertexBuffer, size_t vertexOffset,
                                      sk_sp<Buffer> instanceBuffer, size_t instanceOffset) {
    this->onBindVertexBuffers(vertexBuffer.get(), vertexOffset,
                              instanceBuffer.get(), instanceOffset);
    if (vertexBuffer) {
        this->trackResource(std::move(vertexBuffer));
    }
    if (instanceBuffer) {
        this->trackResource(std::move(instanceBuffer));
    }
    fHasWork = true;
}

void CommandBuffer::bindIndexBuffer(sk_sp<Buffer> indexBuffer, size_t bufferOffset) {
    this->onBindIndexBuffer(indexBuffer.get(), bufferOffset);
    if (indexBuffer) {
        this->trackResource(std::move(indexBuffer));
    }
    fHasWork = true;
}

static bool check_max_blit_width(int widthInPixels) {
    if (widthInPixels > 32767) {
        SkASSERT(false); // surfaces should not be this wide anyway
        return false;
    }
    return true;
}

void CommandBuffer::copyTextureToBuffer(sk_sp<skgpu::Texture> texture,
                                        SkIRect srcRect,
                                        sk_sp<skgpu::Buffer> buffer,
                                        size_t bufferOffset,
                                        size_t bufferRowBytes) {
    if (!check_max_blit_width(srcRect.width())) {
        return;
    }

    this->onCopyTextureToBuffer(texture.get(), srcRect, buffer.get(), bufferOffset, bufferRowBytes);

    this->trackResource(std::move(texture));
    this->trackResource(std::move(buffer));

    fHasWork = true;
}

} // namespace skgpu
