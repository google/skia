/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DOpsRenderPass.h"

#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DCommandSignature.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DPipelineState.h"
#include "src/gpu/d3d/GrD3DPipelineStateBuilder.h"
#include "src/gpu/d3d/GrD3DRenderTarget.h"
#include "src/gpu/d3d/GrD3DTexture.h"

#ifdef SK_DEBUG
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#endif

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
    if (d3dRT->numSamples() > 1) {
        d3dRT->msaaTextureResource()->setResourceState(fGpu, D3D12_RESOURCE_STATE_RENDER_TARGET);
    } else {
        d3dRT->setResourceState(fGpu, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    fGpu->currentCommandList()->setRenderTarget(d3dRT);

    if (GrLoadOp::kClear == fColorLoadOp) {
        // Passing in nullptr for the rect clears the entire d3d RT. Is this correct? Does the load
        // op respect the logical bounds of a RT?
        fGpu->currentCommandList()->clearRenderTargetView(d3dRT, fClearColor, nullptr);
    }

    if (auto stencil = d3dRT->getStencilAttachment()) {
        GrD3DAttachment* d3dStencil = static_cast<GrD3DAttachment*>(stencil);
        d3dStencil->setResourceState(fGpu, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        if (fStencilLoadOp == GrLoadOp::kClear) {
            fGpu->currentCommandList()->clearDepthStencilView(d3dStencil, 0, nullptr);
        }
    }
}

void set_stencil_ref(GrD3DGpu* gpu, const GrProgramInfo& info) {
    GrStencilSettings stencilSettings = info.nonGLStencilSettings();
    if (!stencilSettings.isDisabled()) {
        unsigned int stencilRef = 0;
        if (stencilSettings.isTwoSided()) {
            SkASSERT(stencilSettings.postOriginCCWFace(info.origin()).fRef ==
                     stencilSettings.postOriginCWFace(info.origin()).fRef);
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
    if (info.pipeline().isHWAntialiasState()) {
        fGpu->currentCommandList()->setDefaultSamplePositions();
    } else {
        fGpu->currentCommandList()->setCenteredSamplePositions(fRenderTarget->numSamples());
    }

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

void GrD3DOpsRenderPass::onSetScissorRect(const SkIRect& scissor) {
    SkIRect combinedScissorRect;
    if (!combinedScissorRect.intersect(fCurrentPipelineBounds, scissor)) {
        combinedScissorRect = SkIRect::MakeEmpty();
    }

    set_scissor_rects(fGpu, fRenderTarget, fOrigin, combinedScissorRect);
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

    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        update_resource_state(te.texture(), fRenderTarget, fGpu);
    });

    if (GrTexture* dstTexture = pipeline.peekDstTexture()) {
        update_resource_state(dstTexture, fRenderTarget, fGpu);
    }

    // TODO: possibly check for success once we start binding properly
    fCurrentPipelineState->setAndBindTextures(fGpu, primProc, primProcTextures, pipeline);

    return true;
}

void GrD3DOpsRenderPass::onBindBuffers(sk_sp<const GrBuffer> indexBuffer,
                                       sk_sp<const GrBuffer> instanceBuffer,
                                       sk_sp<const GrBuffer> vertexBuffer,
                                       GrPrimitiveRestart primRestart) {
    SkASSERT(GrPrimitiveRestart::kNo == primRestart);
    SkASSERT(fCurrentPipelineState);
    SkASSERT(!fGpu->caps()->usePrimitiveRestart());  // Ignore primitiveRestart parameter.

    GrD3DDirectCommandList* currCmdList = fGpu->currentCommandList();
    SkASSERT(currCmdList);

    fCurrentPipelineState->bindBuffers(fGpu, std::move(indexBuffer), std::move(instanceBuffer),
                                       std::move(vertexBuffer), currCmdList);
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

void GrD3DOpsRenderPass::onDrawIndirect(const GrBuffer* buffer, size_t offset, int drawCount) {
    constexpr unsigned int kSlot = 0;
    sk_sp<GrD3DCommandSignature> cmdSig = fGpu->resourceProvider().findOrCreateCommandSignature(
            GrD3DCommandSignature::ForIndexed::kNo, kSlot);
    fGpu->currentCommandList()->executeIndirect(cmdSig, drawCount,
                                                static_cast<const GrD3DBuffer*>(buffer), offset);
    fGpu->stats()->incNumDraws();
}

void GrD3DOpsRenderPass::onDrawIndexedIndirect(const GrBuffer* buffer, size_t offset,
                                               int drawCount) {
    constexpr unsigned int kSlot = 0;
    sk_sp<GrD3DCommandSignature> cmdSig = fGpu->resourceProvider().findOrCreateCommandSignature(
            GrD3DCommandSignature::ForIndexed::kYes, kSlot);
    fGpu->currentCommandList()->executeIndirect(cmdSig, drawCount,
                                                static_cast<const GrD3DBuffer*>(buffer), offset);
    fGpu->stats()->incNumDraws();
}


static D3D12_RECT scissor_to_d3d_clear_rect(const GrScissorState& scissor,
                                            const GrSurface* surface,
                                            GrSurfaceOrigin origin) {
    D3D12_RECT clearRect;
    // Flip rect if necessary
    SkIRect d3dRect;
    if (!scissor.enabled()) {
        d3dRect.setXYWH(0, 0, surface->width(), surface->height());
    } else if (kBottomLeft_GrSurfaceOrigin != origin) {
        d3dRect = scissor.rect();
    } else {
        d3dRect.setLTRB(scissor.rect().fLeft, surface->height() - scissor.rect().fBottom,
                        scissor.rect().fRight, surface->height() - scissor.rect().fTop);
    }
    clearRect.left = d3dRect.fLeft;
    clearRect.right = d3dRect.fRight;
    clearRect.top = d3dRect.fTop;
    clearRect.bottom = d3dRect.fBottom;
    return clearRect;
}

void GrD3DOpsRenderPass::onClear(const GrScissorState& scissor, std::array<float, 4> color) {
    D3D12_RECT clearRect = scissor_to_d3d_clear_rect(scissor, fRenderTarget, fOrigin);
    auto d3dRT = static_cast<GrD3DRenderTarget*>(fRenderTarget);
    SkASSERT(d3dRT->grD3DResourceState()->getResourceState() == D3D12_RESOURCE_STATE_RENDER_TARGET);
    fGpu->currentCommandList()->clearRenderTargetView(d3dRT, color, &clearRect);
}

void GrD3DOpsRenderPass::onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) {
    GrAttachment* sb = fRenderTarget->getStencilAttachment();
    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);
    int stencilBitCount = GrBackendFormatStencilBits(sb->backendFormat());

    // The contract with the callers does not guarantee that we preserve all bits in the stencil
    // during this clear. Thus we will clear the entire stencil to the desired value.

    uint8_t stencilColor = 0;
    if (insideStencilMask) {
        stencilColor = (1 << (stencilBitCount - 1));
    }

    D3D12_RECT clearRect = scissor_to_d3d_clear_rect(scissor, fRenderTarget, fOrigin);

    auto d3dStencil = static_cast<GrD3DAttachment*>(sb);
    fGpu->currentCommandList()->clearDepthStencilView(d3dStencil, stencilColor, &clearRect);
}

void GrD3DOpsRenderPass::inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) {
    // If we ever start using copy command lists for doing uploads, then we'll need to make sure
    // we submit our main command list before doing the copy here and then start a new main command
    // list.
    state->doUpload(upload);
}
