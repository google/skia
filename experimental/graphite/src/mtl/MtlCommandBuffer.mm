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
#include "experimental/graphite/src/mtl/MtlUtils.h"

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
#ifdef SK_BUILD_FOR_IOS
    if (IsAppInBackground()) {
        NSLog(@"CommandBuffer: Tried to commit command buffer while in background.\n");
        return false;
    }
#endif
    [(*fCommandBuffer) commit];

    // TODO: better error reporting
    if ((*fCommandBuffer).status == MTLCommandBufferStatusError) {
        NSString* description = (*fCommandBuffer).error.localizedDescription;
        const char* errorString = [description UTF8String];
        SkDebugf("Error submitting command buffer: %s\n", errorString);
    }

    return ((*fCommandBuffer).status != MTLCommandBufferStatusError);
}

bool CommandBuffer::onBeginRenderPass(const RenderPassDesc& renderPassDesc,
                                      const skgpu::Texture* colorTexture,
                                      const skgpu::Texture* resolveTexture,
                                      const skgpu::Texture* depthStencilTexture) {
    SkASSERT(!fActiveRenderCommandEncoder);
    this->endBlitCommandEncoder();
#ifdef SK_BUILD_FOR_IOS
    if (IsAppInBackground()) {
        NSLog(@"CommandBuffer: tried to create MTLRenderCommandEncoder while in background.\n");
        return false;
    }
#endif

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
    if (colorTexture) {
        // TODO: check Texture matches RenderPassDesc
        auto colorAttachment = (*descriptor).colorAttachments[0];
        colorAttachment.texture = ((Texture*)colorTexture)->mtlTexture();
        const std::array<float, 4>& clearColor = renderPassDesc.fClearColor;
        colorAttachment.clearColor =
                MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        colorAttachment.loadAction = mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
        colorAttachment.storeAction = mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];
        // Set up resolve attachment
        if (resolveTexture) {
            SkASSERT(renderPassDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore);
            // TODO: check Texture matches RenderPassDesc
            colorAttachment.resolveTexture = ((Texture*)resolveTexture)->mtlTexture();
            // Inclusion of a resolve texture implies the client wants to finish the
            // renderpass with a resolve.
            if (@available(macOS 10.12, iOS 10.0, *)) {
                if (colorAttachment.storeAction == MTLStoreActionStore) {
                    colorAttachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
                } else {
                    SkASSERT(colorAttachment.storeAction == MTLStoreActionDontCare);
                    colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
                }
            } else {
                // We expect at least Metal 2
                // TODO: Add error output
                SkASSERT(false);
            }
        }
    }

    // Set up stencil/depth attachment
    auto& depthStencilInfo = renderPassDesc.fDepthStencilAttachment;
    if (depthStencilTexture) {
        // TODO: check Texture matches RenderPassDesc
        id<MTLTexture> mtlTexture = ((Texture*)depthStencilTexture)->mtlTexture();
        if (FormatIsDepth(mtlTexture.pixelFormat)) {
            auto depthAttachment = (*descriptor).depthAttachment;
            depthAttachment.texture = mtlTexture;
            depthAttachment.clearDepth = renderPassDesc.fClearDepth;
            depthAttachment.loadAction =
                     mtlLoadAction[static_cast<int>(depthStencilInfo.fLoadOp)];
            depthAttachment.storeAction =
                     mtlStoreAction[static_cast<int>(depthStencilInfo.fStoreOp)];
        }
        if (FormatIsStencil(mtlTexture.pixelFormat)) {
            auto stencilAttachment = (*descriptor).stencilAttachment;
            stencilAttachment.texture = mtlTexture;
            stencilAttachment.clearStencil = renderPassDesc.fClearStencil;
            stencilAttachment.loadAction =
                     mtlLoadAction[static_cast<int>(depthStencilInfo.fLoadOp)];
            stencilAttachment.storeAction =
                     mtlStoreAction[static_cast<int>(depthStencilInfo.fStoreOp)];
        }
    } else {
        SkASSERT(!depthStencilInfo.fTextureInfo.isValid());
    }

    fActiveRenderCommandEncoder = RenderCommandEncoder::Make(fGpu,
                                                             fCommandBuffer.get(),
                                                             descriptor.get());

    this->trackResource(fActiveRenderCommandEncoder);

    return true;
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
#ifdef SK_BUILD_FOR_IOS
    if (IsAppInBackground()) {
        NSLog(@"CommandBuffer: tried to create MTLBlitCommandEncoder while in background.\n");
        return nullptr;
    }
#endif

    fActiveBlitCommandEncoder = BlitCommandEncoder::Make(fGpu, fCommandBuffer.get());

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
    auto depthStencilState = mtlPipeline->mtlDepthStencilState();
    fActiveRenderCommandEncoder->setDepthStencilState(depthStencilState);
    uint32_t stencilRefValue = mtlPipeline->stencilReferenceValue();
    fActiveRenderCommandEncoder->setStencilReferenceValue(stencilRefValue);

    fCurrentVertexStride = mtlPipeline->vertexStride();
    fCurrentInstanceStride = mtlPipeline->instanceStride();
}

