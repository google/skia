/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlGpuCommandBuffer.h"

#include "include/private/GrColor.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"
#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"
#include "src/gpu/mtl/GrMtlRenderTarget.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlGpuRTCommandBuffer::GrMtlGpuRTCommandBuffer(
        GrMtlGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin, const SkRect& bounds,
        const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
        const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu)
#ifdef SK_DEBUG
        , fBounds(bounds)
#endif
        , fColorLoadAndStoreInfo(colorInfo)
        , fStencilLoadAndStoreInfo(stencilInfo)
        , fRenderPassDesc(this->createRenderPassDesc()) {
    (void)fStencilLoadAndStoreInfo; // Silence unused var warning
    const GrMtlStencilAttachment* stencil = static_cast<GrMtlStencilAttachment*>(
                                                     rt->renderTargetPriv().getStencilAttachment());
    if (stencil) {
        fRenderPassDesc.stencilAttachment.texture = stencil->stencilView();
    }
    if (fColorLoadAndStoreInfo.fLoadOp == GrLoadOp::kClear) {
        fCommandBufferInfo.fBounds = SkRect::MakeWH(fRenderTarget->width(),
                                                    fRenderTarget->height());
        this->addNullCommand();
        fRenderPassDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
    } else {
        fCommandBufferInfo.fBounds.setEmpty();
    }
    switch (stencilInfo.fLoadOp) {
        case GrLoadOp::kLoad:
            fRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
            break;
        case GrLoadOp::kClear:
            fCommandBufferInfo.fBounds = SkRect::MakeWH(fRenderTarget->width(),
                                                        fRenderTarget->height());
            fRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionClear;
            this->addNullCommand();
            fRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
            break;
        case GrLoadOp::kDiscard:
            fRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionDontCare;
            break;
    }
    switch (stencilInfo.fStoreOp) {
        case GrStoreOp::kStore:
            fRenderPassDesc.stencilAttachment.storeAction = MTLStoreActionStore;
            break;
        case GrStoreOp::kDiscard:
            fRenderPassDesc.stencilAttachment.storeAction = MTLStoreActionDontCare;
            break;
    }
}

GrMtlGpuRTCommandBuffer::~GrMtlGpuRTCommandBuffer() {
    SkASSERT(nil == fActiveRenderCmdEncoder);
}

void GrMtlGpuRTCommandBuffer::addNullCommand() {
    SkASSERT(nil == fActiveRenderCmdEncoder);

    SkDEBUGCODE(id<MTLRenderCommandEncoder> cmdEncoder =)
            fGpu->commandBuffer()->getRenderCommandEncoder(fRenderPassDesc, nullptr);
    SkASSERT(nil != cmdEncoder);
}

void GrMtlGpuRTCommandBuffer::submit() {
    if (!fRenderTarget) {
        return;
    }
    SkIRect iBounds;
    fCommandBufferInfo.fBounds.roundOut(&iBounds);
    fGpu->submitIndirectCommandBuffer(fRenderTarget, fOrigin, &iBounds);
}

void GrMtlGpuRTCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin,
                                   const SkIRect& srcRect, const SkIPoint& dstPoint) {
    // We cannot have an active encoder when we call copy since it requires its own
    // command encoder.
    SkASSERT(nil == fActiveRenderCmdEncoder);
    fGpu->copySurface(fRenderTarget, fOrigin, src, srcOrigin, srcRect, dstPoint);
}

void GrMtlGpuRTCommandBuffer::transferFrom(const SkIRect& srcRect, GrColorType bufferColorType,
                                           GrGpuBuffer* transferBuffer, size_t offset) {
    // We cannot have an active encoder when we call transferFrom since it requires its own
    // command encoder.
    SkASSERT(nil == fActiveRenderCmdEncoder);
    fGpu->transferPixelsFrom(fRenderTarget, srcRect.fLeft, srcRect.fTop, srcRect.width(),
                             srcRect.height(), bufferColorType, transferBuffer, offset);
}

