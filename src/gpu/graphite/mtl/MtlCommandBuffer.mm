/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"

#include "include/gpu/graphite/BackendSemaphore.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/mtl/MtlBlitCommandEncoder.h"
#include "src/gpu/graphite/mtl/MtlBuffer.h"
#include "src/gpu/graphite/mtl/MtlCaps.h"
#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlComputeCommandEncoder.h"
#include "src/gpu/graphite/mtl/MtlComputePipeline.h"
#include "src/gpu/graphite/mtl/MtlGraphicsPipeline.h"
#include "src/gpu/graphite/mtl/MtlRenderCommandEncoder.h"
#include "src/gpu/graphite/mtl/MtlSampler.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

namespace skgpu::graphite {

std::unique_ptr<MtlCommandBuffer> MtlCommandBuffer::Make(id<MTLCommandQueue> queue,
                                                         const MtlSharedContext* sharedContext,
                                                         MtlResourceProvider* resourceProvider) {
    auto commandBuffer = std::unique_ptr<MtlCommandBuffer>(
            new MtlCommandBuffer(queue, sharedContext, resourceProvider));
    if (!commandBuffer) {
        return nullptr;
    }
    if (!commandBuffer->createNewMTLCommandBuffer()) {
        return nullptr;
    }
    return commandBuffer;
}

MtlCommandBuffer::MtlCommandBuffer(id<MTLCommandQueue> queue,
                                   const MtlSharedContext* sharedContext,
                                   MtlResourceProvider* resourceProvider)
        : CommandBuffer(Protected::kNo)  // Metal doesn't support protected memory
        , fQueue(queue)
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider) {}

MtlCommandBuffer::~MtlCommandBuffer() {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    SkASSERT(!fActiveBlitCommandEncoder);
}

bool MtlCommandBuffer::setNewCommandBufferResources() {
    return this->createNewMTLCommandBuffer();
}

bool MtlCommandBuffer::createNewMTLCommandBuffer() {
    SkASSERT(fCommandBuffer == nil);

    // Inserting a pool here so the autorelease occurs when we return and the
    // only remaining ref is the retain below.
    @autoreleasepool {
        if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
            sk_cfp<MTLCommandBufferDescriptor*> desc([[MTLCommandBufferDescriptor alloc] init]);
            (*desc).retainedReferences = NO;
#ifdef SK_ENABLE_MTL_DEBUG_INFO
            (*desc).errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
#endif
            // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
            fCommandBuffer.reset([[fQueue commandBufferWithDescriptor:desc.get()] retain]);
        } else {
            // We add a retain here because the command buffer is set to autorelease (not alloc or copy)
            fCommandBuffer.reset([[fQueue commandBufferWithUnretainedReferences] retain]);
        }
    }
    return fCommandBuffer != nil;
}

bool MtlCommandBuffer::commit() {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    this->endBlitCommandEncoder();
    [(*fCommandBuffer) commit];

    if ((*fCommandBuffer).status == MTLCommandBufferStatusError) {
        NSString* description = (*fCommandBuffer).error.localizedDescription;
        const char* errorString = [description UTF8String];
        SKGPU_LOG_E("Failure submitting command buffer: %s", errorString);
    }

    return ((*fCommandBuffer).status != MTLCommandBufferStatusError);
}

void MtlCommandBuffer::onResetCommandBuffer() {
    fCommandBuffer.reset();
    fActiveRenderCommandEncoder.reset();
    fActiveComputeCommandEncoder.reset();
    fActiveBlitCommandEncoder.reset();
    fCurrentIndexBuffer = nil;
    fCurrentIndexBufferOffset = 0;
}

void MtlCommandBuffer::addWaitSemaphores(size_t numWaitSemaphores,
                                         const BackendSemaphore* waitSemaphores) {
    if (!waitSemaphores) {
        SkASSERT(numWaitSemaphores == 0);
        return;
    }

    // Can only insert events with no active encoder
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    this->endBlitCommandEncoder();
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        for (size_t i = 0; i < numWaitSemaphores; ++i) {
            auto semaphore = waitSemaphores[i];
            if (semaphore.isValid() && semaphore.backend() == BackendApi::kMetal) {
                id<MTLEvent> mtlEvent =
                        (__bridge id<MTLEvent>)BackendSemaphores::GetMtlEvent(semaphore);
                [(*fCommandBuffer) encodeWaitForEvent:mtlEvent
                                                value:BackendSemaphores::GetMtlValue(semaphore)];
            }
        }
    }
}

