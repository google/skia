/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DOpsRenderPass.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DPipelineState.h"
#include "src/gpu/d3d/GrD3DPipelineStateBuilder.h"
#include "src/gpu/d3d/GrD3DRenderTarget.h"
#include "src/gpu/d3d/GrD3DTexture.h"

GrD3DOpsRenderPass::GrD3DOpsRenderPass(GrD3DGpu* gpu) : fGpu(gpu) {}

bool GrD3DOpsRenderPass::set(GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
                             const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                             const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
                             const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    SkASSERT(!fRenderTarget);
    SkASSERT(fGpu == rt->getContext()->priv().getGpu());

    this->INHERITED::set(rt, origin);

    fBounds = bounds;

    fColorLoadOp = colorInfo.fLoadOp;
    fClearColor = colorInfo.fClearColor;

    // TODO

    return true;
}

GrD3DOpsRenderPass::~GrD3DOpsRenderPass() {}

GrGpu* GrD3DOpsRenderPass::gpu() { return fGpu; }

void GrD3DOpsRenderPass::onBegin() {
    GrD3DRenderTarget* d3dRT = static_cast<GrD3DRenderTarget*>(fRenderTarget);
    d3dRT->setResourceState(fGpu, D3D12_RESOURCE_STATE_RENDER_TARGET);
    fGpu->currentCommandList()->setRenderTarget(d3dRT);
    // TODO: set stencil too

    if (GrLoadOp::kClear == fColorLoadOp) {
        fGpu->currentCommandList()->clearRenderTargetView(
                d3dRT, fClearColor, GrScissorState(fRenderTarget->dimensions()));
    }
}

void set_stencil_ref(GrD3DGpu* gpu, const GrProgramInfo& info) {
    GrStencilSettings stencilSettings = info.nonGLStencilSettings();
    if (!stencilSettings.isDisabled()) {
        unsigned int stencilRef = 0;
        if (stencilSettings.isTwoSided()) {
            stencilRef = stencilSettings.postOriginCCWFace(info.origin()).fRef;
        } else {
            stencilRef = stencilSettings.singleSidedFace().fRef;
        }
        gpu->currentCommandList()->setStencilRef(stencilRef);
    }
}

void set_blend_factor(GrD3DGpu* gpu, const GrProgramInfo& info) {
    const GrXferProcessor& xferProcessor = info.pipeline().getXferProcessor();
    const GrSwizzle& swizzle = info.pipeline().writeSwizzle();
    const GrXferProcessor::BlendInfo& blendInfo = xferProcessor.getBlendInfo();
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    float floatColors[4];
    if (GrBlendCoeffRefsConstant(srcCoeff) || GrBlendCoeffRefsConstant(dstCoeff)) {
        // Swizzle the blend to match what the shader will output.
        SkPMColor4f blendConst = swizzle.applyTo(blendInfo.fBlendConstant);
        floatColors[0] = blendConst.fR;
        floatColors[1] = blendConst.fG;
        floatColors[2] = blendConst.fB;
        floatColors[3] = blendConst.fA;
    } else {
        memset(floatColors, 0, 4 * sizeof(float));
    }
    gpu->currentCommandList()->setBlendFactor(floatColors);
}

void set_primitive_topology(GrD3DGpu* gpu, const GrProgramInfo& info) {
    D3D12_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    switch (info.primitiveType()) {
        case GrPrimitiveType::kTriangles:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case GrPrimitiveType::kTriangleStrip:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        case GrPrimitiveType::kPoints:
            topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case GrPrimitiveType::kLines:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case GrPrimitiveType::kLineStrip:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case GrPrimitiveType::kPatches: // Unsupported
        case GrPrimitiveType::kPath: // Unsupported
        default:
            SkUNREACHABLE;
    }
    gpu->currentCommandList()->setPrimitiveTopology(topology);
}

void set_scissor_rects(GrD3DGpu* gpu, const GrRenderTarget* renderTarget, GrSurfaceOrigin rtOrigin,
                       const SkIRect& scissorRect) {
    SkASSERT(scissorRect.isEmpty() ||
             SkIRect::MakeWH(renderTarget->width(), renderTarget->height()).contains(scissorRect));

    D3D12_RECT scissor;
    scissor.left = scissorRect.fLeft;
    scissor.right = scissorRect.fRight;
    if (kTopLeft_GrSurfaceOrigin == rtOrigin) {
        scissor.top = scissorRect.fTop;
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == rtOrigin);
        scissor.top = renderTarget->height() - scissorRect.fBottom;
    }
    scissor.bottom = scissor.top + scissorRect.height();

    SkASSERT(scissor.left >= 0);
    SkASSERT(scissor.top >= 0);
    gpu->currentCommandList()->setScissorRects(1, &scissor);
}

