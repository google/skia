/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/CommandBuffer.h"

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "experimental/graphite/src/RenderPipeline.h"
#include "src/core/SkTraceEvent.h"

#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/Texture.h"

namespace skgpu {

CommandBuffer::CommandBuffer() {}

void CommandBuffer::releaseResources() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    fTrackedResources.reset();
}

void CommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc) {
    this->onBeginRenderPass(renderPassDesc);

    auto& colorInfo = renderPassDesc.fColorAttachment;
    if (colorInfo.fTexture) {
        this->trackResource(std::move(colorInfo.fTexture));
    }
    if (colorInfo.fStoreOp == StoreOp::kStore) {
        fHasWork = true;
    }
}

void CommandBuffer::bindRenderPipeline(sk_sp<RenderPipeline> renderPipeline) {
    this->onBindRenderPipeline(renderPipeline.get());
    this->trackResource(std::move(renderPipeline));
    fHasWork = true;
}

void CommandBuffer::bindUniformBuffer(sk_sp<Buffer> uniformBuffer, size_t offset) {
    this->onBindUniformBuffer(uniformBuffer.get(), offset);
    this->trackResource(std::move(uniformBuffer));
    fHasWork = true;
}

void CommandBuffer::bindVertexBuffers(sk_sp<Buffer> vertexBuffer, sk_sp<Buffer> instanceBuffer) {
    this->onBindVertexBuffers(vertexBuffer.get(), instanceBuffer.get());
    if (vertexBuffer) {
        this->trackResource(std::move(vertexBuffer));
    }
    if (instanceBuffer) {
        this->trackResource(std::move(instanceBuffer));
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