void MtlCommandBuffer::addSignalSemaphores(size_t numSignalSemaphores,
                                           const BackendSemaphore* signalSemaphores) {
    if (!signalSemaphores) {
        SkASSERT(numSignalSemaphores == 0);
        return;
    }

    // Can only insert events with no active encoder
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    this->endBlitCommandEncoder();

    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        for (size_t i = 0; i < numSignalSemaphores; ++i) {
            auto semaphore = signalSemaphores[i];
            if (semaphore.isValid() && semaphore.backend() == BackendApi::kMetal) {
                id<MTLEvent> mtlEvent = (__bridge id<MTLEvent>)BackendSemaphores::GetMtlEvent;
                [(*fCommandBuffer) encodeSignalEvent:mtlEvent
                                               value:BackendSemaphores::GetMtlValue(semaphore)];
            }
        }
    }
}

bool MtlCommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                       SkIRect renderPassBounds,
                                       const Texture* colorTexture,
                                       const Texture* resolveTexture,
                                       const Texture* depthStencilTexture,
                                       SkIPoint resolveOffset,
                                       SkIRect viewport,
                                       const DrawPassList& drawPasses) {
    SkASSERT(resolveOffset.isZero());
    if (!this->beginRenderPass(renderPassDesc, colorTexture, resolveTexture, depthStencilTexture)) {
        return false;
    }

    this->setViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height(), 0, 1);
    this->updateIntrinsicUniforms(viewport);

    for (const auto& drawPass : drawPasses) {
        this->addDrawPass(drawPass.get());
    }

    this->endRenderPass();
    return true;
}

