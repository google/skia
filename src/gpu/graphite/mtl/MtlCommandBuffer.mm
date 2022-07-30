/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/mtl/MtlBlitCommandEncoder.h"
#include "src/gpu/graphite/mtl/MtlBuffer.h"
#include "src/gpu/graphite/mtl/MtlCaps.h"
#include "src/gpu/graphite/mtl/MtlComputeCommandEncoder.h"
#include "src/gpu/graphite/mtl/MtlComputePipeline.h"
#include "src/gpu/graphite/mtl/MtlGpu.h"
#include "src/gpu/graphite/mtl/MtlGraphicsPipeline.h"
#include "src/gpu/graphite/mtl/MtlRenderCommandEncoder.h"
#include "src/gpu/graphite/mtl/MtlSampler.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"
#include "src/gpu/graphite/mtl/MtlUtils.h"

namespace skgpu::graphite {

sk_sp<MtlCommandBuffer> MtlCommandBuffer::Make(const MtlGpu* gpu) {
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
     (*cmdBuffer).label = @"MtlCommandBuffer::Make";
#endif

    return sk_sp<MtlCommandBuffer>(new MtlCommandBuffer(std::move(cmdBuffer), gpu));
}

MtlCommandBuffer::MtlCommandBuffer(sk_cfp<id<MTLCommandBuffer>> cmdBuffer, const MtlGpu* gpu)
    : fCommandBuffer(std::move(cmdBuffer)), fGpu(gpu) {}

MtlCommandBuffer::~MtlCommandBuffer() {}

bool MtlCommandBuffer::commit() {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    this->endBlitCommandEncoder();
#ifdef SK_BUILD_FOR_IOS
    if (MtlIsAppInBackground()) {
        NSLog(@"CommandBuffer: Tried to commit command buffer while in background.\n");
        return false;
    }
#endif
    [(*fCommandBuffer) commit];

    if ((*fCommandBuffer).status == MTLCommandBufferStatusError) {
        NSString* description = (*fCommandBuffer).error.localizedDescription;
        const char* errorString = [description UTF8String];
        SKGPU_LOG_E("Failure submitting command buffer: %s", errorString);
    }

    return ((*fCommandBuffer).status != MTLCommandBufferStatusError);
}

bool MtlCommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                       const Texture* colorTexture,
                                       const Texture* resolveTexture,
                                       const Texture* depthStencilTexture,
                                       const std::vector<std::unique_ptr<DrawPass>>& drawPasses) {
    if (!this->beginRenderPass(renderPassDesc, colorTexture, resolveTexture, depthStencilTexture)) {
        return false;
    }

    for (size_t i = 0; i < drawPasses.size(); ++i) {
        this->addDrawPass(drawPasses[i].get());
    }

    this->endRenderPass();
    return true;
}

bool MtlCommandBuffer::onAddComputePass(const ComputePassDesc& computePassDesc,
                                        const ComputePipeline* pipeline,
                                        const std::vector<ResourceBinding>& bindings) {
    this->beginComputePass();
    this->bindComputePipeline(pipeline);
    for (const ResourceBinding& binding : bindings) {
        this->bindBuffer(
                binding.fResource.fBuffer.get(), binding.fResource.fOffset, binding.fIndex);
    }
    this->dispatchThreadgroups(computePassDesc.fGlobalDispatchSize,
                               computePassDesc.fLocalDispatchSize);
    this->endComputePass();
    return true;
}