GrMtlPipelineState* GrMtlGpuRTCommandBuffer::prepareDrawState(
        const GrPrimitiveProcessor& primProc,
        const GrPipeline& pipeline,
        const GrPipeline::FixedDynamicState* fixedDynamicState,
        GrPrimitiveType primType) {
    // TODO: resolve textures and regenerate mipmaps as needed

    const GrTextureProxy* const* primProcProxies = nullptr;
    if (fixedDynamicState) {
        primProcProxies = fixedDynamicState->fPrimitiveProcessorTextures;
    }
    SkASSERT(SkToBool(primProcProxies) == SkToBool(primProc.numTextureSamplers()));

    GrMtlPipelineState* pipelineState =
        fGpu->resourceProvider().findOrCreateCompatiblePipelineState(fRenderTarget, fOrigin,
                                                                     pipeline,
                                                                     primProc,
                                                                     primProcProxies,
                                                                     primType);
    if (!pipelineState) {
        return nullptr;
    }
    pipelineState->setData(fRenderTarget, fOrigin, primProc, pipeline, primProcProxies);

    return pipelineState;
}

void GrMtlGpuRTCommandBuffer::onDraw(const GrPrimitiveProcessor& primProc,
                                     const GrPipeline& pipeline,
                                     const GrPipeline::FixedDynamicState* fixedDynamicState,
                                     const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                     const GrMesh meshes[],
                                     int meshCount,
                                     const SkRect& bounds) {
    if (!meshCount) {
        return;
    }

    auto prepareSampledImage = [&](GrTexture* texture, GrSamplerState::Filter filter) {
        // Check if we need to regenerate any mip maps
        if (GrSamplerState::Filter::kMipMap == filter &&
            (texture->width() != 1 || texture->height() != 1)) {
            SkASSERT(texture->texturePriv().mipMapped() == GrMipMapped::kYes);
            if (texture->texturePriv().mipMapsAreDirty()) {
                fGpu->regenerateMipMapLevels(texture);
            }
        }
    };

    if (dynamicStateArrays && dynamicStateArrays->fPrimitiveProcessorTextures) {
        for (int m = 0, i = 0; m < meshCount; ++m) {
            for (int s = 0; s < primProc.numTextureSamplers(); ++s, ++i) {
                auto texture = dynamicStateArrays->fPrimitiveProcessorTextures[i]->peekTexture();
                prepareSampledImage(texture, primProc.textureSampler(s).samplerState().filter());
            }
        }
    } else {
        for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
            auto texture = fixedDynamicState->fPrimitiveProcessorTextures[i]->peekTexture();
            prepareSampledImage(texture, primProc.textureSampler(i).samplerState().filter());
        }
    }
    GrFragmentProcessor::Iter iter(pipeline);
    while (const GrFragmentProcessor* fp = iter.next()) {
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            const GrFragmentProcessor::TextureSampler& sampler = fp->textureSampler(i);
            prepareSampledImage(sampler.peekTexture(), sampler.samplerState().filter());
        }
    }

    GrPrimitiveType primitiveType = meshes[0].primitiveType();
    GrMtlPipelineState* pipelineState = this->prepareDrawState(primProc, pipeline,
                                                               fixedDynamicState, primitiveType);
    if (!pipelineState) {
        return;
    }

    SkASSERT(nil == fActiveRenderCmdEncoder);
    fActiveRenderCmdEncoder = fGpu->commandBuffer()->getRenderCommandEncoder(fRenderPassDesc,
                                                                             pipelineState);
    SkASSERT(fActiveRenderCmdEncoder);
    // TODO: can we set this once somewhere at the beginning of the draw?
    [fActiveRenderCmdEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    // Strictly speaking we shouldn't have to set this, as the default viewport is the size of
    // the drawable used to generate the renderCommandEncoder -- but just in case.
    MTLViewport viewport = { 0.0, 0.0,
                             (double) fRenderTarget->width(), (double) fRenderTarget->height(),
                             0.0, 1.0 };
    [fActiveRenderCmdEncoder setViewport:viewport];

    [fActiveRenderCmdEncoder setRenderPipelineState:pipelineState->mtlPipelineState()];
    pipelineState->setDrawState(fActiveRenderCmdEncoder, fRenderTarget->config(),
                                pipeline.getXferProcessor());

    bool dynamicScissor =
            pipeline.isScissorEnabled() && dynamicStateArrays && dynamicStateArrays->fScissorRects;
    if (!pipeline.isScissorEnabled()) {
        GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder,
                                                       fRenderTarget, fOrigin,
                                                       SkIRect::MakeWH(fRenderTarget->width(),
                                                                       fRenderTarget->height()));
    } else if (!dynamicScissor) {
        SkASSERT(fixedDynamicState);
        GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder,
                                                       fRenderTarget, fOrigin,
                                                       fixedDynamicState->fScissorRect);
    }

    for (int i = 0; i < meshCount; ++i) {
        const GrMesh& mesh = meshes[i];
        SkASSERT(nil != fActiveRenderCmdEncoder);
        if (mesh.primitiveType() != primitiveType) {
            SkDEBUGCODE(pipelineState = nullptr);
            primitiveType = mesh.primitiveType();
            pipelineState = this->prepareDrawState(primProc, pipeline, fixedDynamicState,
                                                   primitiveType);
            if (!pipelineState) {
                return;
            }

            [fActiveRenderCmdEncoder setRenderPipelineState:pipelineState->mtlPipelineState()];
            pipelineState->setDrawState(fActiveRenderCmdEncoder, fRenderTarget->config(),
                                        pipeline.getXferProcessor());
        }

        if (dynamicScissor) {
            GrMtlPipelineState::SetDynamicScissorRectState(fActiveRenderCmdEncoder, fRenderTarget,
                                                           fOrigin,
                                                           dynamicStateArrays->fScissorRects[i]);
        }

        mesh.sendToGpu(this);
    }

    fActiveRenderCmdEncoder = nil;
    this->resetBufferBindings();
    fCommandBufferInfo.fBounds.join(bounds);
}

