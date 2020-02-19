/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlOpsRenderPass.h"

#include "src/gpu/GrColor.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrRenderTargetPriv.h"
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

bool GrMtlOpsRenderPass::onBindPipeline(const GrProgramInfo& programInfo,
                                        const SkRect& drawBounds) {
    fActivePipelineState = fGpu->resourceProvider().findOrCreateCompatiblePipelineState(
            fRenderTarget, programInfo);
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
                                       programInfo.pipeline().outputSwizzle(),
                                       programInfo.pipeline().getXferProcessor());
    if (this->gpu()->caps()->wireframeMode() || programInfo.pipeline().isWireframe()) {
        [fActiveRenderCmdEncoder setTriangleFillMode:MTLTriangleFillModeLines];
    } else {
        [fActiveRenderCmdEncoder setTriangleFillMode:MTLTriangleFillModeFill];
    }

    if (!programInfo.pipeline().isScissorEnabled()) {
        GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder,
                                                       fRenderTarget, fOrigin,
                                                       SkIRect::MakeWH(fRenderTarget->width(),
                                                                       fRenderTarget->height()));
    } else if (!programInfo.hasDynamicScissors()) {
        SkASSERT(programInfo.hasFixedScissor());

        GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder,
                                                       fRenderTarget, fOrigin,
                                                       programInfo.fixedScissor());
    }

    if (!programInfo.hasDynamicPrimProcTextures()) {
        fActivePipelineState->bindTextures(fActiveRenderCmdEncoder);
    }

    fBounds.join(drawBounds);
    return true;
}

void GrMtlOpsRenderPass::onDrawMeshes(const GrProgramInfo& programInfo, const GrMesh meshes[],
                                      int meshCount) {
    SkASSERT(fActivePipelineState);

    for (int i = 0; i < meshCount; ++i) {
        const GrMesh& mesh = meshes[i];
        SkASSERT(nil != fActiveRenderCmdEncoder);

        if (programInfo.hasDynamicScissors()) {
            GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder, fRenderTarget,
                                                           fOrigin,
                                                           programInfo.dynamicScissor(i));
        }
        if (programInfo.hasDynamicPrimProcTextures()) {
            auto meshProxies = programInfo.dynamicPrimProcTextures(i);
            fActivePipelineState->setTextures(programInfo, meshProxies);
            fActivePipelineState->bindTextures(fActiveRenderCmdEncoder);
        }

        mesh.sendToGpu(programInfo.primitiveType(), this);
    }
}

void GrMtlOpsRenderPass::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    // We should never end up here since all clears should either be done as draws or load ops in
    // metal. If we hit this assert then we missed a chance to set a load op on the
    // GrRenderTargetContext level.
    SkASSERT(false);
}

