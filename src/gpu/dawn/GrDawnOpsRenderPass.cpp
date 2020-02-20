/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnOpsRenderPass.h"

#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/dawn/GrDawnBuffer.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnProgramBuilder.h"
#include "src/gpu/dawn/GrDawnRenderTarget.h"
#include "src/gpu/dawn/GrDawnStencilAttachment.h"
#include "src/gpu/dawn/GrDawnTexture.h"
#include "src/gpu/dawn/GrDawnUtil.h"
#include "src/sksl/SkSLCompiler.h"

////////////////////////////////////////////////////////////////////////////////

static wgpu::LoadOp to_dawn_load_op(GrLoadOp loadOp) {
    switch (loadOp) {
        case GrLoadOp::kLoad:
            return wgpu::LoadOp::Load;
        case GrLoadOp::kDiscard:
            // Use LoadOp::Load to emulate DontCare.
            // Dawn doesn't have DontCare, for security reasons.
            // Load should be equivalent to DontCare for desktop; Clear would
            // probably be better for tilers. If Dawn does add DontCare
            // as an extension, use it here.
            return wgpu::LoadOp::Load;
        case GrLoadOp::kClear:
            return wgpu::LoadOp::Clear;
        default:
            SK_ABORT("Invalid LoadOp");
    }
}

GrDawnOpsRenderPass::GrDawnOpsRenderPass(GrDawnGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                                         const LoadAndStoreInfo& colorInfo,
                                         const StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu)
        , fColorInfo(colorInfo) {
    fEncoder = fGpu->device().CreateCommandEncoder();
    wgpu::LoadOp colorOp = to_dawn_load_op(colorInfo.fLoadOp);
    wgpu::LoadOp stencilOp = to_dawn_load_op(stencilInfo.fLoadOp);
    fPassEncoder = beginRenderPass(colorOp, stencilOp);
}

wgpu::RenderPassEncoder GrDawnOpsRenderPass::beginRenderPass(wgpu::LoadOp colorOp,
                                                             wgpu::LoadOp stencilOp) {
    auto stencilAttachment = static_cast<GrDawnStencilAttachment*>(
        fRenderTarget->renderTargetPriv().getStencilAttachment());
    const float *c = fColorInfo.fClearColor.vec();

    wgpu::RenderPassColorAttachmentDescriptor colorAttachment;
    colorAttachment.attachment = static_cast<GrDawnRenderTarget*>(fRenderTarget)->textureView();
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.clearColor = { c[0], c[1], c[2], c[3] };
    colorAttachment.loadOp = colorOp;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    wgpu::RenderPassColorAttachmentDescriptor* colorAttachments = { &colorAttachment };
    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = colorAttachments;
    if (stencilAttachment) {
        wgpu::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = stencilAttachment->view();
        depthStencilAttachment.depthLoadOp = stencilOp;
        depthStencilAttachment.stencilLoadOp = stencilOp;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    } else {
        renderPassDescriptor.depthStencilAttachment = nullptr;
    }
    return fEncoder.BeginRenderPass(&renderPassDescriptor);
}

GrDawnOpsRenderPass::~GrDawnOpsRenderPass() {
}

GrGpu* GrDawnOpsRenderPass::gpu() { return fGpu; }

void GrDawnOpsRenderPass::end() {
    fPassEncoder.EndPass();
}

void GrDawnOpsRenderPass::submit() {
    fGpu->appendCommandBuffer(fEncoder.Finish());
}

void GrDawnOpsRenderPass::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    fPassEncoder.EndPass();
    fPassEncoder = beginRenderPass(wgpu::LoadOp::Load, wgpu::LoadOp::Clear);
}

