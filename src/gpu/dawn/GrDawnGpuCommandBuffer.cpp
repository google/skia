/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnGpuCommandBuffer.h"

#include "GrFixedClip.h"
#include "GrMesh.h"
#include "GrOpFlushState.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrTexturePriv.h"
#include "GrDawnBuffer.h"
#include "GrDawnGpu.h"
#include "GrDawnProgramBuilder.h"
#include "GrDawnStencilAttachment.h"
#include "GrDawnTexture.h"
#include "GrDawnUtil.h"
#include "SkRect.h"
#include "SkSLCompiler.h"

GrDawnGpuTextureCommandBuffer::GrDawnGpuTextureCommandBuffer(GrDawnGpu* gpu,
                                                           GrTexture* texture,
                                                           GrSurfaceOrigin origin)
    : INHERITED(texture, origin)
    , fGpu(gpu) {
    fBuilder = fGpu->device().CreateCommandBufferBuilder();
}

void GrDawnGpuTextureCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin,
                                        const SkIRect& srcRect, const SkIPoint& dstPoint) {
    if (!src->asTexture()) {
        return;
    }
    int32_t width = srcRect.width(), height = srcRect.height();
    size_t rowBytes = srcRect.width() * GrBytesPerPixel(src->config());
    if ((rowBytes & 0xFF) != 0) {
        rowBytes = (rowBytes + 0xFF) & ~0xFF;
    }
    size_t sizeInBytes = height * rowBytes;
    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
    desc.size = sizeInBytes;
    dawn::Buffer buffer = fGpu->device().CreateBuffer(&desc);
    dawn::Texture s = static_cast<GrDawnTexture*>(src->asTexture())->texture();
    dawn::Texture d = static_cast<GrDawnTexture*>(fTexture)->texture();
    fBuilder.CopyTextureToBuffer(s, srcRect.x(), srcRect.y(), 0, width, height, 1,
                                 0, 0, buffer, 0, rowBytes);
    fBuilder.CopyBufferToTexture(buffer, 0, rowBytes, d, dstPoint.x(), dstPoint.y(), 0,
                                 width, height, 1, 0, 0);
}

void GrDawnGpuTextureCommandBuffer::submit() {
    dawn::CommandBuffer cmdBuf = fBuilder.GetResult();
    fGpu->queue().Submit(1, &cmdBuf);
}

GrDawnGpuTextureCommandBuffer::~GrDawnGpuTextureCommandBuffer() {}

namespace {

dawn::LoadOp to_dawn_load_op(GrLoadOp loadOp) {
    switch (loadOp) {
        case GrLoadOp::kLoad:
            return dawn::LoadOp::Load;
        case GrLoadOp::kDiscard:
            return dawn::LoadOp::DontCare;
        case GrLoadOp::kClear:
            return dawn::LoadOp::Clear;
        default:
            SK_ABORT("Invalid LoadOp");
            return dawn::LoadOp::Load;
    }
}

}

GrDawnGpuRTCommandBuffer::GrDawnGpuRTCommandBuffer(GrDawnGpu* gpu,
                                                 GrRenderTarget* rt, GrSurfaceOrigin origin,
                                                 const LoadAndStoreInfo& colorInfo,
                                                 const StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu)
        , fColorInfo(colorInfo) {
    fBuilder = fGpu->device().CreateCommandBufferBuilder();
    dawn::LoadOp colorOp = to_dawn_load_op(colorInfo.fLoadOp);
    dawn::LoadOp stencilOp = to_dawn_load_op(stencilInfo.fLoadOp);
    fEncoder = fBuilder.BeginRenderPass(createRenderPassDescriptor(colorOp, stencilOp));
}

GrDawnGpuRTCommandBuffer::~GrDawnGpuRTCommandBuffer() {
}

GrGpu* GrDawnGpuRTCommandBuffer::gpu() { return fGpu; }

dawn::RenderPassDescriptor GrDawnGpuRTCommandBuffer::createRenderPassDescriptor(
        dawn::LoadOp colorOp,
        dawn::LoadOp stencilOp) const {
    GrBackendRenderTarget backendRT = fRenderTarget->getBackendRenderTarget();
    GrDawnImageInfo info;
    backendRT.getDawnImageInfo(&info);
    dawn::Texture texture(info.fTexture.Clone());
    auto stencilAttachment = static_cast<GrDawnStencilAttachment*>(
        fRenderTarget->renderTargetPriv().getStencilAttachment());
    dawn::TextureView colorView = texture.CreateDefaultTextureView();
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

void GrDawnGpuRTCommandBuffer::end() {
    fEncoder.EndPass();
}

void GrDawnGpuRTCommandBuffer::submit() {
    fGpu->appendCommandBuffer(fBuilder.GetResult());
}

void GrDawnGpuRTCommandBuffer::discard() {
    SkASSERT(!"unimplemented");
}

void GrDawnGpuRTCommandBuffer::insertEventMarker(const char* msg) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpuRTCommandBuffer::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    fEncoder.EndPass();
    fEncoder = fBuilder.BeginRenderPass(createRenderPassDescriptor(dawn::LoadOp::Load, dawn::LoadOp::Clear));
}

