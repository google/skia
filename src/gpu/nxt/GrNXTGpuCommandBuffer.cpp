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
#include "SkRect.h"
#include "SkSLCompiler.h"

namespace {

nxt::Framebuffer create_framebuffer(GrNXTGpu* gpu, nxt::RenderPass renderPass,
                                    GrRenderTarget* rt, GrColor clearColor) {
    auto stencilAttachment = static_cast<GrNXTStencilAttachment*>(
        rt->renderTargetPriv().getStencilAttachment());
    GrNXTImageInfo* info = reinterpret_cast<GrNXTImageInfo*>(rt->getRenderTargetHandle());
    nxt::Texture texture(info->fTexture);
    nxt::TextureView colorView = texture.CreateTextureViewBuilder().GetResult();
    auto framebufferBuilder = gpu->device().CreateFramebufferBuilder()
        .SetRenderPass(renderPass)
        .SetDimensions(rt->width(), rt->height())
        .SetAttachment(0, colorView)
        .Clone();
    if (stencilAttachment) {
        framebufferBuilder.SetAttachment(1, stencilAttachment->view());
    }
    auto framebuffer = framebufferBuilder.GetResult();
    GrColor4f clearColor4(GrColor4f::FromGrColor(clearColor));
    const float *c = clearColor4.fRGBA;
    framebuffer.AttachmentSetClearColor(0, c[0], c[1], c[2], c[3]);
    return framebuffer;
}

}

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

////////////////////////////////////////////////////////////////////////////////

nxt::LoadOp GrLoadOpToNXTLoadOp(GrLoadOp loadOpIn) {
    switch (loadOpIn) {
        case GrLoadOp::kLoad:
            return nxt::LoadOp::Load;
        case GrLoadOp::kDiscard:
            // FIXME: NXT doesn't support discard (yet), so fall-through to clear
        case GrLoadOp::kClear:
            return nxt::LoadOp::Clear;
        default:
            SK_ABORT("Invalid LoadOp");
            return nxt::LoadOp::Load;
    }
}

GrNXTGpuRTCommandBuffer::GrNXTGpuRTCommandBuffer(GrNXTGpu* gpu,
                                                 GrRenderTarget* rt, GrSurfaceOrigin origin,
                                                 const LoadAndStoreInfo& colorInfo,
                                                 const StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu) {
    fRenderPass = gpu->getOrCreateRenderPass(rt, colorInfo.fLoadOp, stencilInfo.fLoadOp,
                                             &fRenderPassHash);
    nxt::Framebuffer framebuffer = create_framebuffer(gpu, fRenderPass.Clone(), rt,
                                                      colorInfo.fClearColor);
    fBuilder = fGpu->device().CreateCommandBufferBuilder();
    fBuilder.BeginRenderPass(fRenderPass, framebuffer)
            .BeginRenderSubpass();
}

GrNXTGpuRTCommandBuffer::~GrNXTGpuRTCommandBuffer() {
}

GrGpu* GrNXTGpuRTCommandBuffer::gpu() { return fGpu; }

void GrNXTGpuRTCommandBuffer::end() {
    fBuilder.EndRenderSubpass()
            .EndRenderPass();
}

void GrNXTGpuRTCommandBuffer::submit() {
    nxt::CommandBuffer commandBuffer = fBuilder.GetResult();
    if (commandBuffer) {
        fGpu->queue().Submit(1, &commandBuffer);
    }
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
    SkASSERT(!"unimplemented");
}

////////////////////////////////////////////////////////////////////////////////