bool MtlCommandBuffer::onAddComputePass(DispatchGroupSpan groups) {
    this->beginComputePass();
    for (const auto& group : groups) {
        group->addResourceRefs(this);
        for (const auto& dispatch : group->dispatches()) {
            this->bindComputePipeline(group->getPipeline(dispatch.fPipelineIndex));
            for (const ResourceBinding& binding : dispatch.fBindings) {
                if (const BindBufferInfo* buffer = std::get_if<BindBufferInfo>(&binding.fResource)) {
                    this->bindBuffer(buffer->fBuffer, buffer->fOffset, binding.fIndex);
                } else if (const TextureIndex* texIdx =
                                   std::get_if<TextureIndex>(&binding.fResource)) {
                    SkASSERT(texIdx);
                    this->bindTexture(group->getTexture(texIdx->fValue), binding.fIndex);
                } else {
                    const SamplerIndex* samplerIdx = std::get_if<SamplerIndex>(&binding.fResource);
                    SkASSERT(samplerIdx);
                    this->bindSampler(group->getSampler(samplerIdx->fValue), binding.fIndex);
                }
            }
            SkASSERT(fActiveComputeCommandEncoder);
            for (const ComputeStep::WorkgroupBufferDesc& wgBuf : dispatch.fWorkgroupBuffers) {
                fActiveComputeCommandEncoder->setThreadgroupMemoryLength(
                        SkAlignTo(wgBuf.size, 16),
                        wgBuf.index);
            }
            if (const WorkgroupSize* globalSize =
                        std::get_if<WorkgroupSize>(&dispatch.fGlobalSizeOrIndirect)) {
                this->dispatchThreadgroups(*globalSize, dispatch.fLocalSize);
            } else {
                SkASSERT(std::holds_alternative<BindBufferInfo>(dispatch.fGlobalSizeOrIndirect));
                const BindBufferInfo& indirect =
                        *std::get_if<BindBufferInfo>(&dispatch.fGlobalSizeOrIndirect);
                this->dispatchThreadgroupsIndirect(
                        dispatch.fLocalSize, indirect.fBuffer, indirect.fOffset);
            }
        }
    }
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
    // Validate attachment descs and textures
    const auto& colorInfo = renderPassDesc.fColorAttachment;
    const auto& resolveInfo = renderPassDesc.fColorResolveAttachment;
    const auto& depthStencilInfo = renderPassDesc.fDepthStencilAttachment;
    SkASSERT(colorTexture ? colorInfo.isCompatible(colorTexture->textureInfo())
                          : colorInfo.fFormat == TextureFormat::kUnsupported);
    SkASSERT(resolveTexture ? resolveInfo.isCompatible(resolveTexture->textureInfo())
                            : resolveInfo.fFormat == TextureFormat::kUnsupported);
    SkASSERT(depthStencilTexture ? depthStencilInfo.isCompatible(depthStencilTexture->textureInfo())
                                 : depthStencilInfo.fFormat == TextureFormat::kUnsupported);

    // Set up color attachment.
    bool loadMSAAFromResolve = false;
    if (colorTexture) {
        auto colorAttachment = (*descriptor).colorAttachments[0];
        colorAttachment.texture = ((const MtlTexture*)colorTexture)->mtlTexture();
        const std::array<float, 4>& clearColor = renderPassDesc.fClearColor;
        colorAttachment.clearColor =
                MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        colorAttachment.loadAction = mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
        colorAttachment.storeAction = mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];

        // Set up resolve attachment
        if (resolveTexture) {
            SkASSERT(resolveInfo.fStoreOp == StoreOp::kStore);

            colorAttachment.resolveTexture = ((const MtlTexture*)resolveTexture)->mtlTexture();
            // Inclusion of a resolve texture implies the client wants to finish the
            // renderpass with a resolve.
            if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
                SkASSERT(colorAttachment.storeAction == MTLStoreActionDontCare);
                colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
            } else {
                // We expect at least Metal 2
                // TODO: Add error output
                SkASSERT(false);
            }
            // But it also means we have to load the resolve texture into the MSAA color attachment
            loadMSAAFromResolve = resolveInfo.fLoadOp == LoadOp::kLoad;
            // TODO: If the color resolve texture is read-only we can use a private (vs. memoryless)
            // msaa attachment that's coupled to the framebuffer and the StoreAndMultisampleResolve
            // action instead of loading as a draw.
        }
    }

    // Set up stencil/depth attachment
    if (depthStencilTexture) {
        id<MTLTexture> mtlTexture = ((const MtlTexture*)depthStencilTexture)->mtlTexture();
        if (TextureFormatHasDepth(depthStencilInfo.fFormat)) {
            auto depthAttachment = (*descriptor).depthAttachment;
            depthAttachment.texture = mtlTexture;
            depthAttachment.clearDepth = renderPassDesc.fClearDepth;
            depthAttachment.loadAction =
                     mtlLoadAction[static_cast<int>(depthStencilInfo.fLoadOp)];
            depthAttachment.storeAction =
                     mtlStoreAction[static_cast<int>(depthStencilInfo.fStoreOp)];
        }
        if (TextureFormatHasStencil(depthStencilInfo.fFormat)) {
            auto stencilAttachment = (*descriptor).stencilAttachment;
            stencilAttachment.texture = mtlTexture;
            stencilAttachment.clearStencil = renderPassDesc.fClearStencil;
            stencilAttachment.loadAction =
                     mtlLoadAction[static_cast<int>(depthStencilInfo.fLoadOp)];
            stencilAttachment.storeAction =
                     mtlStoreAction[static_cast<int>(depthStencilInfo.fStoreOp)];
        }
    }

    fActiveRenderCommandEncoder = MtlRenderCommandEncoder::Make(fSharedContext,
                                                                fCommandBuffer.get(),
                                                                descriptor.get());
    this->trackResource(fActiveRenderCommandEncoder);

    if (loadMSAAFromResolve) {
        // Manually load the contents of the resolve texture into the MSAA attachment as a draw,
        // so the actual load op for the MSAA attachment had better have been discard.
        SkASSERT(colorInfo.fLoadOp == LoadOp::kDiscard);
        auto loadPipeline = fResourceProvider->findOrCreateLoadMSAAPipeline(renderPassDesc);
        if (!loadPipeline) {
            SKGPU_LOG_E("Unable to create pipeline to load resolve texture into MSAA attachment");
            return false;
        }
        this->bindGraphicsPipeline(loadPipeline.get());
        // The load msaa pipeline takes no uniforms, no vertex/instance attributes and only uses
        // one texture that does not require a sampler.
        fActiveRenderCommandEncoder->setFragmentTexture(
                ((const MtlTexture*) resolveTexture)->mtlTexture(), 0);
        this->draw(PrimitiveType::kTriangleStrip, 0, 4);
    }

    return true;
}

