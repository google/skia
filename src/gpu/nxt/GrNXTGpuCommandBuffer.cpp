/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTGpuCommandBuffer.h"

#include "GrFixedClip.h"
#include "GrMesh.h"
#include "GrOpFlushState.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrTexturePriv.h"
#include "GrNXTBuffer.h"
#include "GrNXTGpu.h"
#include "GrNXTProgramBuilder.h"
#include "GrNXTStencilAttachment.h"
#include "GrNXTTexture.h"
#include "GrNXTUtil.h"
#include "SkRect.h"
#include "SkSLCompiler.h"

void GrNXTGpuTextureCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin,
                                        const SkIRect& srcRect, const SkIPoint& dstPoint) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpuTextureCommandBuffer::insertEventMarker(const char* msg) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpuTextureCommandBuffer::submit() {
    for (int i = 0; i < fCopies.count(); ++i) {
        CopyInfo& copyInfo = fCopies[i];
        fGpu->copySurface(fTexture, fOrigin, copyInfo.fSrc, copyInfo.fSrcOrigin, copyInfo.fSrcRect,
                          copyInfo.fDstPoint);
    }
}

GrNXTGpuTextureCommandBuffer::~GrNXTGpuTextureCommandBuffer() {}

namespace {

nxt::LoadOp to_nxt_load_op(GrLoadOp loadOp) {
    switch (loadOp) {
        case GrLoadOp::kLoad:
            return nxt::LoadOp::Load;
        case GrLoadOp::kDiscard:
            return nxt::LoadOp::DontCare;
        case GrLoadOp::kClear:
            return nxt::LoadOp::Clear;
        default:
            SK_ABORT("Invalid LoadOp");
            return nxt::LoadOp::Load;
    }
}

}

GrNXTGpuRTCommandBuffer::GrNXTGpuRTCommandBuffer(GrNXTGpu* gpu,
                                                 GrRenderTarget* rt, GrSurfaceOrigin origin,
                                                 const LoadAndStoreInfo& colorInfo,
                                                 const StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu)
        , fColorInfo(colorInfo)
        , fStencilInfo(stencilInfo) {
    fBuilder = fGpu->device().CreateCommandBufferBuilder();
    nxt::LoadOp colorOp = to_nxt_load_op(colorInfo.fLoadOp);
    nxt::LoadOp stencilOp = to_nxt_load_op(stencilInfo.fLoadOp);
    fBuilder.BeginRenderPass(createRenderPassDescriptor(colorOp, stencilOp));
}

GrNXTGpuRTCommandBuffer::~GrNXTGpuRTCommandBuffer() {
}

GrGpu* GrNXTGpuRTCommandBuffer::gpu() { return fGpu; }

nxt::RenderPassDescriptor GrNXTGpuRTCommandBuffer::createRenderPassDescriptor(
        nxt::LoadOp colorOp,
        nxt::LoadOp stencilOp) const {
    GrBackendRenderTarget backendRT = fRenderTarget->getBackendRenderTarget();
    GrNXTImageInfo info;
    backendRT.getNXTImageInfo(&info);
    nxt::Texture texture(info.fTexture);
    auto stencilAttachment = static_cast<GrNXTStencilAttachment*>(
        fRenderTarget->renderTargetPriv().getStencilAttachment());
    nxt::TextureView colorView = texture.CreateTextureViewBuilder().GetResult();
    GrColor4f clearColor4(GrColor4f::FromGrColor(fColorInfo.fClearColor));
    const float *c = clearColor4.fRGBA;

    auto renderPassBuilder = fGpu->device().CreateRenderPassDescriptorBuilder()
        .SetColorAttachment(0, colorView, colorOp)
        .SetColorAttachmentClearColor(0, c[0], c[1], c[2], c[3])
        .Clone();
    if (stencilAttachment) {
        renderPassBuilder
            .SetDepthStencilAttachment(stencilAttachment->view(), stencilOp, stencilOp);
    }
    return renderPassBuilder.GetResult();
}

void GrNXTGpuRTCommandBuffer::end() {
    fBuilder.EndRenderPass();
}

void GrNXTGpuRTCommandBuffer::submit() {
    fGpu->appendCommandBuffer(fBuilder.GetResult());
}

void GrNXTGpuRTCommandBuffer::discard() {
    SkASSERT(!"unimplemented");
}

void GrNXTGpuRTCommandBuffer::insertEventMarker(const char* msg) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpuRTCommandBuffer::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpuRTCommandBuffer::onClear(const GrFixedClip& clip, GrColor color) {
    SkASSERT(!"unimplemented");
}

////////////////////////////////////////////////////////////////////////////////

void GrNXTGpuRTCommandBuffer::inlineUpload(GrOpFlushState* state,
                                           GrDeferredTextureUploadFn& upload) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpuRTCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                                   const SkIPoint& dstPoint) {
    auto s = static_cast<GrNXTTexture*>(src->asTexture());
    auto d = static_cast<GrNXTTexture*>(fRenderTarget->asTexture());

    if (!s || !d) {
        return;
    }

    nxt::Texture srcTex = s->texture();
    nxt::Texture dstTex = d->texture();

    int x = srcRect.x();
    int y = srcRect.y();
    int width = srcRect.width();
    int height = srcRect.height();
    int rowPitch = (width * GrBytesPerPixel(src->config()) + 0xFF) & ~0xFF;
    int sizeInBytes = rowPitch * height;
    nxt::Buffer buffer = fGpu->device().CreateBufferBuilder()
        .SetAllowedUsage(nxt::BufferUsageBit::TransferSrc | nxt::BufferUsageBit::TransferDst)
        .SetSize(sizeInBytes)
        .GetResult();
    int dstX = dstPoint.x();
    int dstY = dstPoint.y();
    fBuilder.EndRenderPass();
    fBuilder.CopyTextureToBuffer(srcTex, x, y, 0, width, height, 1, 0, buffer, 0, rowPitch);
    fBuilder.CopyBufferToTexture(buffer, 0, rowPitch, dstTex, dstX, dstY, 0, width, height, 1, 0);
    fBuilder.BeginRenderPass(createRenderPassDescriptor(nxt::LoadOp::Load, nxt::LoadOp::Load));
}

