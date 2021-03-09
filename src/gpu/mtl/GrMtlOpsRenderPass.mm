/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlOpsRenderPass.h"

#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"
#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"
#include "src/gpu/mtl/GrMtlRenderTarget.h"
#include "src/gpu/mtl/GrMtlTexture.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlOpsRenderPass::GrMtlOpsRenderPass(GrMtlGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                                       const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                                       const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu) {
    this->setupRenderPass(colorInfo, stencilInfo);
}

GrMtlOpsRenderPass::~GrMtlOpsRenderPass() {
}

void GrMtlOpsRenderPass::precreateCmdEncoder() {
    // For clears, we may not have an associated draw. So we prepare a cmdEncoder that
    // will be submitted whether there's a draw or not.
    SkDEBUGCODE(id<MTLRenderCommandEncoder> cmdEncoder =)
            fGpu->commandBuffer()->getRenderCommandEncoder(fRenderPassDesc, nullptr, this);
    SkASSERT(nil != cmdEncoder);
}

void GrMtlOpsRenderPass::submit() {
    if (!fRenderTarget) {
        return;
    }
    SkIRect iBounds;
    fBounds.roundOut(&iBounds);
    fGpu->submitIndirectCommandBuffer(fRenderTarget, fOrigin, &iBounds);
    fActiveRenderCmdEncoder = nil;
}

static MTLPrimitiveType gr_to_mtl_primitive(GrPrimitiveType primitiveType) {
    const static MTLPrimitiveType mtlPrimitiveType[] {
        MTLPrimitiveTypeTriangle,
        MTLPrimitiveTypeTriangleStrip,
        MTLPrimitiveTypePoint,
        MTLPrimitiveTypeLine,
        MTLPrimitiveTypeLineStrip
    };
    static_assert((int)GrPrimitiveType::kTriangles == 0);
    static_assert((int)GrPrimitiveType::kTriangleStrip == 1);
    static_assert((int)GrPrimitiveType::kPoints == 2);
    static_assert((int)GrPrimitiveType::kLines == 3);
    static_assert((int)GrPrimitiveType::kLineStrip == 4);

    SkASSERT(primitiveType <= GrPrimitiveType::kLineStrip);
    return mtlPrimitiveType[static_cast<int>(primitiveType)];
}

bool GrMtlOpsRenderPass::onBindPipeline(const GrProgramInfo& programInfo,
                                        const SkRect& drawBounds) {
    const GrMtlCaps& caps = fGpu->mtlCaps();
    GrProgramDesc programDesc = caps.makeDesc(fRenderTarget, programInfo,
                                              GrCaps::ProgramDescOverrideFlags::kNone);
    if (!programDesc.isValid()) {
        return false;
    }

    fActivePipelineState = fGpu->resourceProvider().findOrCreateCompatiblePipelineState(
            programDesc, programInfo);
    if (!fActivePipelineState) {
        return false;
    }

    fActivePipelineState->setData(fRenderTarget, programInfo);
    fCurrentVertexStride = programInfo.primProc().vertexStride();

    if (!fActiveRenderCmdEncoder) {
        fActiveRenderCmdEncoder =
                fGpu->commandBuffer()->getRenderCommandEncoder(fRenderPassDesc, nullptr, this);
    }

    [fActiveRenderCmdEncoder setRenderPipelineState:fActivePipelineState->mtlPipelineState()];
    fActivePipelineState->setDrawState(fActiveRenderCmdEncoder,
                                       programInfo.pipeline().writeSwizzle(),
                                       programInfo.pipeline().getXferProcessor());
    if (this->gpu()->caps()->wireframeMode() || programInfo.pipeline().isWireframe()) {
        [fActiveRenderCmdEncoder setTriangleFillMode:MTLTriangleFillModeLines];
    } else {
        [fActiveRenderCmdEncoder setTriangleFillMode:MTLTriangleFillModeFill];
    }

    if (!programInfo.pipeline().isScissorTestEnabled()) {
        // "Disable" scissor by setting it to the full pipeline bounds.
        GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder,
                                                       fRenderTarget, fOrigin,
                                                       SkIRect::MakeWH(fRenderTarget->width(),
                                                                       fRenderTarget->height()));
    }

    fActivePrimitiveType = gr_to_mtl_primitive(programInfo.primitiveType());
    fBounds.join(drawBounds);
    return true;
}