void GrDawnGpuRTCommandBuffer::onClear(const GrFixedClip& clip, GrColor color) {
    fEncoder.EndPass();
    fEncoder = fBuilder.BeginRenderPass(createRenderPassDescriptor(dawn::LoadOp::Clear, dawn::LoadOp::Load));
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnGpuRTCommandBuffer::inlineUpload(GrOpFlushState* state,
                                           GrDeferredTextureUploadFn& upload) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpuRTCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                                   const SkIPoint& dstPoint) {
    auto s = static_cast<GrDawnTexture*>(src->asTexture());
    auto d = static_cast<GrDawnTexture*>(fRenderTarget->asTexture());

    if (!s || !d) {
        return;
    }

    dawn::Texture srcTex = s->texture();
    dawn::Texture dstTex = d->texture();

    int x = srcRect.x();
    int y = srcRect.y();
    int width = srcRect.width();
    int height = srcRect.height();
    int rowPitch = (width * GrBytesPerPixel(src->config()) + 0xFF) & ~0xFF;
    int sizeInBytes = rowPitch * height;
    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
    desc.size = sizeInBytes;
    dawn::Buffer buffer = fGpu->device().CreateBuffer(&desc);
    int dstX = dstPoint.x();
    int dstY = dstPoint.y();
    fEncoder.EndPass();
    fBuilder.CopyTextureToBuffer(srcTex, x, y, 0, width, height, 1, 0, 0, buffer, 0, rowPitch);
    fBuilder.CopyBufferToTexture(buffer, 0, rowPitch, dstTex, dstX, dstY, 0, width, height, 1, 0, 0);
    fEncoder = fBuilder.BeginRenderPass(createRenderPassDescriptor(dawn::LoadOp::Load, dawn::LoadOp::Load));
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnGpuRTCommandBuffer::setScissorState(
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
    fEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void GrDawnGpuRTCommandBuffer::applyState(const GrPipeline& pipeline,
                                         const GrPrimitiveProcessor& primProc,
                                         const GrPipeline::FixedDynamicState* fixedDynamicState,
                                         const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                         const GrPrimitiveType primitiveType,
                                         bool hasPoints) {
    sk_sp<GrDawnProgram> program = fGpu->getOrCreateRenderPipeline(fRenderTarget,
                                                                  pipeline,
                                                                  primProc,
                                                                  hasPoints,
                                                                  primitiveType);
    const GrTextureProxy* const* primProcTextures = nullptr;
    if (fixedDynamicState) {
        primProcTextures = fixedDynamicState->fPrimitiveProcessorTextures;
    }
    auto bindGroup = program->setData(fGpu, primProc, pipeline, primProcTextures);
    fVertexStride = program->fVertexStride;
    fEncoder.SetRenderPipeline(program->fRenderPipeline);
    fEncoder.SetBindGroup(0, bindGroup);
    if (pipeline.isStencilEnabled()) {
        fEncoder.SetStencilReference(pipeline.getUserStencil()->fFront.fRef);
    }
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);
    float c[4];
    GrColorToRGBAFloat(blendInfo.fBlendConstant, c);
    fEncoder.SetBlendColor(c[0], c[1], c[2], c[3]);
    setScissorState(pipeline, fixedDynamicState, dynamicStateArrays);
}

void GrDawnGpuRTCommandBuffer::onDraw(const GrPrimitiveProcessor& primProc,
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

void GrDawnGpuRTCommandBuffer::sendInstancedMeshToGpu(GrPrimitiveType,
                                                     const GrBuffer* vertexBuffer,
                                                     int vertexCount,
                                                     int baseVertex,
                                                     const GrBuffer* instanceBuffer,
                                                     int instanceCount,
                                                     int baseInstance) {
    static const uint32_t vertexBufferOffsets[1] = {0};
    dawn::Buffer vb = static_cast<const GrDawnBuffer*>(vertexBuffer)->get();
    fEncoder.SetVertexBuffers(0, 1, &vb, vertexBufferOffsets);
    fEncoder.DrawArrays(vertexCount, 1, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrDawnGpuRTCommandBuffer::sendIndexedInstancedMeshToGpu(GrPrimitiveType,
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
    dawn::Buffer vb = static_cast<const GrDawnBuffer*>(vertexBuffer)->get();
    dawn::Buffer ib = static_cast<const GrDawnBuffer*>(indexBuffer)->get();
    fEncoder.SetIndexBuffer(ib, 0);
    fEncoder.SetVertexBuffers(0, 1, &vb, vertexBufferOffsets);
    fEncoder.DrawElements(indexCount, 1, baseIndex, baseInstance);
    fGpu->stats()->incNumDraws();
}