void MtlCommandBuffer::endRenderPass() {
    SkASSERT(fActiveRenderCommandEncoder);
    fActiveRenderCommandEncoder->endEncoding();
    fActiveRenderCommandEncoder.reset();
    fDrawIsOffscreen = false;
}

void MtlCommandBuffer::addDrawPass(const DrawPass* drawPass) {
    const SkIRect replayedBounds = drawPass->bounds().makeOffset(fReplayTranslation.x(),
                                                                 fReplayTranslation.y());
    if (!SkIRect::Intersects(replayedBounds, fRenderPassBounds)) {
        // The entire DrawPass is offscreen given the replay translation so skip adding any
        // commands. When the DrawPass is partially offscreen individual draw commands will be
        // culled while preserving state changing commands.
        return;
    }

    drawPass->addResourceRefs(this);

    for (auto[type, cmdPtr] : drawPass->commands()) {
        // Skip draw commands if they'd be offscreen.
        if (fDrawIsOffscreen) {
            switch (type) {
                case DrawPassCommands::Type::kDraw:
                case DrawPassCommands::Type::kDrawIndexed:
                case DrawPassCommands::Type::kDrawInstanced:
                case DrawPassCommands::Type::kDrawIndexedInstanced:
                    continue;
                default:
                    break;
            }
        }

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
            case DrawPassCommands::Type::kBindStaticDataBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindStaticDataBuffer*>(cmdPtr);
                this->bindInputBuffer(bdb->fStaticData.fBuffer, bdb->fStaticData.fOffset,
                                      MtlGraphicsPipeline::kStaticDataBufferIndex);
                break;
            }
            case DrawPassCommands::Type::kBindAppendDataBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindAppendDataBuffer*>(cmdPtr);
                this->bindInputBuffer(bdb->fAppendData.fBuffer, bdb->fAppendData.fOffset,
                                      MtlGraphicsPipeline::kAppendDataBufferIndex);
                break;
            }
            case DrawPassCommands::Type::kBindIndexBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindIndexBuffer*>(cmdPtr);
                this->bindIndexBuffer(
                        bdb->fIndices.fBuffer, bdb->fIndices.fOffset);
                break;
            }
            case DrawPassCommands::Type::kBindIndirectBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindIndirectBuffer*>(cmdPtr);
                this->bindIndirectBuffer(
                        bdb->fIndirect.fBuffer, bdb->fIndirect.fOffset);
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
            case DrawPassCommands::Type::kSetScissor: {
                auto ss = static_cast<DrawPassCommands::SetScissor*>(cmdPtr);
                this->setScissor(ss->fScissor);
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
            case DrawPassCommands::Type::kDrawIndirect: {
                auto draw = static_cast<DrawPassCommands::DrawIndirect*>(cmdPtr);
                this->drawIndirect(draw->fType);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedIndirect: {
                auto draw = static_cast<DrawPassCommands::DrawIndexedIndirect*>(cmdPtr);
                this->drawIndexedIndirect(draw->fType);
                break;
            }
            case DrawPassCommands::Type::kAddBarrier: {
                SKGPU_LOG_E("MtlCommandBuffer does not support the addition of barriers.");
                break;
            }
        }
    }
}