void GrMtlOpsRenderPass::onSetScissorRect(const SkIRect& scissor) {
    SkASSERT(fActivePipelineState);
    SkASSERT(fActiveRenderCmdEncoder);
    GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder, fRenderTarget,
                                                   fOrigin, scissor);
}

bool GrMtlOpsRenderPass::onBindTextures(const GrPrimitiveProcessor& primProc,
                                        const GrSurfaceProxy* const primProcTextures[],
                                        const GrPipeline& pipeline) {
    SkASSERT(fActivePipelineState);
    SkASSERT(fActiveRenderCmdEncoder);
    fActivePipelineState->setTextures(primProc, pipeline, primProcTextures);
    fActivePipelineState->bindTextures(fActiveRenderCmdEncoder);
    return true;
}

void GrMtlOpsRenderPass::onClear(const GrScissorState& scissor, std::array<float, 4> color) {
    // Partial clears are not supported
    SkASSERT(!scissor.enabled());

    // Ideally we should never end up here since all clears should either be done as draws or
    // load ops in metal. However, if a client inserts a wait op we need to handle it.
    auto colorAttachment = fRenderPassDesc.colorAttachments[0];
    colorAttachment.clearColor = MTLClearColorMake(color[0], color[1], color[2], color[3]);
    colorAttachment.loadAction = MTLLoadActionClear;
    this->precreateCmdEncoder();
    colorAttachment.loadAction = MTLLoadActionLoad;
    fActiveRenderCmdEncoder =
            fGpu->commandBuffer()->getRenderCommandEncoder(fRenderPassDesc, nullptr, this);
}

void GrMtlOpsRenderPass::onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) {
    // Partial clears are not supported
    SkASSERT(!scissor.enabled());

    GrAttachment* sb = fRenderTarget->getStencilAttachment();
    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);
    int stencilBitCount = GrBackendFormatStencilBits(sb->backendFormat());

    // The contract with the callers does not guarantee that we preserve all bits in the stencil
    // during this clear. Thus we will clear the entire stencil to the desired value.
    auto stencilAttachment = fRenderPassDesc.stencilAttachment;
    if (insideStencilMask) {
        stencilAttachment.clearStencil = (1 << (stencilBitCount - 1));
    } else {
        stencilAttachment.clearStencil = 0;
    }

    stencilAttachment.loadAction = MTLLoadActionClear;
    this->precreateCmdEncoder();
    stencilAttachment.loadAction = MTLLoadActionLoad;
    fActiveRenderCmdEncoder =
            fGpu->commandBuffer()->getRenderCommandEncoder(fRenderPassDesc, nullptr, this);
}

void GrMtlOpsRenderPass::inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) {
    // TODO: this could be more efficient
    state->doUpload(upload);
    // doUpload() creates a blitCommandEncoder, so we need to recreate a renderCommandEncoder
    fActiveRenderCmdEncoder =
            fGpu->commandBuffer()->getRenderCommandEncoder(fRenderPassDesc, nullptr, this);
}

void GrMtlOpsRenderPass::initRenderState(id<MTLRenderCommandEncoder> encoder) {
    [encoder pushDebugGroup:@"initRenderState"];
    [encoder setFrontFacingWinding:MTLWindingCounterClockwise];
    // Strictly speaking we shouldn't have to set this, as the default viewport is the size of
    // the drawable used to generate the renderCommandEncoder -- but just in case.
    MTLViewport viewport = { 0.0, 0.0,
                             (double) fRenderTarget->width(), (double) fRenderTarget->height(),
                             0.0, 1.0 };
    [encoder setViewport:viewport];
    this->resetBufferBindings();
    [encoder popDebugGroup];
}

