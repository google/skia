/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlCommandBuffer.h"

#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/mtl/MtlBlitCommandEncoder.h"
#include "experimental/graphite/src/mtl/MtlBuffer.h"
#include "experimental/graphite/src/mtl/MtlCaps.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlGraphicsPipeline.h"
#include "experimental/graphite/src/mtl/MtlRenderCommandEncoder.h"
#include "experimental/graphite/src/mtl/MtlTexture.h"

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
    SkASSERT(!fActiveRenderCommandEncoder);
    this->endBlitCommandEncoder();
    [(*fCommandBuffer) commit];

    // TODO: better error reporting
    if ((*fCommandBuffer).status == MTLCommandBufferStatusError) {
        NSString* description = (*fCommandBuffer).error.localizedDescription;
        const char* errorString = [description UTF8String];
        SkDebugf("Error submitting command buffer: %s\n", errorString);
    }

    return ((*fCommandBuffer).status != MTLCommandBufferStatusError);
}

void CommandBuffer::onBeginRenderPass(const RenderPassDesc& renderPassDesc) {
    SkASSERT(!fActiveRenderCommandEncoder);
    this->endBlitCommandEncoder();

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
    auto& colorInfo = renderPassDesc.fColorAttachment;
    if (colorInfo.fTextureProxy) {
        auto colorAttachment = (*descriptor).colorAttachments[0];
        const Texture* colorTexture = (const Texture*)colorInfo.fTextureProxy->texture();
        colorAttachment.texture = colorTexture->mtlTexture();
        const std::array<float, 4>& clearColor = renderPassDesc.fClearColor;
        colorAttachment.clearColor =
                MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        colorAttachment.loadAction = mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
        colorAttachment.storeAction = mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];
    }

    // TODO:
    // * setup resolve
    // * set up stencil and depth

    fActiveRenderCommandEncoder = RenderCommandEncoder::Make(fCommandBuffer.get(),
                                                             descriptor.get());

    this->trackResource(fActiveRenderCommandEncoder);
}

void CommandBuffer::endRenderPass() {
    SkASSERT(fActiveRenderCommandEncoder);
    fActiveRenderCommandEncoder->endEncoding();
    fActiveRenderCommandEncoder.reset();
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

void CommandBuffer::endBlitCommandEncoder() {
    if (fActiveBlitCommandEncoder) {
        fActiveBlitCommandEncoder->endEncoding();
        fActiveBlitCommandEncoder.reset();
    }
}

void CommandBuffer::onBindGraphicsPipeline(const skgpu::GraphicsPipeline* graphicsPipeline) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPipeline = static_cast<const GraphicsPipeline*>(graphicsPipeline);
    auto pipelineState = mtlPipeline->mtlPipelineState();
    fActiveRenderCommandEncoder->setRenderPipelineState(pipelineState);
    fCurrentVertexStride = mtlPipeline->vertexStride();
    fCurrentInstanceStride = mtlPipeline->instanceStride();
}

void CommandBuffer::onBindUniformBuffer(const skgpu::Buffer* uniformBuffer,
                                        size_t uniformOffset) {
    SkASSERT(fActiveRenderCommandEncoder);

    id<MTLBuffer> mtlBuffer = static_cast<const Buffer*>(uniformBuffer)->mtlBuffer();

    if (fGpu->mtlCaps().isMac()) {
        SkASSERT((uniformOffset & 0xFF) == 0);
    } else {
        SkASSERT((uniformOffset & 0xF) == 0);
    }
    fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, uniformOffset,
                                                 GraphicsPipeline::kUniformBufferIndex);
    fActiveRenderCommandEncoder->setFragmentBuffer(mtlBuffer, uniformOffset,
                                                   GraphicsPipeline::kUniformBufferIndex);
}