void GrNXTGpuRTCommandBuffer::setScissorState(const GrPipeline& pipeline) {
    GrScissorState scissorState = pipeline.getScissorState();
    SkIRect rect;
    if (scissorState.enabled()) {
        rect = scissorState.rect();
        if (kBottomLeft_GrSurfaceOrigin == pipeline.proxy()->origin()) {
            rect.setXYWH(rect.x(), fRenderTarget->height() - rect.bottom(),
                         rect.width(), rect.height());
        }
    } else {
        rect = SkIRect::MakeWH(fRenderTarget->width(), fRenderTarget->height());
    }
    fBuilder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void GrNXTGpuRTCommandBuffer::beginDraw(const GrPipeline& pipeline,
                                        const GrPrimitiveProcessor& primProc,
                                        const GrPrimitiveType primitiveType,
                                        bool hasPoints) {
    sk_sp<GrNXTProgram> program = fGpu->getOrCreateRenderPipeline(fRenderPass.Clone(),
                                                                  fRenderPassHash,
                                                                  pipeline,
                                                                  primProc,
                                                                  hasPoints,
                                                                  primitiveType);
    auto bindGroup = program->setData(fGpu, primProc, pipeline, fBuilder.Clone());
    for (int i = 0; i < primProc.numTextureSamplers(); i++) {
        GrNXTTexture* tex = static_cast<GrNXTTexture*>(primProc.textureSampler(i).peekTexture());
        fBuilder.TransitionTextureUsage(tex->texture(), nxt::TextureUsageBit::Sampled);
    }
    GrFragmentProcessor::Iter iter(pipeline);
    const GrFragmentProcessor* fp = iter.next();
    while (fp) {
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            GrNXTTexture* tex = static_cast<GrNXTTexture*>(fp->textureSampler(i).peekTexture());
            fBuilder.TransitionTextureUsage(tex->texture(), nxt::TextureUsageBit::Sampled);
        }
        fp = iter.next();
    }
    fBuilder.SetRenderPipeline(program->fRenderPipeline)
            .SetBindGroup(0, bindGroup);
    if (pipeline.isStencilEnabled()) {
        fBuilder.SetStencilReference(pipeline.getUserStencil()->fFront.fRef);
    }
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);
    float c[4];
    GrColorToRGBAFloat(blendInfo.fBlendConstant, c);
    fBuilder.SetBlendColor(c[0], c[1], c[2], c[3]);
    setScissorState(pipeline);
}

void GrNXTGpuRTCommandBuffer::endDraw() {
}

void GrNXTGpuRTCommandBuffer::onDraw(const GrPipeline& pipeline,
                                     const GrPrimitiveProcessor& primProc,
                                     const GrMesh meshes[],
                                     const GrPipeline::DynamicState dynamicStates[],
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
    beginDraw(pipeline, primProc, meshes[0].primitiveType(), hasPoints);
    for (int i = 0; i < meshCount; ++i) {
        // FIXME: if the primitive type for this draw is different, we need to beginDraw()
        // again here and create a new pipeline.
        meshes[i].sendToGpu(primProc, this);
    }
    endDraw();
}

void GrNXTGpuRTCommandBuffer::sendInstancedMeshToGpu(const GrPrimitiveProcessor& primProc,
                                                     GrPrimitiveType,
                                                     const GrBuffer* vertexBuffer,
                                                     int vertexCount,
                                                     int baseVertex,
                                                     const GrBuffer* instanceBuffer,
                                                     int instanceCount,
                                                     int baseInstance) {
    static const uint32_t vertexBufferOffsets[1] = {0};
    nxt::Buffer vb = static_cast<const GrNXTBuffer*>(vertexBuffer)->get();
    fBuilder.TransitionBufferUsage(vb, nxt::BufferUsageBit::Vertex)
            .SetVertexBuffers(0, 1, &vb, vertexBufferOffsets)
            .DrawArrays(vertexCount, 1, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrNXTGpuRTCommandBuffer::sendIndexedInstancedMeshToGpu(const GrPrimitiveProcessor& primProc,
                                                            GrPrimitiveType,
                                                            const GrBuffer* indexBuffer,
                                                            int indexCount,
                                                            int baseIndex,
                                                            const GrBuffer* vertexBuffer,
                                                            int baseVertex,
                                                            const GrBuffer* instanceBuffer,
                                                            int instanceCount,
                                                            int baseInstance) {
    uint32_t vertexBufferOffsets[1];
    vertexBufferOffsets[0] = baseVertex * primProc.getVertexStride();
    nxt::Buffer vb = static_cast<const GrNXTBuffer*>(vertexBuffer)->get();
    nxt::Buffer ib = static_cast<const GrNXTBuffer*>(indexBuffer)->get();
    fBuilder.TransitionBufferUsage(ib, nxt::BufferUsageBit::Index)
            .TransitionBufferUsage(vb, nxt::BufferUsageBit::Vertex)
            .SetIndexBuffer(ib, 0)
            .SetVertexBuffers(0, 1, &vb, vertexBufferOffsets)
            .DrawElements(indexCount, 1, baseIndex, baseInstance);
    fGpu->stats()->incNumDraws();
}