MtlBlitCommandEncoder* MtlCommandBuffer::getBlitCommandEncoder() {
    if (fActiveBlitCommandEncoder) {
        return fActiveBlitCommandEncoder.get();
    }

    fActiveBlitCommandEncoder = MtlBlitCommandEncoder::Make(fSharedContext, fCommandBuffer.get());

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

    if (graphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy) {
        // The last texture binding is reserved for the dstCopy texture, which is not included in
        // the list on each BindTexturesAndSamplers command. We can set it once now and any
        // subsequent BindTexturesAndSamplers commands in a DrawPass will set the other N-1.
        SkASSERT(fDstCopy.first && fDstCopy.second);
        const int textureIndex = graphicsPipeline->numFragTexturesAndSamplers() - 1;
        this->bindTextureAndSampler(fDstCopy.first, fDstCopy.second, textureIndex);
    }
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
        case UniformSlot::kGradient:
            bufferIndex = MtlGraphicsPipeline::kGradientBufferIndex;
            break;
    }

    fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, info.fOffset, bufferIndex);
    fActiveRenderCommandEncoder->setFragmentBuffer(mtlBuffer, info.fOffset, bufferIndex);
}

void MtlCommandBuffer::bindInputBuffer(const Buffer* buffer, size_t offset, uint32_t bindingIndex) {
    SkASSERT(fActiveRenderCommandEncoder);
    if (buffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(buffer)->mtlBuffer();
        SkASSERT((offset & 0b11) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, offset, bindingIndex);
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

void MtlCommandBuffer::bindIndirectBuffer(const Buffer* indirectBuffer, size_t offset) {
    if (indirectBuffer) {
        fCurrentIndirectBuffer = static_cast<const MtlBuffer*>(indirectBuffer)->mtlBuffer();
        fCurrentIndirectBufferOffset = offset;
    } else {
        fCurrentIndirectBuffer = nil;
        fCurrentIndirectBufferOffset = 0;
    }
}

void MtlCommandBuffer::bindTextureAndSampler(const Texture* texture,
                                             const Sampler* sampler,
                                             unsigned int bindIndex) {
    SkASSERT(texture && sampler);
    SkASSERT(fActiveRenderCommandEncoder);

    id<MTLTexture> mtlTexture = ((const MtlTexture*)texture)->mtlTexture();
    id<MTLSamplerState> mtlSamplerState = ((const MtlSampler*)sampler)->mtlSamplerState();
    fActiveRenderCommandEncoder->setFragmentTexture(mtlTexture, bindIndex);
    fActiveRenderCommandEncoder->setFragmentSamplerState(mtlSamplerState, bindIndex);
}

void MtlCommandBuffer::setScissor(const Scissor& scissor) {
    SkASSERT(fActiveRenderCommandEncoder);

    SkIRect rect = scissor.getRect(fReplayTranslation, fRenderPassBounds);
    fDrawIsOffscreen = rect.isEmpty();

    fActiveRenderCommandEncoder->setScissorRect({
            static_cast<unsigned int>(rect.x()),
            static_cast<unsigned int>(rect.y()),
            static_cast<unsigned int>(rect.width()),
            static_cast<unsigned int>(rect.height()),
    });
}

void MtlCommandBuffer::setViewport(float x, float y, float width, float height,
                                   float minDepth, float maxDepth) {
    SkASSERT(fActiveRenderCommandEncoder);
    MTLViewport viewport = {x,
                            y,
                            width,
                            height,
                            minDepth,
                            maxDepth};
    fActiveRenderCommandEncoder->setViewport(viewport);
}

void MtlCommandBuffer::updateIntrinsicUniforms(SkIRect viewport) {
    UniformManager intrinsicValues{Layout::kMetal};
    CollectIntrinsicUniforms(fSharedContext->caps(), viewport, fDstReadBounds, &intrinsicValues);
    SkSpan<const char> bytes = intrinsicValues.finish();
    fActiveRenderCommandEncoder->setVertexBytes(
            bytes.data(), bytes.size_bytes(), MtlGraphicsPipeline::kIntrinsicUniformBufferIndex);
    fActiveRenderCommandEncoder->setFragmentBytes(
            bytes.data(), bytes.size_bytes(), MtlGraphicsPipeline::kIntrinsicUniformBufferIndex);
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

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        auto mtlPrimitiveType = graphite_to_mtl_primitive(type);
        size_t indexOffset =  fCurrentIndexBufferOffset + sizeof(uint16_t )* baseIndex;
        // Use the "instance" variant witha count of 1 so that we can pass in a base vertex
        // instead of rebinding a vertex buffer offset.
        fActiveRenderCommandEncoder->drawIndexedPrimitives(mtlPrimitiveType, indexCount,
                                                           MTLIndexTypeUInt16, fCurrentIndexBuffer,
                                                           indexOffset, 1, baseVertex, 0);

    } else {
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

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        auto mtlPrimitiveType = graphite_to_mtl_primitive(type);
        size_t indexOffset =  fCurrentIndexBufferOffset + sizeof(uint16_t) * baseIndex;
        fActiveRenderCommandEncoder->drawIndexedPrimitives(mtlPrimitiveType, indexCount,
                                                           MTLIndexTypeUInt16, fCurrentIndexBuffer,
                                                           indexOffset, instanceCount,
                                                           baseVertex, baseInstance);
    } else {
        SKGPU_LOG_E("Skipping unsupported draw call.");
    }
}

void MtlCommandBuffer::drawIndirect(PrimitiveType type) {
    SkASSERT(fActiveRenderCommandEncoder);
    SkASSERT(fCurrentIndirectBuffer);

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        auto mtlPrimitiveType = graphite_to_mtl_primitive(type);
        fActiveRenderCommandEncoder->drawPrimitives(
                mtlPrimitiveType, fCurrentIndirectBuffer, fCurrentIndirectBufferOffset);
    } else {
        SKGPU_LOG_E("Skipping unsupported draw call.");
    }
}