void set_viewport(GrD3DGpu* gpu, const GrRenderTarget* renderTarget) {
    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = SkIntToScalar(renderTarget->width());
    viewport.Height = SkIntToScalar(renderTarget->height());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    gpu->currentCommandList()->setViewports(1, &viewport);
}

bool GrD3DOpsRenderPass::onBindPipeline(const GrProgramInfo& info, const SkRect& drawBounds) {
    SkRect rtRect = SkRect::Make(fBounds);
    if (rtRect.intersect(drawBounds)) {
        rtRect.roundOut(&fCurrentPipelineBounds);
    } else {
        fCurrentPipelineBounds.setEmpty();
    }

    fCurrentPipelineState =
            fGpu->resourceProvider().findOrCreateCompatiblePipelineState(fRenderTarget, info);
    if (!fCurrentPipelineState) {
        return false;
    }

    fGpu->currentCommandList()->setGraphicsRootSignature(fCurrentPipelineState->rootSignature());
    fGpu->currentCommandList()->setPipelineState(fCurrentPipelineState);

    fCurrentPipelineState->setAndBindConstants(fGpu, fRenderTarget, info);

    set_stencil_ref(fGpu, info);
    set_blend_factor(fGpu, info);
    set_primitive_topology(fGpu, info);
    if (!info.pipeline().isScissorTestEnabled()) {
        // "Disable" scissor by setting it to the full pipeline bounds.
        set_scissor_rects(fGpu, fRenderTarget, fOrigin, fCurrentPipelineBounds);
    }
    set_viewport(fGpu, fRenderTarget);

    return true;
}

void update_resource_state(GrTexture* tex, GrRenderTarget* rt, GrD3DGpu* gpu) {
    SkASSERT(!tex->isProtected() || (rt->isProtected() && gpu->protectedContext()));
    GrD3DTexture* d3dTex = static_cast<GrD3DTexture*>(tex);
    SkASSERT(d3dTex);
    d3dTex->setResourceState(gpu, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

bool GrD3DOpsRenderPass::onBindTextures(const GrPrimitiveProcessor& primProc,
                                        const GrSurfaceProxy* const primProcTextures[],
                                        const GrPipeline& pipeline) {
    SkASSERT(fCurrentPipelineState);

    // update textures to sampled resource state
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        update_resource_state(primProcTextures[i]->peekTexture(), fRenderTarget, fGpu);
    }
    GrFragmentProcessor::PipelineTextureSamplerRange textureSamplerRange(pipeline);
    for (auto [sampler, fp] : textureSamplerRange) {
        update_resource_state(sampler.peekTexture(), fRenderTarget, fGpu);
    }
    if (GrTexture* dstTexture = pipeline.peekDstTexture()) {
        update_resource_state(dstTexture, fRenderTarget, fGpu);
    }

    // TODO: possibly check for success once we start binding properly
    fCurrentPipelineState->setAndBindTextures(fGpu, primProc, primProcTextures, pipeline);

    return true;
}

void GrD3DOpsRenderPass::onBindBuffers(const GrBuffer* indexBuffer, const GrBuffer* instanceBuffer,
                                       const GrBuffer* vertexBuffer,
                                       GrPrimitiveRestart primRestart) {
    SkASSERT(GrPrimitiveRestart::kNo == primRestart);
    SkASSERT(fCurrentPipelineState);
    SkASSERT(!fGpu->caps()->usePrimitiveRestart());  // Ignore primitiveRestart parameter.

    GrD3DDirectCommandList* currCmdList = fGpu->currentCommandList();
    SkASSERT(currCmdList);

    // TODO: do we need a memory barrier here?

    fCurrentPipelineState->bindBuffers(fGpu, indexBuffer, instanceBuffer, vertexBuffer,
                                       currCmdList);
}

void GrD3DOpsRenderPass::onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                                            int baseVertex) {
    SkASSERT(fCurrentPipelineState);
    fGpu->currentCommandList()->drawInstanced(vertexCount, instanceCount, baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrD3DOpsRenderPass::onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount,
                                                int baseInstance, int baseVertex) {
    SkASSERT(fCurrentPipelineState);
    fGpu->currentCommandList()->drawIndexedInstanced(indexCount, instanceCount, baseIndex,
                                                     baseVertex, baseInstance);
    fGpu->stats()->incNumDraws();
}

void GrD3DOpsRenderPass::onClear(const GrScissorState& scissor, const SkPMColor4f& color) {
    fGpu->clear(scissor, color, fRenderTarget);
}