void GrDawnOpsRenderPass::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    fPassEncoder.EndPass();
    fPassEncoder = beginRenderPass(wgpu::LoadOp::Clear, wgpu::LoadOp::Load);
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnOpsRenderPass::inlineUpload(GrOpFlushState* state,
                                       GrDeferredTextureUploadFn& upload) {
    SkASSERT(!"unimplemented");
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnOpsRenderPass::setScissorState(const GrProgramInfo& programInfo) {
    SkIRect rect;
    if (programInfo.pipeline().isScissorEnabled()) {
        constexpr SkIRect kBogusScissor{0, 0, 1, 1};
        rect = programInfo.hasFixedScissor() ? programInfo.fixedScissor() : kBogusScissor;
        if (kBottomLeft_GrSurfaceOrigin == fOrigin) {
            rect.setXYWH(rect.x(), fRenderTarget->height() - rect.bottom(),
                         rect.width(), rect.height());
        }
    } else {
        rect = SkIRect::MakeWH(fRenderTarget->width(), fRenderTarget->height());
    }
    fPassEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void GrDawnOpsRenderPass::applyState(GrDawnProgram* program, const GrProgramInfo& programInfo) {
    auto bindGroup = program->setUniformData(fGpu, fRenderTarget, programInfo);
    fPassEncoder.SetPipeline(program->fRenderPipeline);
    fPassEncoder.SetBindGroup(0, bindGroup, 0, nullptr);
    const GrPipeline& pipeline = programInfo.pipeline();
    if (pipeline.isStencilEnabled()) {
        fPassEncoder.SetStencilReference(pipeline.getUserStencil()->fCCWFace.fRef);
    }
    GrXferProcessor::BlendInfo blendInfo = pipeline.getXferProcessor().getBlendInfo();
    const float* c = blendInfo.fBlendConstant.vec();
    wgpu::Color color{c[0], c[1], c[2], c[3]};
    fPassEncoder.SetBlendColor(&color);
    this->setScissorState(programInfo);
}

bool GrDawnOpsRenderPass::onBindPipeline(const GrProgramInfo& programInfo,
                                         const SkRect& drawBounds) {
    return true;
}

void GrDawnOpsRenderPass::onDrawMeshes(const GrProgramInfo& programInfo,
                                       const GrMesh meshes[],
                                       int meshCount) {
    if (!meshCount) {
        return;
    }
    sk_sp<GrDawnProgram> program = fGpu->getOrCreateRenderPipeline(fRenderTarget, programInfo);
    if (!programInfo.hasDynamicPrimProcTextures()) {
        auto textures = programInfo.hasFixedPrimProcTextures() ? programInfo.fixedPrimProcTextures()
                                                               : nullptr;
        auto bindGroup = program->setTextures(fGpu, programInfo, textures);
        fPassEncoder.SetBindGroup(1, bindGroup, 0, nullptr);
    }
    for (int i = 0; i < meshCount; ++i) {
        if (programInfo.hasDynamicPrimProcTextures()) {
            auto textures = programInfo.dynamicPrimProcTextures(i);
            auto bindGroup = program->setTextures(fGpu, programInfo, textures);
            fPassEncoder.SetBindGroup(1, bindGroup, 0, nullptr);
        }
        this->applyState(program.get(), programInfo);
        meshes[i].sendToGpu(programInfo.primitiveType(), this);
    }
}

void GrDawnOpsRenderPass::sendInstancedMeshToGpu(GrPrimitiveType, const GrMesh& mesh,
                                                 int vertexCount, int baseVertex, int instanceCount,
                                                 int baseInstance) {
    wgpu::Buffer vb = static_cast<const GrDawnBuffer*>(mesh.vertexBuffer())->get();
    fPassEncoder.SetVertexBuffer(0, vb);
    fPassEncoder.Draw(vertexCount, 1, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrDawnOpsRenderPass::sendIndexedInstancedMeshToGpu(GrPrimitiveType, const GrMesh& mesh,
                                                        int indexCount, int baseIndex,
                                                        int baseVertex, int instanceCount,
                                                        int baseInstance) {
    wgpu::Buffer vb = static_cast<const GrDawnBuffer*>(mesh.vertexBuffer())->get();
    wgpu::Buffer ib = static_cast<const GrDawnBuffer*>(mesh.indexBuffer())->get();
    fPassEncoder.SetIndexBuffer(ib);
    fPassEncoder.SetVertexBuffer(0, vb);
    fPassEncoder.DrawIndexed(indexCount, 1, baseIndex, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}
