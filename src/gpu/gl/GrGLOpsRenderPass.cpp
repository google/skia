/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLOpsRenderPass.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTargetPriv.h"

void GrGLOpsRenderPass::set(GrRenderTarget* rt, const SkIRect& contentBounds,
                            GrSurfaceOrigin origin, const LoadAndStoreInfo& colorInfo,
                            const StencilLoadAndStoreInfo& stencilInfo) {
    SkASSERT(fGpu);
    SkASSERT(!fRenderTarget);
    SkASSERT(fGpu == rt->getContext()->priv().getGpu());

    this->INHERITED::set(rt, origin);
    fContentBounds = contentBounds;
    fColorLoadAndStoreInfo = colorInfo;
    fStencilLoadAndStoreInfo = stencilInfo;
}

bool GrGLOpsRenderPass::onBindPipeline(const GrProgramInfo& programInfo,
                                       const SkRect& drawBounds) {
    fPrimitiveType = programInfo.primitiveType();
    return fGpu->flushGLState(fRenderTarget, programInfo);
}

void GrGLOpsRenderPass::onSetScissorRect(const SkIRect& scissor) {
    fGpu->flushScissorRect(scissor, fRenderTarget->width(), fRenderTarget->height(), fOrigin);
}

bool GrGLOpsRenderPass::onBindTextures(const GrPrimitiveProcessor& primProc,
                                       const GrPipeline& pipeline,
                                       const GrSurfaceProxy* const primProcTextures[]) {
    fGpu->bindTextures(primProc, pipeline, primProcTextures);
    return true;
}

void GrGLOpsRenderPass::onDraw(const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) {
    fGpu->draw(fPrimitiveType, vertexBuffer, vertexCount, baseVertex);
}

void GrGLOpsRenderPass::onDrawIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                                      GrPrimitiveRestart primitiveRestart, uint16_t minIndexValue,
                                      uint16_t maxIndexValue, const GrBuffer* vertexBuffer,
                                      int baseVertex) {
    fGpu->drawIndexed(fPrimitiveType, indexBuffer, indexCount, baseIndex, primitiveRestart,
                      minIndexValue, maxIndexValue, vertexBuffer, baseVertex);
}

void GrGLOpsRenderPass::onDrawInstanced(const GrBuffer* instanceBuffer, int instanceCount,
                                        int baseInstance, const GrBuffer* vertexBuffer,
                                        int vertexCount, int baseVertex) {
    fGpu->drawInstanced(fPrimitiveType, instanceBuffer, instanceCount, baseInstance, vertexBuffer,
                      vertexCount, baseVertex);
}

void GrGLOpsRenderPass::onDrawIndexedInstanced(
        const GrBuffer* indexBuffer, int indexCount, int baseIndex,
        GrPrimitiveRestart primitiveRestart, const GrBuffer* instanceBuffer, int instanceCount,
        int baseInstance, const GrBuffer* vertexBuffer, int baseVertex) {
    fGpu->drawIndexedInstanced(fPrimitiveType, indexBuffer, indexCount, baseIndex, primitiveRestart,
                               instanceBuffer, instanceCount, baseInstance, vertexBuffer,
                               baseVertex);
}

void GrGLOpsRenderPass::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    fGpu->clear(clip, color, fRenderTarget, fOrigin);
}

void GrGLOpsRenderPass::onClearStencilClip(const GrFixedClip& clip,
                                           bool insideStencilMask) {
    fGpu->clearStencilClip(clip, insideStencilMask, fRenderTarget, fOrigin);
}