void GrMtlOpsRenderPass::setupRenderPass(
        const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
        const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo) {
    const static MTLLoadAction mtlLoadAction[] {
        MTLLoadActionLoad,
        MTLLoadActionClear,
        MTLLoadActionDontCare
    };
    static_assert((int)GrLoadOp::kLoad == 0);
    static_assert((int)GrLoadOp::kClear == 1);
    static_assert((int)GrLoadOp::kDiscard == 2);
    SkASSERT(colorInfo.fLoadOp <= GrLoadOp::kDiscard);
    SkASSERT(stencilInfo.fLoadOp <= GrLoadOp::kDiscard);

    const static MTLStoreAction mtlStoreAction[] {
        MTLStoreActionStore,
        MTLStoreActionDontCare
    };
    static_assert((int)GrStoreOp::kStore == 0);
    static_assert((int)GrStoreOp::kDiscard == 1);
    SkASSERT(colorInfo.fStoreOp <= GrStoreOp::kDiscard);
    SkASSERT(stencilInfo.fStoreOp <= GrStoreOp::kDiscard);

    fRenderPassDesc = [MTLRenderPassDescriptor new];
    auto colorAttachment = fRenderPassDesc.colorAttachments[0];
    colorAttachment.texture =
            static_cast<GrMtlRenderTarget*>(fRenderTarget)->mtlColorTexture();
    const std::array<float, 4>& clearColor = colorInfo.fClearColor;
    colorAttachment.clearColor =
            MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    colorAttachment.loadAction = mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
    colorAttachment.storeAction = mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];

    auto* stencil = static_cast<GrMtlAttachment*>(fRenderTarget->getStencilAttachment());
    auto mtlStencil = fRenderPassDesc.stencilAttachment;
    if (stencil) {
        mtlStencil.texture = stencil->view();
    }
    mtlStencil.clearStencil = 0;
    mtlStencil.loadAction = mtlLoadAction[static_cast<int>(stencilInfo.fLoadOp)];
    mtlStencil.storeAction = mtlStoreAction[static_cast<int>(stencilInfo.fStoreOp)];

    // Manage initial clears
    if (colorInfo.fLoadOp == GrLoadOp::kClear || stencilInfo.fLoadOp == GrLoadOp::kClear)  {
        fBounds = SkRect::MakeWH(fRenderTarget->width(),
                                 fRenderTarget->height());
        this->precreateCmdEncoder();
        if (colorInfo.fLoadOp == GrLoadOp::kClear) {
            colorAttachment.loadAction = MTLLoadActionLoad;
        }
        if (stencilInfo.fLoadOp == GrLoadOp::kClear) {
            mtlStencil.loadAction = MTLLoadActionLoad;
        }
    } else {
        fBounds.setEmpty();
    }

    // For now, we lazily create the renderCommandEncoder because we may have no draws,
    // and an empty renderCommandEncoder can still produce output. This can cause issues
    // when we've cleared a texture upon creation -- we'll subsequently discard the contents.
    // This can be removed when that ordering is fixed.
    fActiveRenderCmdEncoder = nil;
}

