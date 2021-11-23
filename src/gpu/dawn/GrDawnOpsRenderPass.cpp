/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnOpsRenderPass.h"

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/dawn/GrDawnAttachment.h"
#include "src/gpu/dawn/GrDawnBuffer.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnProgramBuilder.h"
#include "src/gpu/dawn/GrDawnRenderTarget.h"
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
    if (GrTexture* tex = fRenderTarget->asTexture()) {
        tex->markMipmapsDirty();
    }
    auto stencilAttachment = static_cast<GrDawnAttachment*>(fRenderTarget->getStencilAttachment());

    const float* c = fColorInfo.fClearColor.data();

    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.view = static_cast<GrDawnRenderTarget*>(fRenderTarget)->textureView();
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.clearColor = { c[0], c[1], c[2], c[3] };
    colorAttachment.loadOp = colorOp;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    wgpu::RenderPassColorAttachment* colorAttachments = { &colorAttachment };
    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = colorAttachments;
    if (stencilAttachment) {
        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
        depthStencilAttachment.view = stencilAttachment->view();
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

void GrDawnOpsRenderPass::submit() {
    fGpu->appendCommandBuffer(fEncoder.Finish());
}

void GrDawnOpsRenderPass::onClearStencilClip(const GrScissorState& scissor,
                                             bool insideStencilMask) {
    SkASSERT(!scissor.enabled());
    fPassEncoder.EndPass();
    fPassEncoder = beginRenderPass(wgpu::LoadOp::Load, wgpu::LoadOp::Clear);
}

void GrDawnOpsRenderPass::onClear(const GrScissorState& scissor, std::array<float, 4> color) {
    SkASSERT(!scissor.enabled());
    fPassEncoder.EndPass();
    fPassEncoder = beginRenderPass(wgpu::LoadOp::Clear, wgpu::LoadOp::Load);
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnOpsRenderPass::inlineUpload(GrOpFlushState* state,
                                       GrDeferredTextureUploadFn& upload) {
    fGpu->submitToGpu(false);
    state->doUpload(upload);
}

////////////////////////////////////////////////////////////////////////////////

// Our caps require there to be a single reference value for both faces. However, our stencil
// object asserts if the correct face getter is not queried.
static uint16_t get_stencil_ref(const GrProgramInfo& info) {
    GrStencilSettings stencilSettings = info.nonGLStencilSettings();
    if (stencilSettings.isTwoSided()) {
        SkASSERT(stencilSettings.postOriginCCWFace(info.origin()).fRef ==
                 stencilSettings.postOriginCWFace(info.origin()).fRef);
        return stencilSettings.postOriginCCWFace(info.origin()).fRef;
    } else {
        return stencilSettings.singleSidedFace().fRef;
    }
}

void GrDawnOpsRenderPass::applyState(GrDawnProgram* program, const GrProgramInfo& programInfo) {
    auto bindGroup = program->setUniformData(fGpu, fRenderTarget, programInfo);
    fPassEncoder.SetPipeline(program->fRenderPipeline);
    fPassEncoder.SetBindGroup(0, bindGroup, 0, nullptr);
    if (programInfo.isStencilEnabled()) {
        fPassEncoder.SetStencilReference(get_stencil_ref(programInfo));
    }
    const GrPipeline& pipeline = programInfo.pipeline();
    GrXferProcessor::BlendInfo blendInfo = pipeline.getXferProcessor().getBlendInfo();
    const float* c = blendInfo.fBlendConstant.vec();
    wgpu::Color color{c[0], c[1], c[2], c[3]};
    fPassEncoder.SetBlendConstant(&color);
    if (!programInfo.pipeline().isScissorTestEnabled()) {
        // "Disable" scissor by setting it to the full pipeline bounds.
        SkIRect rect = SkIRect::MakeWH(fRenderTarget->width(), fRenderTarget->height());
        fPassEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
    }
}

void GrDawnOpsRenderPass::onEnd() {
    fPassEncoder.EndPass();
}

bool GrDawnOpsRenderPass::onBindPipeline(const GrProgramInfo& programInfo,
                                         const SkRect& drawBounds) {
    fCurrentProgram = fGpu->getOrCreateRenderPipeline(fRenderTarget, programInfo);
    if (!fCurrentProgram) {
        return false;
    }
    this->applyState(fCurrentProgram.get(), programInfo);
    return true;
}

void GrDawnOpsRenderPass::onSetScissorRect(const SkIRect& scissor) {
    // Higher-level skgpu::v1::SurfaceDrawContext and clips should have already ensured draw
    // bounds are restricted to the render target.
    SkASSERT(SkIRect::MakeSize(fRenderTarget->dimensions()).contains(scissor));
    auto nativeScissorRect =
            GrNativeRect::MakeRelativeTo(fOrigin, fRenderTarget->height(), scissor);
    fPassEncoder.SetScissorRect(nativeScissorRect.fX, nativeScissorRect.fY,
                                nativeScissorRect.fWidth, nativeScissorRect.fHeight);
}

bool GrDawnOpsRenderPass::onBindTextures(const GrGeometryProcessor& geomProc,
                                         const GrSurfaceProxy* const geomProcTextures[],
                                         const GrPipeline& pipeline) {
    auto bindGroup = fCurrentProgram->setTextures(fGpu, geomProc, pipeline, geomProcTextures);
    if (bindGroup) {
        fPassEncoder.SetBindGroup(1, bindGroup, 0, nullptr);
    }
    return true;
}

void GrDawnOpsRenderPass::onBindBuffers(sk_sp<const GrBuffer> indexBuffer,
                                        sk_sp<const GrBuffer> instanceBuffer,
                                        sk_sp<const GrBuffer> vertexBuffer,
                                        GrPrimitiveRestart) {
    if (vertexBuffer) {
        wgpu::Buffer vertex = static_cast<const GrDawnBuffer*>(vertexBuffer.get())->get();
        fPassEncoder.SetVertexBuffer(0, vertex);
    }
    if (instanceBuffer) {
        wgpu::Buffer instance = static_cast<const GrDawnBuffer*>(instanceBuffer.get())->get();
        fPassEncoder.SetVertexBuffer(1, instance);
    }
    if (indexBuffer) {
        wgpu::Buffer index = static_cast<const GrDawnBuffer*>(indexBuffer.get())->get();
        fPassEncoder.SetIndexBuffer(index, wgpu::IndexFormat::Uint16);
    }
}

void GrDawnOpsRenderPass::onDraw(int vertexCount, int baseVertex) {
    this->onDrawInstanced(1, 0, vertexCount, baseVertex);
}

void GrDawnOpsRenderPass::onDrawInstanced(int instanceCount, int baseInstance,
                                          int vertexCount, int baseVertex) {
    fPassEncoder.Draw(vertexCount, instanceCount, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrDawnOpsRenderPass::onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                                        uint16_t maxIndexValue, int baseVertex) {
    this->onDrawIndexedInstanced(indexCount, baseIndex, 1, 0, baseVertex);
}

void GrDawnOpsRenderPass::onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount,
                                                 int baseInstance, int baseVertex) {
    fPassEncoder.DrawIndexed(indexCount, instanceCount, baseIndex, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}
