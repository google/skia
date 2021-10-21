/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlCommandBuffer.h"

#include "experimental/graphite/src/mtl/MtlBlitCommandEncoder.h"
#include "experimental/graphite/src/mtl/MtlBuffer.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlRenderCommandEncoder.h"
#include "experimental/graphite/src/mtl/MtlRenderPipeline.h"
#include "experimental/graphite/src/mtl/MtlTexture.h"
#include "include/core/SkRect.h"

namespace skgpu::mtl {

sk_sp<CommandBuffer> CommandBuffer::Make(const Gpu* gpu) {
    sk_cfp<id<MTLCommandBuffer>> cmdBuffer;
    id<MTLCommandQueue> queue = gpu->queue();
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        sk_cfp<MTLCommandBufferDescriptor*> desc([[MTLCommandBufferDescriptor alloc] init]);
        (*desc).retainedReferences = NO;
#ifdef SK_ENABLE_MTL_DEBUG_INFO
        (*desc).errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
#endif
        // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
        cmdBuffer.reset([[queue commandBufferWithDescriptor:desc.get()] retain]);
    } else {
        // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
        cmdBuffer.reset([[queue commandBufferWithUnretainedReferences] retain]);
    }
    if (cmdBuffer == nil) {
        return nullptr;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
     (*cmdBuffer).label = @"CommandBuffer::Make";
#endif

    return sk_sp<CommandBuffer>(new CommandBuffer(std::move(cmdBuffer), gpu));
}

CommandBuffer::CommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer, const Gpu* gpu)
    : fCommandBuffer(std::move(cmdBuffer)), fGpu(gpu) {}

CommandBuffer::~CommandBuffer() {}

bool CommandBuffer::commit() {
    // TODO: end any encoding
    [(*fCommandBuffer) commit];

    // TODO: better error reporting
    if ((*fCommandBuffer).status == MTLCommandBufferStatusError) {
        NSString* description = (*fCommandBuffer).error.localizedDescription;
        const char* errorString = [description UTF8String];
        SkDebugf("Error submitting command buffer: %s\n", errorString);
    }

    return ((*fCommandBuffer).status != MTLCommandBufferStatusError);
}

void CommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc) {
    SkASSERT(!fActiveRenderCommandEncoder);

    auto& colorInfo = renderPassDesc.fColorAttachment;

    const static MTLLoadAction mtlLoadAction[] {
        MTLLoadActionLoad,
        MTLLoadActionClear,
        MTLLoadActionDontCare
    };
    static_assert((int)LoadOp::kLoad == 0);
    static_assert((int)LoadOp::kClear == 1);
    static_assert((int)LoadOp::kDiscard == 2);
    static_assert(SK_ARRAY_COUNT(mtlLoadAction) == kLoadOpCount);

    const static MTLStoreAction mtlStoreAction[] {
        MTLStoreActionStore,
        MTLStoreActionDontCare
    };
    static_assert((int)StoreOp::kStore == 0);
    static_assert((int)StoreOp::kDiscard == 1);
    static_assert(SK_ARRAY_COUNT(mtlStoreAction) == kStoreOpCount);

    sk_cfp<MTLRenderPassDescriptor*> descriptor([[MTLRenderPassDescriptor alloc] init]);
    // Set up color attachment.
    auto colorAttachment = (*descriptor).colorAttachments[0];
    Texture* colorTexture = (Texture*)colorInfo.fTexture.get();
    colorAttachment.texture = colorTexture->mtlTexture();
    const std::array<float, 4>& clearColor = renderPassDesc.fClearColor;
    colorAttachment.clearColor =
            MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    colorAttachment.loadAction = mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
    colorAttachment.storeAction = mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];

    // TODO:
    // * setup resolve
    // * set up stencil and depth

    fActiveRenderCommandEncoder = RenderCommandEncoder::Make(fCommandBuffer.get(),
                                                             descriptor.get());

    if (colorTexture) {
        this->trackResource(std::move(colorInfo.fTexture));
    }
    this->trackResource(fActiveRenderCommandEncoder);

    if (colorInfo.fStoreOp == StoreOp::kStore) {
        fHasWork = true;
    }
}