void GrMtlOpsRenderPass::onBindBuffers(sk_sp<const GrBuffer> indexBuffer,
                                       sk_sp<const GrBuffer> instanceBuffer,
                                       sk_sp<const GrBuffer> vertexBuffer,
                                       GrPrimitiveRestart primRestart) {
    SkASSERT(GrPrimitiveRestart::kNo == primRestart);
    int inputBufferIndex = 0;
    if (vertexBuffer) {
        SkASSERT(!vertexBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(vertexBuffer.get())->isMapped());
        fActiveVertexBuffer = std::move(vertexBuffer);
        fGpu->commandBuffer()->addGrBuffer(fActiveVertexBuffer);
        ++inputBufferIndex;
    }
    if (instanceBuffer) {
        SkASSERT(!instanceBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(instanceBuffer.get())->isMapped());
        this->setVertexBuffer(fActiveRenderCmdEncoder, instanceBuffer.get(), 0, inputBufferIndex++);
        fActiveInstanceBuffer = std::move(instanceBuffer);
        fGpu->commandBuffer()->addGrBuffer(fActiveInstanceBuffer);
    }
    if (indexBuffer) {
        SkASSERT(!indexBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(indexBuffer.get())->isMapped());
        fActiveIndexBuffer = std::move(indexBuffer);
        fGpu->commandBuffer()->addGrBuffer(fActiveIndexBuffer);
    }
}

void GrMtlOpsRenderPass::onDraw(int vertexCount, int baseVertex) {
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    [fActiveRenderCmdEncoder drawPrimitives:fActivePrimitiveType
                                vertexStart:baseVertex
                                vertexCount:vertexCount];
    fGpu->stats()->incNumDraws();
}

void GrMtlOpsRenderPass::onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                                       uint16_t maxIndexValue, int baseVertex) {
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    SkASSERT(fActiveIndexBuffer);
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(),
                          fCurrentVertexStride * baseVertex, 0);

    auto mtlIndexBuffer = static_cast<const GrMtlBuffer*>(fActiveIndexBuffer.get());
    size_t indexOffset = mtlIndexBuffer->offset() + sizeof(uint16_t) * baseIndex;
    [fActiveRenderCmdEncoder drawIndexedPrimitives:fActivePrimitiveType
                                        indexCount:indexCount
                                         indexType:MTLIndexTypeUInt16
                                       indexBuffer:mtlIndexBuffer->mtlBuffer()
                                 indexBufferOffset:indexOffset];
    fGpu->stats()->incNumDraws();
}

void GrMtlOpsRenderPass::onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                                         int baseVertex) {
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    if (@available(macOS 10.11, iOS 9.0, *)) {
        [fActiveRenderCmdEncoder drawPrimitives:fActivePrimitiveType
                                    vertexStart:baseVertex
                                    vertexCount:vertexCount
                                  instanceCount:instanceCount
                                   baseInstance:baseInstance];
    } else {
        SkASSERT(false);
    }
    fGpu->stats()->incNumDraws();
}

void GrMtlOpsRenderPass::onDrawIndexedInstanced(
        int indexCount, int baseIndex, int instanceCount, int baseInstance, int baseVertex) {
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    SkASSERT(fActiveIndexBuffer);
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    auto mtlIndexBuffer = static_cast<const GrMtlBuffer*>(fActiveIndexBuffer.get());
    size_t indexOffset = mtlIndexBuffer->offset() + sizeof(uint16_t) * baseIndex;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        [fActiveRenderCmdEncoder drawIndexedPrimitives:fActivePrimitiveType
                                            indexCount:indexCount
                                             indexType:MTLIndexTypeUInt16
                                           indexBuffer:mtlIndexBuffer->mtlBuffer()
                                     indexBufferOffset:indexOffset
                                         instanceCount:instanceCount
                                            baseVertex:baseVertex
                                          baseInstance:baseInstance];
    } else {
        SkASSERT(false);
    }
    fGpu->stats()->incNumDraws();
}

void GrMtlOpsRenderPass::onDrawIndirect(const GrBuffer* drawIndirectBuffer,
                                        size_t bufferOffset,
                                        int drawCount) {
    SkASSERT(fGpu->caps()->nativeDrawIndirectSupport());
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    auto mtlIndirectBuffer = static_cast<const GrMtlBuffer*>(drawIndirectBuffer);
    const size_t stride = sizeof(GrDrawIndirectCommand);
    while (drawCount >= 1) {
        if (@available(macOS 10.11, iOS 9.0, *)) {
            [fActiveRenderCmdEncoder drawPrimitives:fActivePrimitiveType
                                     indirectBuffer:mtlIndirectBuffer->mtlBuffer()
                               indirectBufferOffset:bufferOffset];
        } else {
            SkASSERT(false);
        }
        drawCount--;
        bufferOffset += stride;
        fGpu->stats()->incNumDraws();
    }
}

