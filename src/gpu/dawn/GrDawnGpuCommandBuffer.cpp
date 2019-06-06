/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnGpuCommandBuffer.h"

#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/dawn/GrDawnBuffer.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnProgramBuilder.h"
#include "src/gpu/dawn/GrDawnStencilAttachment.h"
#include "src/gpu/dawn/GrDawnTexture.h"
#include "src/gpu/dawn/GrDawnUtil.h"
#include "include/core/SkRect.h"
#include "src/sksl/SkSLCompiler.h"

GrDawnGpuTextureCommandBuffer::GrDawnGpuTextureCommandBuffer(GrDawnGpu* gpu,
                                                             GrTexture* texture,
                                                             GrSurfaceOrigin origin)
    : INHERITED(texture, origin)
    , fGpu(gpu) {
    fEncoder = fGpu->device().CreateCommandEncoder();
}

void GrDawnGpuTextureCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin,
                                        const SkIRect& srcRect, const SkIPoint& dstPoint) {
    if (!src->asTexture()) {
        return;
    }
    uint32_t width = srcRect.width(), height = srcRect.height();
    size_t rowBytes = srcRect.width() * GrBytesPerPixel(src->config());
    if ((rowBytes & 0xFF) != 0) {
        rowBytes = (rowBytes + 0xFF) & ~0xFF;
    }
    size_t sizeInBytes = height * rowBytes;
    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
    desc.size = sizeInBytes;
    dawn::Buffer buffer = fGpu->device().CreateBuffer(&desc);
    dawn::TextureCopyView srcTextureView, dstTextureView;
    srcTextureView.texture = static_cast<GrDawnTexture*>(src->asTexture())->texture();
    srcTextureView.level = 0;
    srcTextureView.slice = 0;
    srcTextureView.origin = {(uint32_t) srcRect.x(), (uint32_t) srcRect.y(), 0};
    dstTextureView.texture = static_cast<GrDawnTexture*>(fTexture)->texture();
    srcTextureView.level = 0;
    srcTextureView.slice = 0;
    dstTextureView.origin = {(uint32_t) dstPoint.x(), (uint32_t) dstPoint.y(), 0};
    dawn::BufferCopyView bufferView;
    bufferView.buffer = buffer;
    bufferView.offset = 0;
    bufferView.rowPitch = rowBytes;
    bufferView.imageHeight = height;
    dawn::Extent3D copySize = {width, height, 1};
    fEncoder.CopyTextureToBuffer(&srcTextureView, &bufferView, &copySize);
    fEncoder.CopyBufferToTexture(&bufferView, &dstTextureView, &copySize);
}

void GrDawnGpuTextureCommandBuffer::transferFrom(const SkIRect& srcRect, GrColorType bufferColorType,
                                                 GrGpuBuffer* transferBuffer, size_t offset) {
    fGpu->transferPixelsFrom(fTexture, srcRect.fLeft, srcRect.fTop, srcRect.width(),
                             srcRect.height(), bufferColorType, transferBuffer, offset);
}

void GrDawnGpuTextureCommandBuffer::submit() {
    dawn::CommandBuffer cmdBuf = fEncoder.Finish();
    fGpu->queue().Submit(1, &cmdBuf);
}

GrDawnGpuTextureCommandBuffer::~GrDawnGpuTextureCommandBuffer() {}