void CommandBuffer::endRenderPass() {
    SkASSERT(fActiveRenderCommandEncoder);
    fActiveRenderCommandEncoder->endEncoding();
    fActiveRenderCommandEncoder.reset();
}

static bool check_max_blit_width(int widthInPixels) {
    if (widthInPixels > 32767) {
        SkASSERT(false); // surfaces should not be this wide anyway
        return false;
    }
    return true;
}

BlitCommandEncoder* CommandBuffer::getBlitCommandEncoder() {
    if (fActiveBlitCommandEncoder) {
        return fActiveBlitCommandEncoder.get();
    }

    fActiveBlitCommandEncoder = BlitCommandEncoder::Make(fCommandBuffer.get());

    if (!fActiveBlitCommandEncoder) {
        return nullptr;
    }

    // We add the ref on the command buffer for the BlitCommandEncoder now so that we don't need
    // to add a ref for every copy we do.
    this->trackResource(fActiveBlitCommandEncoder);
    return fActiveBlitCommandEncoder.get();
}

void CommandBuffer::copyTextureToBuffer(sk_sp<skgpu::Texture> texture,
                                        SkIRect srcRect,
                                        sk_sp<skgpu::Buffer> buffer,
                                        size_t bufferOffset,
                                        size_t bufferRowBytes) {
    SkASSERT(!fActiveRenderCommandEncoder);

    if (!check_max_blit_width(srcRect.width())) {
        return;
    }

    id<MTLTexture> mtlTexture = static_cast<Texture*>(texture.get())->mtlTexture();
    id<MTLBuffer> mtlBuffer = static_cast<Buffer*>(buffer.get())->mtlBuffer();

    BlitCommandEncoder* blitCmdEncoder = this->getBlitCommandEncoder();

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->pushDebugGroup(@"readOrTransferPixels");
#endif
    blitCmdEncoder->copyFromTexture(mtlTexture, srcRect, mtlBuffer, bufferOffset, bufferRowBytes);

    if (fGpu->mtlCaps().isMac()) {
#ifdef SK_BUILD_FOR_MAC
        // Sync GPU data back to the CPU
        blitCmdEncoder->synchronizeResource(mtlBuffer);
#endif
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->popDebugGroup();
#endif

    this->trackResource(std::move(texture));
    this->trackResource(std::move(buffer));

    fHasWork = true;
}

void CommandBuffer::onSetRenderPipeline(sk_sp<skgpu::RenderPipeline>& renderPipeline) {
    SkASSERT(fActiveRenderCommandEncoder);

    id<MTLRenderPipelineState> pipelineState =
            static_cast<RenderPipeline*>(renderPipeline.get())->mtlPipelineState();
    fActiveRenderCommandEncoder->setRenderPipelineState(pipelineState);
}

static MTLPrimitiveType graphite_to_mtl_primitive(PrimitiveType primitiveType) {
    const static MTLPrimitiveType mtlPrimitiveType[] {
        MTLPrimitiveTypeTriangle,
        MTLPrimitiveTypeTriangleStrip,
        MTLPrimitiveTypePoint,
    };
    static_assert((int)PrimitiveType::kTriangles == 0);
    static_assert((int)PrimitiveType::kTriangleStrip == 1);
    static_assert((int)PrimitiveType::kPoints == 2);

    SkASSERT(primitiveType <= PrimitiveType::kPoints);
    return mtlPrimitiveType[static_cast<int>(primitiveType)];
}

void CommandBuffer::onDraw(PrimitiveType type, unsigned int vertexStart, unsigned int vertexCount) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPrimitiveType = graphite_to_mtl_primitive(type);

    fActiveRenderCommandEncoder->drawPrimitives(mtlPrimitiveType, vertexStart, vertexCount);
}

} // namespace skgpu::mtl