void MtlCommandBuffer::drawIndexedIndirect(PrimitiveType type) {
    SkASSERT(fActiveRenderCommandEncoder);
    SkASSERT(fCurrentIndirectBuffer);

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        auto mtlPrimitiveType = graphite_to_mtl_primitive(type);
        fActiveRenderCommandEncoder->drawIndexedPrimitives(mtlPrimitiveType,
                                                           MTLIndexTypeUInt32,
                                                           fCurrentIndexBuffer,
                                                           fCurrentIndexBufferOffset,
                                                           fCurrentIndirectBuffer,
                                                           fCurrentIndirectBufferOffset);
    } else {
        SKGPU_LOG_E("Skipping unsupported draw call.");
    }
}

void MtlCommandBuffer::beginComputePass() {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);
    this->endBlitCommandEncoder();
    fActiveComputeCommandEncoder = MtlComputeCommandEncoder::Make(fSharedContext,
                                                                  fCommandBuffer.get());
}

void MtlCommandBuffer::bindComputePipeline(const ComputePipeline* computePipeline) {
    SkASSERT(fActiveComputeCommandEncoder);

    auto mtlPipeline = static_cast<const MtlComputePipeline*>(computePipeline);
    fActiveComputeCommandEncoder->setComputePipelineState(mtlPipeline->mtlPipelineState());
}

void MtlCommandBuffer::bindBuffer(const Buffer* buffer, unsigned int offset, unsigned int index) {
    SkASSERT(fActiveComputeCommandEncoder);

    id<MTLBuffer> mtlBuffer = buffer ? static_cast<const MtlBuffer*>(buffer)->mtlBuffer() : nil;
    fActiveComputeCommandEncoder->setBuffer(mtlBuffer, offset, index);
}

void MtlCommandBuffer::bindTexture(const Texture* texture, unsigned int index) {
    SkASSERT(fActiveComputeCommandEncoder);

    id<MTLTexture> mtlTexture =
            texture ? static_cast<const MtlTexture*>(texture)->mtlTexture() : nil;
    fActiveComputeCommandEncoder->setTexture(mtlTexture, index);
}

void MtlCommandBuffer::bindSampler(const Sampler* sampler, unsigned int index) {
    SkASSERT(fActiveComputeCommandEncoder);

    id<MTLSamplerState> mtlSamplerState =
            sampler ? static_cast<const MtlSampler*>(sampler)->mtlSamplerState() : nil;
    fActiveComputeCommandEncoder->setSamplerState(mtlSamplerState, index);
}

void MtlCommandBuffer::dispatchThreadgroups(const WorkgroupSize& globalSize,
                                            const WorkgroupSize& localSize) {
    SkASSERT(fActiveComputeCommandEncoder);
    fActiveComputeCommandEncoder->dispatchThreadgroups(globalSize, localSize);
}