void GrMtlGpuRTCommandBuffer::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    // if we end up here from absClear, the clear bounds may be bigger than the RT proxy bounds -
    // but in that case, scissor should be enabled, so this check should still succeed
    SkASSERT(!clip.scissorEnabled() || clip.scissorRect().contains(fBounds));
    fRenderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(color.fR, color.fG, color.fB,
                                                                       color.fA);
    fRenderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    this->addNullCommand();
    fRenderPassDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
}

void GrMtlGpuRTCommandBuffer::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
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
    this->addNullCommand();
    fRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
}

MTLRenderPassDescriptor* GrMtlGpuRTCommandBuffer::createRenderPassDesc() const {
    const static MTLLoadAction mtlLoadAction[] {
        MTLLoadActionLoad,
        MTLLoadActionClear,
        MTLLoadActionDontCare
    };
    GR_STATIC_ASSERT((int)GrLoadOp::kLoad == 0);
    GR_STATIC_ASSERT((int)GrLoadOp::kClear == 1);
    GR_STATIC_ASSERT((int)GrLoadOp::kDiscard == 2);
    SkASSERT(fColorLoadAndStoreInfo.fLoadOp <= GrLoadOp::kDiscard);

    const static MTLStoreAction mtlStoreAction[] {
        MTLStoreActionStore,
        MTLStoreActionDontCare
    };
    GR_STATIC_ASSERT((int)GrStoreOp::kStore == 0);
    GR_STATIC_ASSERT((int)GrStoreOp::kDiscard == 1);
    SkASSERT(fColorLoadAndStoreInfo.fStoreOp <= GrStoreOp::kDiscard);

    auto renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDesc.colorAttachments[0].texture =
            static_cast<GrMtlRenderTarget*>(fRenderTarget)->mtlRenderTexture();
    renderPassDesc.colorAttachments[0].slice = 0;
    renderPassDesc.colorAttachments[0].level = 0;
    const SkPMColor4f& clearColor = fColorLoadAndStoreInfo.fClearColor;
    renderPassDesc.colorAttachments[0].clearColor =
            MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    renderPassDesc.colorAttachments[0].loadAction =
            mtlLoadAction[static_cast<int>(fColorLoadAndStoreInfo.fLoadOp)];
    renderPassDesc.colorAttachments[0].storeAction =
            mtlStoreAction[static_cast<int>(fColorLoadAndStoreInfo.fStoreOp)];
    return renderPassDesc;
}

static MTLPrimitiveType gr_to_mtl_primitive(GrPrimitiveType primitiveType) {
    const static MTLPrimitiveType mtlPrimitiveType[] {
        MTLPrimitiveTypeTriangle,
        MTLPrimitiveTypeTriangleStrip,
        MTLPrimitiveTypePoint,
        MTLPrimitiveTypeLine,
        MTLPrimitiveTypeLineStrip
    };
    GR_STATIC_ASSERT((int)GrPrimitiveType::kTriangles == 0);
    GR_STATIC_ASSERT((int)GrPrimitiveType::kTriangleStrip == 1);
    GR_STATIC_ASSERT((int)GrPrimitiveType::kPoints == 2);
    GR_STATIC_ASSERT((int)GrPrimitiveType::kLines == 3);
    GR_STATIC_ASSERT((int)GrPrimitiveType::kLineStrip == 4);

    SkASSERT(primitiveType <= GrPrimitiveType::kLineStrip);
    return mtlPrimitiveType[static_cast<int>(primitiveType)];
}