void CommandBuffer::onBindUniformBuffer(UniformSlot slot,
                                        const skgpu::Buffer* uniformBuffer,
                                        size_t uniformOffset) {
    SkASSERT(fActiveRenderCommandEncoder);

    id<MTLBuffer> mtlBuffer = uniformBuffer ? static_cast<const Buffer*>(uniformBuffer)->mtlBuffer()
                                            : nullptr;

    unsigned int bufferIndex;
    switch(slot) {
        case UniformSlot::kRenderStep:
            bufferIndex = GraphicsPipeline::kRenderStepUniformBufferIndex;
            break;
        case UniformSlot::kPaint:
            bufferIndex = GraphicsPipeline::kPaintUniformBufferIndex;
            break;
    }

    fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, uniformOffset, bufferIndex);
    fActiveRenderCommandEncoder->setFragmentBuffer(mtlBuffer, uniformOffset, bufferIndex);
}

void CommandBuffer::onBindVertexBuffers(const skgpu::Buffer* vertexBuffer,
                                        size_t vertexOffset,
                                        const skgpu::Buffer* instanceBuffer,
                                        size_t instanceOffset) {
    SkASSERT(fActiveRenderCommandEncoder);

    if (vertexBuffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const Buffer*>(vertexBuffer)->mtlBuffer();
        // Metal requires buffer offsets to be aligned to the data type, which is at most 4 bytes
        // since we use [[attribute]] to automatically unpack float components into SIMD arrays.
        SkASSERT((vertexOffset & 0b11) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, vertexOffset,
                                                     GraphicsPipeline::kVertexBufferIndex);
    }
    if (instanceBuffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const Buffer*>(instanceBuffer)->mtlBuffer();
        SkASSERT((instanceOffset & 0b11) == 0);
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

void CommandBuffer::onSetScissor(unsigned int left, unsigned int top,
                                 unsigned int width, unsigned int height) {
    SkASSERT(fActiveRenderCommandEncoder);
    MTLScissorRect scissorRect = { left, top, width, height };
    fActiveRenderCommandEncoder->setScissorRect(scissorRect);
}

void CommandBuffer::onSetViewport(float x, float y, float width, float height,
                                  float minDepth, float maxDepth) {
    SkASSERT(fActiveRenderCommandEncoder);
    MTLViewport viewport = { x, y, width, height, minDepth, maxDepth };
    fActiveRenderCommandEncoder->setViewport(viewport);

    float invTwoW = 2.f / width;
    float invTwoH = 2.f / height;
    // Metal's framebuffer space has (0, 0) at the top left. This agrees with Skia's device coords.
    // However, in NDC (-1, -1) is the bottom left. So we flip the origin here (assuming all
    // surfaces we have are TopLeft origin).
    float rtAdjust[4] = {invTwoW, -invTwoH, -1.f - x * invTwoW, 1.f + y * invTwoH};
    fActiveRenderCommandEncoder->setVertexBytes(rtAdjust, 4 * sizeof(float),
                                                GraphicsPipeline::kIntrinsicUniformBufferIndex);
}

void CommandBuffer::onSetBlendConstants(std::array<float, 4> blendConstants) {
    SkASSERT(fActiveRenderCommandEncoder);

    fActiveRenderCommandEncoder->setBlendColor(blendConstants.data());
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

    if (@available(macOS 10.11, iOS 9.0, *)) {
        auto mtlPrimitiveType = graphite_to_mtl_primitive(type);
        size_t indexOffset =  fCurrentIndexBufferOffset + sizeof(uint16_t )* baseIndex;
        // Use the "instance" variant witha count of 1 so that we can pass in a base vertex
        // instead of rebinding a vertex buffer offset.
        fActiveRenderCommandEncoder->drawIndexedPrimitives(mtlPrimitiveType, indexCount,
                                                           MTLIndexTypeUInt16, fCurrentIndexBuffer,
                                                           indexOffset, 1, baseVertex, 0);

    } else {
        // TODO: Do nothing, fatal failure, or just the regular graphite error reporting overhaul?
        SkDebugf("[graphite] WARNING - Skipping unsupported draw call.\n");
    }
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

    if (@available(macOS 10.11, iOS 9.0, *)) {
        auto mtlPrimitiveType = graphite_to_mtl_primitive(type);
        size_t indexOffset =  fCurrentIndexBufferOffset + sizeof(uint16_t) * baseIndex;
        fActiveRenderCommandEncoder->drawIndexedPrimitives(mtlPrimitiveType, indexCount,
                                                           MTLIndexTypeUInt16, fCurrentIndexBuffer,
                                                           indexOffset, instanceCount,
                                                           baseVertex, baseInstance);
    } else {
        // TODO: Do nothing, fatal failure, or just the regular graphite error reporting overhaul?
        SkDebugf("[graphite] WARNING - Skipping unsupported draw call.\n");
    }
}

bool CommandBuffer::onCopyTextureToBuffer(const skgpu::Texture* texture,
                                          SkIRect srcRect,
                                          const skgpu::Buffer* buffer,
                                          size_t bufferOffset,
                                          size_t bufferRowBytes) {
    SkASSERT(!fActiveRenderCommandEncoder);

    id<MTLTexture> mtlTexture = static_cast<const Texture*>(texture)->mtlTexture();
    id<MTLBuffer> mtlBuffer = static_cast<const Buffer*>(buffer)->mtlBuffer();

    BlitCommandEncoder* blitCmdEncoder = this->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }

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
    return true;
}


} // namespace skgpu::mtl