void MtlCommandBuffer::dispatchThreadgroupsIndirect(const WorkgroupSize& localSize,
                                                    const Buffer* indirectBuffer,
                                                    size_t indirectBufferOffset) {
    SkASSERT(fActiveComputeCommandEncoder);

    id<MTLBuffer> mtlIndirectBuffer = static_cast<const MtlBuffer*>(indirectBuffer)->mtlBuffer();
    fActiveComputeCommandEncoder->dispatchThreadgroupsWithIndirectBuffer(
            mtlIndirectBuffer, indirectBufferOffset, localSize);
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

bool MtlCommandBuffer::onCopyBufferToBuffer(const Buffer* srcBuffer,
                                            size_t srcOffset,
                                            const Buffer* dstBuffer,
                                            size_t dstOffset,
                                            size_t size) {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);

    id<MTLBuffer> mtlSrcBuffer = static_cast<const MtlBuffer*>(srcBuffer)->mtlBuffer();
    id<MTLBuffer> mtlDstBuffer = static_cast<const MtlBuffer*>(dstBuffer)->mtlBuffer();

    MtlBlitCommandEncoder* blitCmdEncoder = this->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->pushDebugGroup(@"copyBufferToBuffer");
#endif
    blitCmdEncoder->copyBufferToBuffer(mtlSrcBuffer, srcOffset, mtlDstBuffer, dstOffset, size);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->popDebugGroup();
#endif
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
    blitCmdEncoder->pushDebugGroup(@"copyTextureToBuffer");
#endif
    blitCmdEncoder->copyFromTexture(mtlTexture, srcRect, mtlBuffer, bufferOffset, bufferRowBytes);
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
    blitCmdEncoder->pushDebugGroup(@"copyBufferToTexture");
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

bool MtlCommandBuffer::onCopyTextureToTexture(const Texture* src,
                                              SkIRect srcRect,
                                              const Texture* dst,
                                              SkIPoint dstPoint,
                                              int mipLevel) {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);

    id<MTLTexture> srcMtlTexture = static_cast<const MtlTexture*>(src)->mtlTexture();
    id<MTLTexture> dstMtlTexture = static_cast<const MtlTexture*>(dst)->mtlTexture();

    MtlBlitCommandEncoder* blitCmdEncoder = this->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->pushDebugGroup(@"copyTextureToTexture");
#endif

    blitCmdEncoder->copyTextureToTexture(srcMtlTexture, srcRect, dstMtlTexture, dstPoint, mipLevel);

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->popDebugGroup();
#endif
    return true;
}

bool MtlCommandBuffer::onSynchronizeBufferToCpu(const Buffer* buffer, bool* outDidResultInWork) {
#ifdef SK_BUILD_FOR_MAC
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);

    id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(buffer)->mtlBuffer();
    if ([mtlBuffer storageMode] != MTLStorageModeManaged) {
        *outDidResultInWork = false;
        return true;
    }

    MtlBlitCommandEncoder* blitCmdEncoder = this->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->pushDebugGroup(@"synchronizeToCpu");
#endif
    blitCmdEncoder->synchronizeResource(mtlBuffer);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    blitCmdEncoder->popDebugGroup();
#endif

    *outDidResultInWork = true;
    return true;
#else   // SK_BUILD_FOR_MAC
    // Explicit synchronization is never necessary on builds that are not macOS since we never use
    // discrete GPUs with managed mode buffers outside of macOS.
    *outDidResultInWork = false;
    return true;
#endif  // SK_BUILD_FOR_MAC
}

bool MtlCommandBuffer::onClearBuffer(const Buffer* buffer, size_t offset, size_t size) {
    SkASSERT(!fActiveRenderCommandEncoder);
    SkASSERT(!fActiveComputeCommandEncoder);

    MtlBlitCommandEncoder* blitCmdEncoder = this->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }

    id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(buffer)->mtlBuffer();
    blitCmdEncoder->fillBuffer(mtlBuffer, offset, size, 0);

    return true;
}

} // namespace skgpu::graphite