namespace {

dawn::LoadOp to_dawn_load_op(GrLoadOp loadOp) {
    switch (loadOp) {
        case GrLoadOp::kLoad:
            return dawn::LoadOp::Load;
        case GrLoadOp::kDiscard:
//            return dawn::LoadOp::DontCare;
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
    fEncoder = fGpu->device().CreateCommandEncoder();
    dawn::LoadOp colorOp = to_dawn_load_op(colorInfo.fLoadOp);
    dawn::LoadOp stencilOp = to_dawn_load_op(stencilInfo.fLoadOp);
    fPassEncoder = beginRenderPass(colorOp, stencilOp);
}

GrDawnGpuRTCommandBuffer::~GrDawnGpuRTCommandBuffer() {
}

GrGpu* GrDawnGpuRTCommandBuffer::gpu() { return fGpu; }

dawn::RenderPassEncoder GrDawnGpuRTCommandBuffer::beginRenderPass(dawn::LoadOp colorOp,
                                                                  dawn::LoadOp stencilOp) {
    GrBackendRenderTarget backendRT = fRenderTarget->getBackendRenderTarget();
    GrDawnImageInfo info;
    backendRT.getDawnImageInfo(&info);
    dawn::Texture texture(info.fTexture);
    auto stencilAttachment = static_cast<GrDawnStencilAttachment*>(
        fRenderTarget->renderTargetPriv().getStencilAttachment());
    dawn::TextureView colorView = texture.CreateDefaultView();
    const float *c = fColorInfo.fClearColor.vec();

    dawn::RenderPassColorAttachmentDescriptor colorAttachment;
    colorAttachment.attachment = colorView;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.clearColor = { c[0], c[1], c[2], c[3] };
    colorAttachment.loadOp = colorOp;
    colorAttachment.storeOp = dawn::StoreOp::Store;
    dawn::RenderPassColorAttachmentDescriptor* colorAttachments = { &colorAttachment };
    dawn::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachments;
    if (stencilAttachment) {
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = stencilAttachment->view();
        depthStencilAttachment.depthLoadOp = stencilOp;
        depthStencilAttachment.stencilLoadOp = stencilOp;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    } else {
        renderPassDescriptor.depthStencilAttachment = nullptr;
    }
    return fEncoder.BeginRenderPass(&renderPassDescriptor);
}

void GrDawnGpuRTCommandBuffer::end() {
    fPassEncoder.EndPass();
}

void GrDawnGpuRTCommandBuffer::submit() {
    fGpu->appendCommandBuffer(fEncoder.Finish());
}

void GrDawnGpuRTCommandBuffer::discard() {
    SkASSERT(!"unimplemented");
}

void GrDawnGpuRTCommandBuffer::insertEventMarker(const char* msg) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpuRTCommandBuffer::transferFrom(const SkIRect& srcRect, GrColorType bufferColorType,
                                            GrGpuBuffer* transferBuffer, size_t offset) {
    fGpu->transferPixelsFrom(fRenderTarget, srcRect.fLeft, srcRect.fTop, srcRect.width(),
                             srcRect.height(), bufferColorType, transferBuffer, offset);
}

void GrDawnGpuRTCommandBuffer::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    fPassEncoder.EndPass();
    fPassEncoder = beginRenderPass(dawn::LoadOp::Load, dawn::LoadOp::Clear);
}

void GrDawnGpuRTCommandBuffer::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    fPassEncoder.EndPass();
    fPassEncoder = beginRenderPass(dawn::LoadOp::Clear, dawn::LoadOp::Load);
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnGpuRTCommandBuffer::inlineUpload(GrOpFlushState* state,
                                            GrDeferredTextureUploadFn& upload) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpuRTCommandBuffer::copy(GrSurface* src, GrSurfaceOrigin srcOrigin,
                                    const SkIRect& srcRect, const SkIPoint& dstPoint) {
    auto s = static_cast<GrDawnTexture*>(src->asTexture());
    auto d = static_cast<GrDawnTexture*>(fRenderTarget->asTexture());

    if (!s || !d) {
        return;
    }

    dawn::Texture srcTex = s->texture();
    dawn::Texture dstTex = d->texture();

    uint32_t x = srcRect.x();
    uint32_t y = srcRect.y();
    uint32_t width = srcRect.width();
    uint32_t height = srcRect.height();
    int rowPitch = (width * GrBytesPerPixel(src->config()) + 0xFF) & ~0xFF;
    int sizeInBytes = rowPitch * height;
    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
    desc.size = sizeInBytes;
    dawn::Buffer buffer = fGpu->device().CreateBuffer(&desc);
    uint32_t dstX = dstPoint.x();
    uint32_t dstY = dstPoint.y();
    fPassEncoder.EndPass();
    dawn::TextureCopyView srcTextureCopyView;
    srcTextureCopyView.texture = srcTex;
    srcTextureCopyView.level = 0;
    srcTextureCopyView.slice = 0;
    srcTextureCopyView.origin = {x, y, 0};
    dawn::TextureCopyView dstTextureCopyView;
    dstTextureCopyView.texture = dstTex;
    dstTextureCopyView.level = 0;
    dstTextureCopyView.slice = 0;
    dstTextureCopyView.origin = {dstX, dstY, 0};
    dawn::BufferCopyView bufferCopyView;
    bufferCopyView.buffer = buffer;
    bufferCopyView.offset = 0;
    bufferCopyView.rowPitch = rowPitch;
    bufferCopyView.imageHeight = height;
    dawn::Extent3D copySize = {width, height, 1};
    fEncoder.CopyTextureToBuffer(&srcTextureCopyView, &bufferCopyView, &copySize);
    fEncoder.CopyBufferToTexture(&bufferCopyView, &dstTextureCopyView, &copySize);
    fPassEncoder = beginRenderPass(dawn::LoadOp::Load, dawn::LoadOp::Load);
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
        if (kBottomLeft_GrSurfaceOrigin == fOrigin) {
            rect.setXYWH(rect.x(), fRenderTarget->height() - rect.bottom(),
                         rect.width(), rect.height());
        }
    } else {
        rect = SkIRect::MakeWH(fRenderTarget->width(), fRenderTarget->height());
    }
    fPassEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void GrDawnGpuRTCommandBuffer::applyState(const GrPipeline& pipeline,
                                         const GrPrimitiveProcessor& primProc,
                                         const GrTextureProxy* const primProcProxies[],
                                         const GrPipeline::FixedDynamicState* fixedDynamicState,
                                         const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                         const GrPrimitiveType primitiveType,
                                         bool hasPoints) {
    sk_sp<GrDawnProgram> program = fGpu->getOrCreateRenderPipeline(fRenderTarget,
                                                                  fOrigin,
                                                                  pipeline,
                                                                  primProc,
                                                                  primProcProxies,
                                                                  hasPoints,
                                                                  primitiveType);
    auto bindGroup = program->setData(fGpu, fRenderTarget, fOrigin, primProc, pipeline,
                                      primProcProxies);
    fVertexStride = program->fVertexStride;
    fPassEncoder.SetPipeline(program->fRenderPipeline);
    fPassEncoder.SetBindGroup(0, bindGroup, 0, nullptr);
    if (pipeline.isStencilEnabled()) {
        fPassEncoder.SetStencilReference(pipeline.getUserStencil()->fFront.fRef);
    }
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);
    const float* c = blendInfo.fBlendConstant.vec();
    dawn::Color color{c[0], c[1], c[2], c[3]};
    fPassEncoder.SetBlendColor(&color);
    setScissorState(pipeline, fixedDynamicState, dynamicStateArrays);
}

