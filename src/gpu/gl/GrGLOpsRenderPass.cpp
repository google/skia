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
    fGLProgram = fGpu->flushGLState(fRenderTarget, programInfo);
    return SkToBool(fGLProgram);
}

void GrGLOpsRenderPass::onSetScissorRect(const SkIRect& scissor) {
    fGpu->flushScissorRect(scissor, fRenderTarget->width(), fRenderTarget->height(), fOrigin);
}

bool GrGLOpsRenderPass::onBindTextures(const GrPrimitiveProcessor& primProc,
                                       const GrPipeline& pipeline,
                                       const GrSurfaceProxy* const primProcTextures[]) {
    fGLProgram->bindTextures(primProc, pipeline, primProcTextures);
    return true;
}

void GrGLOpsRenderPass::setupGeometry(const GrBuffer* indexBuffer, const GrBuffer* vertexBuffer,
                                      int baseVertex, const GrBuffer* instanceBuffer,
                                      int baseInstance, GrPrimitiveRestart primitiveRestart) {
    SkASSERT((primitiveRestart == GrPrimitiveRestart::kNo) || indexBuffer);

    int numAttribs = fGLProgram->numVertexAttributes() + fGLProgram->numInstanceAttributes();
    auto* attribState = fGpu->bindInternalVertexArray(indexBuffer, numAttribs, primitiveRestart);

    if (int vertexStride = fGLProgram->vertexStride()) {
        SkASSERT(vertexBuffer);
        SkASSERT(vertexBuffer->isCpuBuffer() ||
                 !static_cast<const GrGpuBuffer*>(vertexBuffer)->isMapped());
        size_t bufferOffset = baseVertex * static_cast<size_t>(vertexStride);
        for (int i = 0; i < fGLProgram->numVertexAttributes(); ++i) {
            const auto& attrib = fGLProgram->vertexAttribute(i);
            static constexpr int kDivisor = 0;
            attribState->set(fGpu, attrib.fLocation, vertexBuffer, attrib.fCPUType, attrib.fGPUType,
                             vertexStride, bufferOffset + attrib.fOffset, kDivisor);
        }
    }
    if (int instanceStride = fGLProgram->instanceStride()) {
        SkASSERT(instanceBuffer);
        SkASSERT(instanceBuffer->isCpuBuffer() ||
                 !static_cast<const GrGpuBuffer*>(instanceBuffer)->isMapped());
        size_t bufferOffset = baseInstance * static_cast<size_t>(instanceStride);
        int attribIdx = fGLProgram->numVertexAttributes();
        for (int i = 0; i < fGLProgram->numInstanceAttributes(); ++i, ++attribIdx) {
            const auto& attrib = fGLProgram->instanceAttribute(i);
            static constexpr int kDivisor = 1;
            attribState->set(fGpu, attrib.fLocation, instanceBuffer, attrib.fCPUType,
                             attrib.fGPUType, instanceStride, bufferOffset + attrib.fOffset,
                             kDivisor);
        }
    }
}

static const GrGLvoid* get_gl_index_ptr(const GrBuffer* indexBuffer, int baseIndex) {
    size_t baseOffset = baseIndex * sizeof(uint16_t);
    if (indexBuffer->isCpuBuffer()) {
        return static_cast<const GrCpuBuffer*>(indexBuffer)->data() + baseOffset;
    } else {
        return reinterpret_cast<const GrGLvoid*>(baseOffset);
    }
}

void GrGLOpsRenderPass::onDraw(const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) {
    if (fGpu->glCaps().drawArraysBaseVertexIsBroken()) {
        this->setupGeometry(nullptr, vertexBuffer, baseVertex, nullptr, 0, GrPrimitiveRestart::kNo);
        fGpu->drawArrays(fPrimitiveType, 0, vertexCount);
        return;
    }

    this->setupGeometry(nullptr, vertexBuffer, 0, nullptr, 0, GrPrimitiveRestart::kNo);
    fGpu->drawArrays(fPrimitiveType, baseVertex, vertexCount);
}

void GrGLOpsRenderPass::onDrawIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                                      GrPrimitiveRestart primitiveRestart, uint16_t minIndexValue,
                                      uint16_t maxIndexValue, const GrBuffer* vertexBuffer,
                                      int baseVertex) {
    const GrGLvoid* indexPtr = get_gl_index_ptr(indexBuffer, baseIndex);
    this->setupGeometry(indexBuffer, vertexBuffer, baseVertex, nullptr, 0, primitiveRestart);
    if (fGpu->glCaps().drawRangeElementsSupport()) {
        fGpu->drawRangeElements(fPrimitiveType, minIndexValue, maxIndexValue, indexCount,
                                GR_GL_UNSIGNED_SHORT, indexPtr);
    } else {
        fGpu->drawElements(fPrimitiveType, indexCount, GR_GL_UNSIGNED_SHORT, indexPtr);
    }
}

void GrGLOpsRenderPass::onDrawInstanced(const GrBuffer* instanceBuffer, int instanceCount,
                                        int baseInstance, const GrBuffer* vertexBuffer,
                                        int vertexCount, int baseVertex) {
    int maxInstances = fGpu->glCaps().maxInstancesPerDrawWithoutCrashing(instanceCount);
    for (int i = 0; i < instanceCount; i += maxInstances) {
        this->setupGeometry(nullptr, vertexBuffer, 0, instanceBuffer, baseInstance + i,
                            GrPrimitiveRestart::kNo);
        fGpu->drawArraysInstanced(fPrimitiveType, baseVertex, vertexCount,
                                  std::min(instanceCount - i, maxInstances));
    }
}

void GrGLOpsRenderPass::onDrawIndexedInstanced(
        const GrBuffer* indexBuffer, int indexCount, int baseIndex,
        GrPrimitiveRestart primitiveRestart, const GrBuffer* instanceBuffer, int instanceCount,
        int baseInstance, const GrBuffer* vertexBuffer, int baseVertex) {
    const GrGLvoid* indexPtr = get_gl_index_ptr(indexBuffer, baseIndex);
    int maxInstances = fGpu->glCaps().maxInstancesPerDrawWithoutCrashing(instanceCount);
    for (int i = 0; i < instanceCount; i += maxInstances) {
        this->setupGeometry(indexBuffer, vertexBuffer, baseVertex,
                            instanceBuffer, baseInstance + i, primitiveRestart);
        fGpu->drawElementsInstanced(fPrimitiveType, indexCount, GR_GL_UNSIGNED_SHORT, indexPtr,
                                    std::min(instanceCount - i, maxInstances));
    }
}

void GrGLOpsRenderPass::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    fGpu->clear(clip, color, fRenderTarget, fOrigin);
}

void GrGLOpsRenderPass::onClearStencilClip(const GrFixedClip& clip,
                                           bool insideStencilMask) {
    fGpu->clearStencilClip(clip, insideStencilMask, fRenderTarget, fOrigin);
}