void GrMtlGpuRTCommandBuffer::bindGeometry(const GrBuffer* vertexBuffer,
                                           const GrBuffer* instanceBuffer) {
    size_t bufferIndex = GrMtlUniformHandler::kLastUniformBinding + 1;
    if (vertexBuffer) {
        SkASSERT(!vertexBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(vertexBuffer)->isMapped());

        const GrMtlBuffer* grMtlBuffer = static_cast<const GrMtlBuffer*>(vertexBuffer);
        this->setVertexBuffer(fActiveRenderCmdEncoder, grMtlBuffer, bufferIndex++);
    }
    if (instanceBuffer) {
        SkASSERT(!instanceBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(instanceBuffer)->isMapped());

        const GrMtlBuffer* grMtlBuffer = static_cast<const GrMtlBuffer*>(instanceBuffer);
        this->setVertexBuffer(fActiveRenderCmdEncoder, grMtlBuffer, bufferIndex++);
    }
}

void GrMtlGpuRTCommandBuffer::sendInstancedMeshToGpu(GrPrimitiveType primitiveType,
                                                     const GrBuffer* vertexBuffer,
                                                     int vertexCount,
                                                     int baseVertex,
                                                     const GrBuffer* instanceBuffer,
                                                     int instanceCount,
                                                     int baseInstance) {
    this->bindGeometry(vertexBuffer, instanceBuffer);

    SkASSERT(primitiveType != GrPrimitiveType::kLinesAdjacency); // Geometry shaders not supported.
    [fActiveRenderCmdEncoder drawPrimitives:gr_to_mtl_primitive(primitiveType)
                                vertexStart:baseVertex
                                vertexCount:vertexCount
                              instanceCount:instanceCount
                               baseInstance:baseInstance];
}

void GrMtlGpuRTCommandBuffer::sendIndexedInstancedMeshToGpu(GrPrimitiveType primitiveType,
                                                            const GrBuffer* indexBuffer,
                                                            int indexCount,
                                                            int baseIndex,
                                                            const GrBuffer* vertexBuffer,
                                                            int baseVertex,
                                                            const GrBuffer* instanceBuffer,
                                                            int instanceCount,
                                                            int baseInstance,
                                                            GrPrimitiveRestart restart) {
    this->bindGeometry(vertexBuffer, instanceBuffer);

    SkASSERT(primitiveType != GrPrimitiveType::kLinesAdjacency); // Geometry shaders not supported.
    id<MTLBuffer> mtlIndexBuffer = nil;
    if (indexBuffer) {
        SkASSERT(!indexBuffer->isCpuBuffer());
        SkASSERT(!static_cast<const GrGpuBuffer*>(indexBuffer)->isMapped());

        mtlIndexBuffer = static_cast<const GrMtlBuffer*>(indexBuffer)->mtlBuffer();
        SkASSERT(mtlIndexBuffer);
    }

    SkASSERT(restart == GrPrimitiveRestart::kNo);
    [fActiveRenderCmdEncoder drawIndexedPrimitives:gr_to_mtl_primitive(primitiveType)
                                        indexCount:indexCount
                                         indexType:MTLIndexTypeUInt16
                                       indexBuffer:mtlIndexBuffer
                                 indexBufferOffset:sizeof(uint16_t) * baseIndex
                                     instanceCount:instanceCount
                                        baseVertex:baseVertex
                                      baseInstance:baseInstance];
    fGpu->stats()->incNumDraws();
}

void GrMtlGpuRTCommandBuffer::setVertexBuffer(id<MTLRenderCommandEncoder> encoder,
                                              const GrMtlBuffer* buffer,
                                              size_t index) {
    SkASSERT(index < 4);
    id<MTLBuffer> mtlVertexBuffer = buffer->mtlBuffer();
    SkASSERT(mtlVertexBuffer);
    // Apple recommends using setVertexBufferOffset: when changing the offset
    // for a currently bound vertex buffer, rather than setVertexBuffer:
    if (fBufferBindings[index] != mtlVertexBuffer) {
        [encoder setVertexBuffer: mtlVertexBuffer
                          offset: 0
                         atIndex: index];
        fBufferBindings[index] = mtlVertexBuffer;
    }
    [encoder setVertexBufferOffset: buffer->offset()
                           atIndex: index];
}

void GrMtlGpuRTCommandBuffer::resetBufferBindings() {
    for (size_t i = 0; i < kNumBindings; ++i) {
        fBufferBindings[i] = nil;
    }
}