void GrDawnGpuRTCommandBuffer::onDraw(const GrPrimitiveProcessor& primProc,
                                      const GrPipeline& pipeline,
                                      const GrPipeline::FixedDynamicState* fixedDynamicState,
                                      const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                      const GrMesh meshes[],
                                      int meshCount,
                                      const SkRect& bounds) {
    if (!meshCount) {
        return;
    }
    bool hasPoints = false;
    for (int i = 0; i < meshCount; ++i) {
        if (meshes[i].primitiveType() == GrPrimitiveType::kPoints) {
            hasPoints = true;
        }
    }
    const GrTextureProxy* const* primProcProxies = nullptr;
    if (dynamicStateArrays && dynamicStateArrays->fPrimitiveProcessorTextures) {
        primProcProxies = dynamicStateArrays->fPrimitiveProcessorTextures;
    } else if (fixedDynamicState) {
        primProcProxies = fixedDynamicState->fPrimitiveProcessorTextures;
    }
    for (int i = 0; i < meshCount; ++i) {
        applyState(pipeline, primProc, primProcProxies, fixedDynamicState, dynamicStateArrays,
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
    static const uint64_t vertexBufferOffsets[1] = {0};
    dawn::Buffer vb = static_cast<const GrDawnBuffer*>(vertexBuffer)->get();
    fPassEncoder.SetVertexBuffers(0, 1, &vb, vertexBufferOffsets);
    fPassEncoder.Draw(vertexCount, 1, baseVertex, baseInstance);
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
    uint64_t vertexBufferOffsets[1];
    vertexBufferOffsets[0] = 0;
    dawn::Buffer vb = static_cast<const GrDawnBuffer*>(vertexBuffer)->get();
    dawn::Buffer ib = static_cast<const GrDawnBuffer*>(indexBuffer)->get();
    fPassEncoder.SetIndexBuffer(ib, 0);
    fPassEncoder.SetVertexBuffers(0, 1, &vb, vertexBufferOffsets);
    fPassEncoder.DrawIndexed(indexCount, 1, baseIndex, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}