void CommandBuffer::onBindVertexBuffers(const skgpu::Buffer* vertexBuffer,
                                        size_t vertexOffset,
                                        const skgpu::Buffer* instanceBuffer,
                                        size_t instanceOffset) {
    SkASSERT(fActiveRenderCommandEncoder);

    if (vertexBuffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const Buffer*>(vertexBuffer)->mtlBuffer();
        SkASSERT((vertexOffset & 0xF) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, vertexOffset,
                                                     GraphicsPipeline::kVertexBufferIndex);
    }
    if (instanceBuffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const Buffer*>(instanceBuffer)->mtlBuffer();
        SkASSERT((instanceOffset & 0xF) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, instanceOffset,
                                                     GraphicsPipeline::kInstanceBufferIndex);
    }
}

void CommandBuffer::onBindIndexBuffer(const skgpu::Buffer* indexBuffer, size_t offset) {
    if (indexBuffer) {
        fCurrentIndexBuffer = static_cast<const Buffer*>(indexBuffer)->mtlBuffer();
        fCurrentIndexBufferOffset = offset;
    } else {
        fCurrentIndexBuffer = nil;
        fCurrentIndexBufferOffset = 0;
    }
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

void CommandBuffer::onDraw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPrimitiveType = graphite_to_mtl_primitive(type);

    fActiveRenderCommandEncoder->drawPrimitives(mtlPrimitiveType, baseVertex, vertexCount);
}

void CommandBuffer::onDrawIndexed(PrimitiveType type, unsigned int baseIndex,
                                  unsigned int indexCount, unsigned int baseVertex) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPrimitiveType = graphite_to_mtl_primitive(type);

    fActiveRenderCommandEncoder->setVertexBufferOffset(baseVertex * fCurrentVertexStride,
                                                       GraphicsPipeline::kVertexBufferIndex);
    size_t indexOffset =  fCurrentIndexBufferOffset + sizeof(uint16_t )* baseIndex;
    fActiveRenderCommandEncoder->drawIndexedPrimitives(mtlPrimitiveType, indexCount,
                                                       MTLIndexTypeUInt16, fCurrentIndexBuffer,
                                                       indexOffset);
}

void CommandBuffer::onDrawInstanced(PrimitiveType type, unsigned int baseVertex,
                                    unsigned int vertexCount, unsigned int baseInstance,
                                    unsigned int instanceCount) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPrimitiveType = graphite_to_mtl_primitive(type);

    // This ordering is correct
    fActiveRenderCommandEncoder->drawPrimitives(mtlPrimitiveType, baseVertex, vertexCount,
                                                instanceCount, baseInstance);
}

void CommandBuffer::onDrawIndexedInstanced(PrimitiveType type, unsigned int baseIndex,
                                           unsigned int indexCount, unsigned int baseVertex,
                                           unsigned int baseInstance, unsigned int instanceCount) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPrimitiveType = graphite_to_mtl_primitive(type);

    fActiveRenderCommandEncoder->setVertexBufferOffset(baseVertex * fCurrentVertexStride,
                                                       GraphicsPipeline::kVertexBufferIndex);
    fActiveRenderCommandEncoder->setVertexBufferOffset(baseInstance * fCurrentInstanceStride,
                                                       GraphicsPipeline::kInstanceBufferIndex);
    size_t indexOffset =  fCurrentIndexBufferOffset + sizeof(uint16_t) * baseIndex;
    fActiveRenderCommandEncoder->drawIndexedPrimitives(mtlPrimitiveType, indexCount,
                                                       MTLIndexTypeUInt16, fCurrentIndexBuffer,
                                                       indexOffset, instanceCount,
                                                       baseVertex, baseInstance);
}

void CommandBuffer::onCopyTextureToBuffer(const skgpu::Texture* texture,
                                          SkIRect srcRect,
                                          const skgpu::Buffer* buffer,
                                          size_t bufferOffset,
                                          size_t bufferRowBytes) {
    SkASSERT(!fActiveRenderCommandEncoder);

    id<MTLTexture> mtlTexture = static_cast<const Texture*>(texture)->mtlTexture();
    id<MTLBuffer> mtlBuffer = static_cast<const Buffer*>(buffer)->mtlBuffer();

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
}


} // namespace skgpu::mtl