void GrMtlOpsRenderPass::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    SkASSERT(!clip.hasWindowRectangles());

    GrStencilAttachment* sb = fRenderTarget->renderTargetPriv().getStencilAttachment();
    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);
    int stencilBitCount = sb->bits();

    // The contract with the callers does not guarantee that we preserve all bits in the stencil
    // during this clear. Thus we will clear the entire stencil to the desired value.
    if (insideStencilMask) {
        fRenderPassDesc.stencilAttachment.clearStencil = (1 << (stencilBitCount - 1));
    } else {
        fRenderPassDesc.stencilAttachment.clearStencil = 0;
    }

    fRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionClear;
    this->precreateCmdEncoder();
    fRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
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

    auto renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture =
            static_cast<GrMtlRenderTarget*>(fRenderTarget)->mtlColorTexture();
    renderPassDesc.colorAttachments[0].slice = 0;
    renderPassDesc.colorAttachments[0].level = 0;
    const SkPMColor4f& clearColor = colorInfo.fClearColor;
    renderPassDesc.colorAttachments[0].clearColor =
            MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    renderPassDesc.colorAttachments[0].loadAction =
            mtlLoadAction[static_cast<int>(colorInfo.fLoadOp)];
    renderPassDesc.colorAttachments[0].storeAction =
            mtlStoreAction[static_cast<int>(colorInfo.fStoreOp)];

    const GrMtlStencilAttachment* stencil = static_cast<GrMtlStencilAttachment*>(
            fRenderTarget->renderTargetPriv().getStencilAttachment());
    if (stencil) {
        renderPassDesc.stencilAttachment.texture = stencil->stencilView();
    }
    renderPassDesc.stencilAttachment.clearStencil = 0;
    renderPassDesc.stencilAttachment.loadAction =
            mtlLoadAction[static_cast<int>(stencilInfo.fLoadOp)];
    renderPassDesc.stencilAttachment.storeAction =
            mtlStoreAction[static_cast<int>(stencilInfo.fStoreOp)];

    fRenderPassDesc = renderPassDesc;

    // Manage initial clears
    if (colorInfo.fLoadOp == GrLoadOp::kClear || stencilInfo.fLoadOp == GrLoadOp::kClear)  {
        fBounds = SkRect::MakeWH(fRenderTarget->width(),
                                 fRenderTarget->height());
        this->precreateCmdEncoder();
        if (colorInfo.fLoadOp == GrLoadOp::kClear) {
            fRenderPassDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
        }
        if (stencilInfo.fLoadOp == GrLoadOp::kClear) {
            fRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
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

void GrMtlOpsRenderPass::bindGeometry(const GrBuffer* vertexBuffer,
                                      size_t vertexOffset,
                                      const GrBuffer* instanceBuffer) {
    size_t bufferIndex = GrMtlUniformHandler::kLastUniformBinding + 1;
    if (vertexBuffer) {
        SkASSERT(!vertexBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(vertexBuffer)->isMapped());

        const GrMtlBuffer* grMtlBuffer = static_cast<const GrMtlBuffer*>(vertexBuffer);
        this->setVertexBuffer(fActiveRenderCmdEncoder, grMtlBuffer, vertexOffset, bufferIndex++);
    }
    if (instanceBuffer) {
        SkASSERT(!instanceBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(instanceBuffer)->isMapped());

        const GrMtlBuffer* grMtlBuffer = static_cast<const GrMtlBuffer*>(instanceBuffer);
        this->setVertexBuffer(fActiveRenderCmdEncoder, grMtlBuffer, 0, bufferIndex++);
    }
}

void GrMtlOpsRenderPass::sendArrayMeshToGpu(GrPrimitiveType primitiveType, const GrMesh& mesh,
                                            int vertexCount, int baseVertex) {
    this->bindGeometry(mesh.vertexBuffer(), 0, nullptr);

    [fActiveRenderCmdEncoder drawPrimitives:gr_to_mtl_primitive(primitiveType)
                                vertexStart:baseVertex
                                vertexCount:vertexCount];
}

void GrMtlOpsRenderPass::sendIndexedMeshToGpu(GrPrimitiveType primitiveType, const GrMesh& mesh,
                                              int indexCount, int baseIndex,
                                              uint16_t /*minIndexValue*/,
                                              uint16_t /*maxIndexValue*/, int baseVertex) {
    this->bindGeometry(mesh.vertexBuffer(), fCurrentVertexStride*baseVertex, nullptr);

    id<MTLBuffer> mtlIndexBuffer = nil;
    if (mesh.indexBuffer()) {
        SkASSERT(!mesh.indexBuffer()->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(mesh.indexBuffer())->isMapped());

        mtlIndexBuffer = static_cast<const GrMtlBuffer*>(mesh.indexBuffer())->mtlBuffer();
        SkASSERT(mtlIndexBuffer);
    }

    SkASSERT(mesh.primitiveRestart() == GrPrimitiveRestart::kNo);
    size_t indexOffset = static_cast<const GrMtlBuffer*>(mesh.indexBuffer())->offset() +
                         sizeof(uint16_t) * baseIndex;
    [fActiveRenderCmdEncoder drawIndexedPrimitives:gr_to_mtl_primitive(primitiveType)
                                        indexCount:indexCount
                                         indexType:MTLIndexTypeUInt16
                                       indexBuffer:mtlIndexBuffer
                                 indexBufferOffset:indexOffset];
    fGpu->stats()->incNumDraws();
}

void GrMtlOpsRenderPass::sendInstancedMeshToGpu(GrPrimitiveType primitiveType, const GrMesh& mesh,
                                                int vertexCount, int baseVertex, int instanceCount,
                                                int baseInstance) {
    this->bindGeometry(mesh.vertexBuffer(), 0, mesh.instanceBuffer());

    if (@available(macOS 10.11, iOS 9.0, *)) {
        [fActiveRenderCmdEncoder drawPrimitives:gr_to_mtl_primitive(primitiveType)
                                    vertexStart:baseVertex
                                    vertexCount:vertexCount
                                  instanceCount:instanceCount
                                   baseInstance:baseInstance];
    } else {
        SkASSERT(false);
    }
}

void GrMtlOpsRenderPass::sendIndexedInstancedMeshToGpu(GrPrimitiveType primitiveType,
                                                       const GrMesh& mesh, int indexCount,
                                                       int baseIndex, int baseVertex,
                                                       int instanceCount, int baseInstance) {
    this->bindGeometry(mesh.vertexBuffer(), 0, mesh.instanceBuffer());

    id<MTLBuffer> mtlIndexBuffer = nil;
    if (mesh.indexBuffer()) {
        SkASSERT(!mesh.indexBuffer()->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(mesh.indexBuffer())->isMapped());

        mtlIndexBuffer = static_cast<const GrMtlBuffer*>(mesh.indexBuffer())->mtlBuffer();
        SkASSERT(mtlIndexBuffer);
    }

    SkASSERT(mesh.primitiveRestart() == GrPrimitiveRestart::kNo);
    size_t indexOffset = static_cast<const GrMtlBuffer*>(mesh.indexBuffer())->offset() +
                         sizeof(uint16_t) * baseIndex;

    if (@available(macOS 10.11, iOS 9.0, *)) {
        [fActiveRenderCmdEncoder drawIndexedPrimitives:gr_to_mtl_primitive(primitiveType)
                                            indexCount:indexCount
                                             indexType:MTLIndexTypeUInt16
                                           indexBuffer:mtlIndexBuffer
                                     indexBufferOffset:indexOffset
                                         instanceCount:instanceCount
                                            baseVertex:baseVertex
                                          baseInstance:baseInstance];
    } else {
        SkASSERT(false);
    }
    fGpu->stats()->incNumDraws();
}

void GrMtlOpsRenderPass::setVertexBuffer(id<MTLRenderCommandEncoder> encoder,
                                         const GrMtlBuffer* buffer,
                                         size_t vertexOffset,
                                         size_t index) {
    SkASSERT(index < 4);
    id<MTLBuffer> mtlVertexBuffer = buffer->mtlBuffer();
    SkASSERT(mtlVertexBuffer);
    // Apple recommends using setVertexBufferOffset: when changing the offset
    // for a currently bound vertex buffer, rather than setVertexBuffer:
    size_t offset = buffer->offset() + vertexOffset;
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
            [encoder setVertexBuffer: mtlVertexBuffer
                              offset: offset
                             atIndex: index];
        }
        fBufferBindings[index].fOffset = offset;
    }
}

void GrMtlOpsRenderPass::resetBufferBindings() {
    for (size_t i = 0; i < kNumBindings; ++i) {
        fBufferBindings[i].fBuffer = nil;
    }
}