////////////////////////////////////////////////////////////////////////////////

void GrNXTGpuRTCommandBuffer::setScissorState(
        const GrPipeline& pipeline,
        const GrPipeline::FixedDynamicState* fixedDynamicState,
        const GrPipeline::DynamicStateArrays* dynamicStateArrays) {
    SkIRect rect;
    if (pipeline.isScissorEnabled()) {
        constexpr SkIRect kBogusScissor{0, 0, 1, 1};
        GrScissorState state(fixedDynamicState ? fixedDynamicState->fScissorRect : kBogusScissor);
        rect = state.rect();
        if (kBottomLeft_GrSurfaceOrigin == pipeline.proxy()->origin()) {
            rect.setXYWH(rect.x(), fRenderTarget->height() - rect.bottom(),
                         rect.width(), rect.height());
        }
    } else {
        rect = SkIRect::MakeWH(fRenderTarget->width(), fRenderTarget->height());
    }
    fBuilder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void GrNXTGpuRTCommandBuffer::applyState(const GrPipeline& pipeline,
                                         const GrPrimitiveProcessor& primProc,
                                         const GrPipeline::FixedDynamicState* fixedDynamicState,
                                         const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                         const GrPrimitiveType primitiveType,
                                         bool hasPoints) {
    sk_sp<GrNXTProgram> program = fGpu->getOrCreateRenderPipeline(fRenderTarget,
                                                                  pipeline,
                                                                  primProc,
                                                                  hasPoints,
                                                                  primitiveType);
    auto bindGroup = program->setData(fGpu, primProc, pipeline);
    fVertexStride = program->fVertexStride;
    fBuilder.SetRenderPipeline(program->fRenderPipeline).SetBindGroup(0, bindGroup);
    if (pipeline.isStencilEnabled()) {
        fBuilder.SetStencilReference(pipeline.getUserStencil()->fFront.fRef);
    }
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);
    float c[4];
    GrColorToRGBAFloat(blendInfo.fBlendConstant, c);
    fBuilder.SetBlendColor(c[0], c[1], c[2], c[3]);
    setScissorState(pipeline, fixedDynamicState, dynamicStateArrays);
}

void GrNXTGpuRTCommandBuffer::onDraw(const GrPrimitiveProcessor& primProc,
                                     const GrPipeline& pipeline,
                                     const GrPipeline::FixedDynamicState* fixedDynamicState,
                                     const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                     const GrMesh meshes[],
                                     int meshCount,
                                     const SkRect& bounds) {
    SkASSERT(pipeline.renderTarget() == fRenderTarget);

    if (!meshCount) {
        return;
    }
    bool hasPoints = false;
    for (int i = 0; i < meshCount; ++i) {
        if (meshes[i].primitiveType() == GrPrimitiveType::kPoints) {
            hasPoints = true;
        }
    }
    for (int i = 0; i < meshCount; ++i) {
        applyState(pipeline, primProc, fixedDynamicState, dynamicStateArrays,
                   meshes[0].primitiveType(), hasPoints);
        meshes[i].sendToGpu(this);
    }
}

void GrNXTGpuRTCommandBuffer::sendInstancedMeshToGpu(GrPrimitiveType,
                                                     const GrBuffer* vertexBuffer,
                                                     int vertexCount,
                                                     int baseVertex,
                                                     const GrBuffer* instanceBuffer,
                                                     int instanceCount,
                                                     int baseInstance) {
    static const uint32_t vertexBufferOffsets[1] = {0};
    nxt::Buffer vb = static_cast<const GrNXTBuffer*>(vertexBuffer)->get();
    fBuilder.SetVertexBuffers(0, 1, &vb, vertexBufferOffsets)
            .DrawArrays(vertexCount, 1, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrNXTGpuRTCommandBuffer::sendIndexedInstancedMeshToGpu(GrPrimitiveType,
                                                            const GrBuffer* indexBuffer,
                                                            int indexCount,
                                                            int baseIndex,
                                                            const GrBuffer* vertexBuffer,
                                                            int baseVertex,
                                                            const GrBuffer* instanceBuffer,
                                                            int instanceCount,
                                                            int baseInstance,
                                                            GrPrimitiveRestart restart) {
    uint32_t vertexBufferOffsets[1];
    vertexBufferOffsets[0] = baseVertex * fVertexStride;
    nxt::Buffer vb = static_cast<const GrNXTBuffer*>(vertexBuffer)->get();
    nxt::Buffer ib = static_cast<const GrNXTBuffer*>(indexBuffer)->get();
    fBuilder.SetIndexBuffer(ib, 0)
            .SetVertexBuffers(0, 1, &vb, vertexBufferOffsets)
            .DrawElements(indexCount, 1, baseIndex, baseInstance);
    fGpu->stats()->incNumDraws();
}