bool MtlCommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc,
                                       const Texture* colorTexture,
                                       const Texture* resolveTexture,
                                       const Texture* depthStencilTexture) {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    this->endBlitCommandEncoder();
#ifdef SK_BUILD_FOR_IOS
    if (MtlIsAppInBackground()) {
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
    static_assert(std::size(mtlLoadAction) == kLoadOpCount);

    const static MTLStoreAction mtlStoreAction[] {
        MTLStoreActionStore,
        MTLStoreActionDontCare
    };
    static_assert((int)StoreOp::kStore == 0);
    static_assert((int)StoreOp::kDiscard == 1);
    static_assert(std::size(mtlStoreAction) == kStoreOpCount);

    sk_cfp<MTLRenderPassDescriptor*> descriptor([[MTLRenderPassDescriptor alloc] init]);
    // Set up color attachment.
    auto& colorInfo = renderPassDesc.fColorAttachment;
    if (colorTexture) {
        // TODO: check Texture matches RenderPassDesc
        auto colorAttachment = (*descriptor).colorAttachments[0];
        colorAttachment.texture = ((MtlTexture*)colorTexture)->mtlTexture();
        const std::array<float, 4>& clearColor = renderPassDesc.fClearColor;
        colorAttachment.clearColor =
                MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        colorAttachment.loadAction = mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
        colorAttachment.storeAction = mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];
        // Set up resolve attachment
        if (resolveTexture) {
            SkASSERT(renderPassDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore);
            // TODO: check Texture matches RenderPassDesc
            colorAttachment.resolveTexture = ((MtlTexture*)resolveTexture)->mtlTexture();
            // Inclusion of a resolve texture implies the client wants to finish the
            // renderpass with a resolve.
            if (@available(macOS 10.12, iOS 10.0, *)) {
                SkASSERT(colorAttachment.storeAction == MTLStoreActionDontCare);
                colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
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
        id<MTLTexture> mtlTexture = ((MtlTexture*)depthStencilTexture)->mtlTexture();
        if (MtlFormatIsDepth(mtlTexture.pixelFormat)) {
            auto depthAttachment = (*descriptor).depthAttachment;
            depthAttachment.texture = mtlTexture;
            depthAttachment.clearDepth = renderPassDesc.fClearDepth;
            depthAttachment.loadAction =
                     mtlLoadAction[static_cast<int>(depthStencilInfo.fLoadOp)];
            depthAttachment.storeAction =
                     mtlStoreAction[static_cast<int>(depthStencilInfo.fStoreOp)];
        }
        if (MtlFormatIsStencil(mtlTexture.pixelFormat)) {
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

    fActiveRenderCommandEncoder = MtlRenderCommandEncoder::Make(fGpu,
                                                                fCommandBuffer.get(),
                                                                descriptor.get());

    this->trackResource(fActiveRenderCommandEncoder);

    return true;
}

void MtlCommandBuffer::endRenderPass() {
    SkASSERT(fActiveRenderCommandEncoder);
    fActiveRenderCommandEncoder->endEncoding();
    fActiveRenderCommandEncoder.reset();
}

void MtlCommandBuffer::addDrawPass(const DrawPass* drawPass) {
    drawPass->addResourceRefs(this);

    for (auto[type, cmdPtr] : drawPass->commands()) {
        switch (type) {
            case DrawPassCommands::Type::kBindGraphicsPipeline: {
                auto bgp = static_cast<DrawPassCommands::BindGraphicsPipeline*>(cmdPtr);
                this->bindGraphicsPipeline(drawPass->getPipeline(bgp->fPipelineIndex));
                break;
            }
            case DrawPassCommands::Type::kSetBlendConstants: {
                auto sbc = static_cast<DrawPassCommands::SetBlendConstants*>(cmdPtr);
                this->setBlendConstants(sbc->fBlendConstants);
                break;
            }
            case DrawPassCommands::Type::kBindUniformBuffer: {
                auto bub = static_cast<DrawPassCommands::BindUniformBuffer*>(cmdPtr);
                this->bindUniformBuffer(bub->fInfo, bub->fSlot);
                break;
            }
            case DrawPassCommands::Type::kBindDrawBuffers: {
                auto bdb = static_cast<DrawPassCommands::BindDrawBuffers*>(cmdPtr);
                this->bindDrawBuffers(bdb->fVertices, bdb->fInstances, bdb->fIndices);
                break;
            }
            case DrawPassCommands::Type::kBindTexturesAndSamplers: {
                auto bts = static_cast<DrawPassCommands::BindTexturesAndSamplers*>(cmdPtr);
                for (int j = 0; j < bts->fNumTexSamplers; ++j) {
                    this->bindTextureAndSampler(drawPass->getTexture(bts->fTextureIndices[j]),
                                                drawPass->getSampler(bts->fSamplerIndices[j]),
                                                j);
                }
                break;
            }
            case DrawPassCommands::Type::kSetViewport: {
                auto sv = static_cast<DrawPassCommands::SetViewport*>(cmdPtr);
                this->setViewport(sv->fViewport.fLeft,
                                  sv->fViewport.fTop,
                                  sv->fViewport.width(),
                                  sv->fViewport.height(),
                                  sv->fMinDepth,
                                  sv->fMaxDepth);
                break;
            }
            case DrawPassCommands::Type::kSetScissor: {
                auto ss = static_cast<DrawPassCommands::SetScissor*>(cmdPtr);
                const SkIRect& rect = ss->fScissor;
                this->setScissor(rect.fLeft, rect.fTop, rect.width(), rect.height());
                break;
            }
            case DrawPassCommands::Type::kDraw: {
                auto draw = static_cast<DrawPassCommands::Draw*>(cmdPtr);
                this->draw(draw->fType, draw->fBaseVertex, draw->fVertexCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexed: {
                auto draw = static_cast<DrawPassCommands::DrawIndexed*>(cmdPtr);
                this->drawIndexed(draw->fType,
                                  draw->fBaseIndex,
                                  draw->fIndexCount,
                                  draw->fBaseVertex);
                break;
            }
            case DrawPassCommands::Type::kDrawInstanced: {
                auto draw = static_cast<DrawPassCommands::DrawInstanced*>(cmdPtr);
                this->drawInstanced(draw->fType,
                                    draw->fBaseVertex,
                                    draw->fVertexCount,
                                    draw->fBaseInstance,
                                    draw->fInstanceCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedInstanced: {
                auto draw = static_cast<DrawPassCommands::DrawIndexedInstanced*>(cmdPtr);
                this->drawIndexedInstanced(draw->fType,
                                           draw->fBaseIndex,
                                           draw->fIndexCount,
                                           draw->fBaseVertex,
                                           draw->fBaseInstance,
                                           draw->fInstanceCount);
                break;
            }
        }
    }
}

MtlBlitCommandEncoder* MtlCommandBuffer::getBlitCommandEncoder() {
    if (fActiveBlitCommandEncoder) {
        return fActiveBlitCommandEncoder.get();
    }
#ifdef SK_BUILD_FOR_IOS
    if (MtlIsAppInBackground()) {
        NSLog(@"CommandBuffer: tried to create MTLBlitCommandEncoder while in background.\n");
        return nullptr;
    }
#endif

    fActiveBlitCommandEncoder = MtlBlitCommandEncoder::Make(fGpu, fCommandBuffer.get());

    if (!fActiveBlitCommandEncoder) {
        return nullptr;
    }

    // We add the ref on the command buffer for the BlitCommandEncoder now so that we don't need
    // to add a ref for every copy we do.
    this->trackResource(fActiveBlitCommandEncoder);
    return fActiveBlitCommandEncoder.get();
}

void MtlCommandBuffer::endBlitCommandEncoder() {
    if (fActiveBlitCommandEncoder) {
        fActiveBlitCommandEncoder->endEncoding();
        fActiveBlitCommandEncoder.reset();
    }
}

void MtlCommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPipeline = static_cast<const MtlGraphicsPipeline*>(graphicsPipeline);
    auto pipelineState = mtlPipeline->mtlPipelineState();
    fActiveRenderCommandEncoder->setRenderPipelineState(pipelineState);
    auto depthStencilState = mtlPipeline->mtlDepthStencilState();
    fActiveRenderCommandEncoder->setDepthStencilState(depthStencilState);
    uint32_t stencilRefValue = mtlPipeline->stencilReferenceValue();
    fActiveRenderCommandEncoder->setStencilReferenceValue(stencilRefValue);

    fCurrentVertexStride = mtlPipeline->vertexStride();
    fCurrentInstanceStride = mtlPipeline->instanceStride();
}

void MtlCommandBuffer::bindUniformBuffer(const BindBufferInfo& info, UniformSlot slot) {
    SkASSERT(fActiveRenderCommandEncoder);

    id<MTLBuffer> mtlBuffer = info.fBuffer ?
            static_cast<const MtlBuffer*>(info.fBuffer)->mtlBuffer() : nullptr;

    unsigned int bufferIndex;
    switch(slot) {
        case UniformSlot::kRenderStep:
            bufferIndex = MtlGraphicsPipeline::kRenderStepUniformBufferIndex;
            break;
        case UniformSlot::kPaint:
            bufferIndex = MtlGraphicsPipeline::kPaintUniformBufferIndex;
            break;
    }

    fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, info.fOffset, bufferIndex);
    fActiveRenderCommandEncoder->setFragmentBuffer(mtlBuffer, info.fOffset, bufferIndex);
}

void MtlCommandBuffer::bindDrawBuffers(const BindBufferInfo& vertices,
                                       const BindBufferInfo& instances,
                                       const BindBufferInfo& indices) {
    this->bindVertexBuffers(vertices.fBuffer,
                            vertices.fOffset,
                            instances.fBuffer,
                            instances.fOffset);
    this->bindIndexBuffer(indices.fBuffer, indices.fOffset);
}

void MtlCommandBuffer::bindVertexBuffers(const Buffer* vertexBuffer,
                                         size_t vertexOffset,
                                         const Buffer* instanceBuffer,
                                         size_t instanceOffset) {
    SkASSERT(fActiveRenderCommandEncoder);

    if (vertexBuffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(vertexBuffer)->mtlBuffer();
        // Metal requires buffer offsets to be aligned to the data type, which is at most 4 bytes
        // since we use [[attribute]] to automatically unpack float components into SIMD arrays.
        SkASSERT((vertexOffset & 0b11) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, vertexOffset,
                                                     MtlGraphicsPipeline::kVertexBufferIndex);
    }
    if (instanceBuffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(instanceBuffer)->mtlBuffer();
        SkASSERT((instanceOffset & 0b11) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, instanceOffset,
                                                     MtlGraphicsPipeline::kInstanceBufferIndex);
    }
}

void MtlCommandBuffer::bindIndexBuffer(const Buffer* indexBuffer, size_t offset) {
    if (indexBuffer) {
        fCurrentIndexBuffer = static_cast<const MtlBuffer*>(indexBuffer)->mtlBuffer();
        fCurrentIndexBufferOffset = offset;
    } else {
        fCurrentIndexBuffer = nil;
        fCurrentIndexBufferOffset = 0;
    }
}

void MtlCommandBuffer::bindTextureAndSampler(const Texture* texture,
                                             const Sampler* sampler,
                                             unsigned int bindIndex) {
    SkASSERT(texture && sampler);

    id<MTLTexture> mtlTexture = ((const MtlTexture*)texture)->mtlTexture();
    id<MTLSamplerState> mtlSamplerState = ((const MtlSampler*)sampler)->mtlSamplerState();
    fActiveRenderCommandEncoder->setFragmentTexture(mtlTexture, bindIndex);
    fActiveRenderCommandEncoder->setFragmentSamplerState(mtlSamplerState, bindIndex);
}

void MtlCommandBuffer::setScissor(unsigned int left, unsigned int top,
                                  unsigned int width, unsigned int height) {
    SkASSERT(fActiveRenderCommandEncoder);
    MTLScissorRect scissorRect = { left, top, width, height };
    fActiveRenderCommandEncoder->setScissorRect(scissorRect);
}

void MtlCommandBuffer::setViewport(float x, float y, float width, float height,
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
                                                MtlGraphicsPipeline::kIntrinsicUniformBufferIndex);
}

void MtlCommandBuffer::setBlendConstants(float* blendConstants) {
    SkASSERT(fActiveRenderCommandEncoder);

    fActiveRenderCommandEncoder->setBlendColor(blendConstants);
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

void MtlCommandBuffer::draw(PrimitiveType type,
                            unsigned int baseVertex,
                            unsigned int vertexCount) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPrimitiveType = graphite_to_mtl_primitive(type);

    fActiveRenderCommandEncoder->drawPrimitives(mtlPrimitiveType, baseVertex, vertexCount);
}

void MtlCommandBuffer::drawIndexed(PrimitiveType type, unsigned int baseIndex,
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
        // TODO: Do nothing, fatal failure, or just the regular graphite error reporting?
        SKGPU_LOG_E("Skipping unsupported draw call.");
    }
}

void MtlCommandBuffer::drawInstanced(PrimitiveType type, unsigned int baseVertex,
                                     unsigned int vertexCount, unsigned int baseInstance,
                                     unsigned int instanceCount) {
    SkASSERT(fActiveRenderCommandEncoder);

    auto mtlPrimitiveType = graphite_to_mtl_primitive(type);

    // This ordering is correct
    fActiveRenderCommandEncoder->drawPrimitives(mtlPrimitiveType, baseVertex, vertexCount,
                                                instanceCount, baseInstance);
}

void MtlCommandBuffer::drawIndexedInstanced(PrimitiveType type,
                                            unsigned int baseIndex,
                                            unsigned int indexCount,
                                            unsigned int baseVertex,
                                            unsigned int baseInstance,
                                            unsigned int instanceCount) {
    SkASSERT(fActiveRenderCommandEncoder);

    if (@available(macOS 10.11, iOS 9.0, *)) {
        auto mtlPrimitiveType = graphite_to_mtl_primitive(type);
        size_t indexOffset =  fCurrentIndexBufferOffset + sizeof(uint16_t) * baseIndex;
        fActiveRenderCommandEncoder->drawIndexedPrimitives(mtlPrimitiveType, indexCount,
                                                           MTLIndexTypeUInt16, fCurrentIndexBuffer,
                                                           indexOffset, instanceCount,
                                                           baseVertex, baseInstance);
    } else {
        // TODO: Do nothing, fatal failure, or just the regular graphite error reporting?
        SKGPU_LOG_W("Skipping unsupported draw call.");
    }
}

void MtlCommandBuffer::beginComputePass() {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    this->endBlitCommandEncoder();
    fActiveComputeCommandEncoder = MtlComputeCommandEncoder::Make(fGpu, fCommandBuffer.get());
}

void MtlCommandBuffer::bindComputePipeline(const ComputePipeline* computePipeline) {
    SkASSERT(fActiveComputeCommandEncoder);

    auto mtlPipeline = static_cast<const MtlComputePipeline*>(computePipeline);
    fActiveComputeCommandEncoder->setComputePipelineState(mtlPipeline->mtlPipelineState());
}

void MtlCommandBuffer::bindBuffer(const Buffer* buffer, unsigned int offset, unsigned int index) {
    SkASSERT(fActiveComputeCommandEncoder);

    id<MTLBuffer> mtlBuffer = buffer ? static_cast<const MtlBuffer*>(buffer)->mtlBuffer() : nullptr;
    fActiveComputeCommandEncoder->setBuffer(mtlBuffer, offset, index);
}

void MtlCommandBuffer::dispatchThreadgroups(const WorkgroupSize& globalSize,
                                            const WorkgroupSize& localSize) {
    SkASSERT(fActiveComputeCommandEncoder);
    fActiveComputeCommandEncoder->dispatchThreadgroups(globalSize, localSize);
}

void MtlCommandBuffer::endComputePass() {
    SkASSERT(fActiveComputeCommandEncoder);
    fActiveComputeCommandEncoder->endEncoding();
    fActiveComputeCommandEncoder.reset();
}

static bool check_max_blit_width(int widthInPixels) {
    if (widthInPixels > 32767) {
        SkASSERT(false); // surfaces should not be this wide anyway
        return false;
    }
    return true;
}

bool MtlCommandBuffer::onCopyTextureToBuffer(const Texture* texture,
                                             SkIRect srcRect,
                                             const Buffer* buffer,
                                             size_t bufferOffset,
                                             size_t bufferRowBytes) {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);

    if (!check_max_blit_width(srcRect.width())) {
        return false;
    }

    id<MTLTexture> mtlTexture = static_cast<const MtlTexture*>(texture)->mtlTexture();
    id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(buffer)->mtlBuffer();

    MtlBlitCommandEncoder* blitCmdEncoder = this->getBlitCommandEncoder();
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

bool MtlCommandBuffer::onCopyBufferToTexture(const Buffer* buffer,
                                             const Texture* texture,
                                             const BufferTextureCopyData* copyData,
                                             int count) {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);

    id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(buffer)->mtlBuffer();
    id<MTLTexture> mtlTexture = static_cast<const MtlTexture*>(texture)->mtlTexture();

    MtlBlitCommandEncoder* blitCmdEncoder = this->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->pushDebugGroup(@"uploadToTexture");
#endif
    for (int i = 0; i < count; ++i) {
        if (!check_max_blit_width(copyData[i].fRect.width())) {
            return false;
        }

        blitCmdEncoder->copyFromBuffer(mtlBuffer,
                                       copyData[i].fBufferOffset,
                                       copyData[i].fBufferRowBytes,
                                       mtlTexture,
                                       copyData[i].fRect,
                                       copyData[i].fMipLevel);
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->popDebugGroup();
#endif
    return true;
}


} // namespace skgpu::graphite
