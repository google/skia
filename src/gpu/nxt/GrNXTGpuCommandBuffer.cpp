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
#include "GrNXTRenderTarget.h"
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

////////////////////////////////////////////////////////////////////////////////

nxt::LoadOp GrLoadOpToNXTLoadOp(GrLoadOp loadOpIn) {
    switch (loadOpIn) {
        case GrLoadOp::kLoad:
            return nxt::LoadOp::Load;
        case GrLoadOp::kClear:
            return nxt::LoadOp::Clear;
        case GrLoadOp::kDiscard:
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
    fRenderPass = fGpu->device().CreateRenderPassBuilder()
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .AttachmentSetColorLoadOp(0, GrLoadOpToNXTLoadOp(colorInfo.fLoadOp))
        .SetSubpassCount(1)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
    GrNXTImageInfo* info = reinterpret_cast<GrNXTImageInfo*>(fRenderTarget->getRenderTargetHandle());
    nxt::Texture texture(info->fTexture);
    nxt::TextureView view = texture.CreateTextureViewBuilder().GetResult();
    auto framebuffer = fGpu->device().CreateFramebufferBuilder()
        .SetRenderPass(fRenderPass)
        .SetDimensions(fRenderTarget->width(), fRenderTarget->height())
        .SetAttachment(0, view)
        .GetResult();
    GrColor4f clearColor(GrColor4f::FromGrColor(colorInfo.fClearColor));
    const float *c = &clearColor.fRGBA[0];
    framebuffer.AttachmentSetClearColor(0, c[0], c[1], c[2], c[3]);
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

static nxt::VertexFormat to_nxt_vertex_format(GrVertexAttribType type) {
    switch (type) {
    case kFloat_GrVertexAttribType:
    case kHalf_GrVertexAttribType:
        return nxt::VertexFormat::FloatR32;
    case kFloat2_GrVertexAttribType:
    case kHalf2_GrVertexAttribType:
        return nxt::VertexFormat::FloatR32G32;
    case kFloat3_GrVertexAttribType:
    case kHalf3_GrVertexAttribType:
        return nxt::VertexFormat::FloatR32G32B32;
    case kFloat4_GrVertexAttribType:
    case kHalf4_GrVertexAttribType:
        return nxt::VertexFormat::FloatR32G32B32A32;
    case kUByte4_norm_GrVertexAttribType:
        return nxt::VertexFormat::UnormR8G8B8A8;
    default:
        SkASSERT(!"unsupported vertex format");
        return nxt::VertexFormat::FloatR32G32B32A32;
    }
}

void GrNXTGpuRTCommandBuffer::beginDraw(const GrPipeline& pipeline,
                                        const GrPrimitiveProcessor& primProc,
                                        bool hasPoints) {
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, primProc, hasPoints, pipeline, *fGpu->caps()->shaderCaps());
    sk_sp<GrNXTProgram> program = GrNXTProgramBuilder::Build(fGpu, pipeline, primProc, &desc);
    SkASSERT(program);
    program->setData(primProc, pipeline);

    auto inputStateBuilder = fGpu->device().CreateInputStateBuilder();
    int vertexBindingSlot = 0, instanceBindingSlot = 1;
    if (primProc.hasVertexAttribs()) {
        inputStateBuilder.SetInput(vertexBindingSlot, primProc.getVertexStride(), nxt::InputStepMode::Vertex);
    }
    if (primProc.hasInstanceAttribs()) {
        inputStateBuilder.SetInput(instanceBindingSlot, primProc.getVertexStride(), nxt::InputStepMode::Instance);
    }
    for (int i = 0; i < primProc.numAttribs(); i++) {
        const GrGeometryProcessor::Attribute& attrib = primProc.getAttrib(i);
        int input = attrib.fInputRate == GrPrimitiveProcessor::Attribute::InputRate::kPerVertex ? vertexBindingSlot : instanceBindingSlot;
        inputStateBuilder
            .SetAttribute(i, input, to_nxt_vertex_format(attrib.fType), attrib.fOffsetInRecord);
    }
    auto inputState = inputStateBuilder.GetResult();
    nxt::RenderPipeline renderPipeline = fGpu->device().CreateRenderPipelineBuilder()
        .SetSubpass(fRenderPass, 0)
        .SetLayout(program->fPipelineLayout)
        .SetStage(nxt::ShaderStage::Vertex, program->fVSModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, program->fFSModule, "main")
        .SetIndexFormat(nxt::IndexFormat::Uint16)
        .SetInputState(inputState)
        .SetColorAttachmentBlendState(0, program->fBlendState)
        .GetResult();
    if (program->fGeometryUniformBuffer) {
        fBuilder.TransitionBufferUsage(program->fGeometryUniformBuffer,
                                       nxt::BufferUsageBit::Uniform);
    }
    if (program->fFragmentUniformBuffer) {
        fBuilder.TransitionBufferUsage(program->fFragmentUniformBuffer,
                                       nxt::BufferUsageBit::Uniform);
    }
    fBuilder.SetRenderPipeline(renderPipeline)
            .SetBindGroup(0, program->fUniformBindGroup);
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
    beginDraw(pipeline, primProc, hasPoints);
    for (int i = 0; i < meshCount; ++i) {
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