void GrMtlOpsRenderPass::onDrawIndexedIndirect(const GrBuffer* drawIndirectBuffer,
                                               size_t bufferOffset,
                                               int drawCount) {
    SkASSERT(fGpu->caps()->nativeDrawIndirectSupport());
    SkASSERT(fActivePipelineState);
    SkASSERT(nil != fActiveRenderCmdEncoder);
    SkASSERT(fActiveIndexBuffer);
    this->setVertexBuffer(fActiveRenderCmdEncoder, fActiveVertexBuffer.get(), 0, 0);

    auto mtlIndexBuffer = static_cast<const GrMtlBuffer*>(fActiveIndexBuffer.get());
    auto mtlIndirectBuffer = static_cast<const GrMtlBuffer*>(drawIndirectBuffer);
    size_t indexOffset = mtlIndexBuffer->offset();

    const size_t stride = sizeof(GrDrawIndexedIndirectCommand);
    while (drawCount >= 1) {
        if (@available(macOS 10.11, iOS 9.0, *)) {
            [fActiveRenderCmdEncoder drawIndexedPrimitives:fActivePrimitiveType
                                                 indexType:MTLIndexTypeUInt16
                                               indexBuffer:mtlIndexBuffer->mtlBuffer()
                                         indexBufferOffset:indexOffset
                                            indirectBuffer:mtlIndirectBuffer->mtlBuffer()
                                      indirectBufferOffset:bufferOffset];
        } else {
            SkASSERT(false);
        }
        drawCount--;
        bufferOffset += stride;
        fGpu->stats()->incNumDraws();
    }
}

void GrMtlOpsRenderPass::setVertexBuffer(id<MTLRenderCommandEncoder> encoder,
                                         const GrBuffer* buffer,
                                         size_t vertexOffset,
                                         size_t inputBufferIndex) {
    constexpr static int kFirstBufferBindingIdx = GrMtlUniformHandler::kLastUniformBinding + 1;
    int index = inputBufferIndex + kFirstBufferBindingIdx;
    SkASSERT(index < 4);
    auto mtlBuffer = static_cast<const GrMtlBuffer*>(buffer);
    id<MTLBuffer> mtlVertexBuffer = mtlBuffer->mtlBuffer();
    SkASSERT(mtlVertexBuffer);
    // Apple recommends using setVertexBufferOffset: when changing the offset
    // for a currently bound vertex buffer, rather than setVertexBuffer:
    size_t offset = mtlBuffer->offset() + vertexOffset;
    if (fBufferBindings[index].fBuffer != mtlVertexBuffer) {
        [encoder setVertexBuffer: mtlVertexBuffer
                          offset: offset
                         atIndex: index];
        fBufferBindings[index].fBuffer = mtlVertexBuffer;
        fBufferBindings[index].fOffset = offset;
    } else if (fBufferBindings[index].fOffset != offset) {
        if (@available(macOS 10.11, iOS 8.3, *)) {
            [encoder setVertexBufferOffset: offset
                                   atIndex: index];
        } else {
            // We only support iOS 9.0+, so we should never hit this
            SK_ABORT("Missing interface. Skia only supports Metal on iOS 9.0 and higher");
        }
        fBufferBindings[index].fOffset = offset;
    }
}

void GrMtlOpsRenderPass::resetBufferBindings() {
    for (size_t i = 0; i < kNumBindings; ++i) {
        fBufferBindings[i].fBuffer = nil;
    }
}
