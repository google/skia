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

void GrGLOpsRenderPass::onBegin() {
    fGpu->beginCommandBuffer(fRenderTarget, fContentBounds, fOrigin, fColorLoadAndStoreInfo,
                             fStencilLoadAndStoreInfo);
}

void GrGLOpsRenderPass::onEnd() {
    fGpu->endCommandBuffer(fRenderTarget, fColorLoadAndStoreInfo, fStencilLoadAndStoreInfo);
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
                                       const GrSurfaceProxy* const primProcTextures[],
                                       const GrPipeline& pipeline) {
    GrGLProgram* program = fGpu->currentProgram();
    if (!program) {
        return false;
    }
    program->bindTextures(primProc, primProcTextures, pipeline);
    return true;
}

void GrGLOpsRenderPass::onBindBuffers(const GrBuffer* indexBuffer, const GrBuffer* instanceBuffer,
                                      const GrBuffer* vertexBuffer,
                                      GrPrimitiveRestart primitiveRestart) {
    SkASSERT((primitiveRestart == GrPrimitiveRestart::kNo) || indexBuffer);
    GrGLProgram* program = fGpu->currentProgram();
    if (!program) {
        return;
    }

    int numAttribs = program->numVertexAttributes() + program->numInstanceAttributes();
    fAttribArrayState = fGpu->bindInternalVertexArray(indexBuffer, numAttribs, primitiveRestart);

    if (indexBuffer) {
        if (indexBuffer->isCpuBuffer()) {
            auto* cpuIndexBuffer = static_cast<const GrCpuBuffer*>(indexBuffer);
            fIndexPointer = reinterpret_cast<const uint16_t*>(cpuIndexBuffer->data());
        } else {
            fIndexPointer = nullptr;
        }
    }

    if (fGpu->glCaps().baseInstanceSupport()) {
        SkASSERT(!fGpu->glCaps().drawArraysBaseVertexIsBroken());
        this->setupGeometry(vertexBuffer, 0, instanceBuffer, 0);
    } else {
        // This platform does not support baseInstance (or baseVertex). Defer binding of instance
        // and vertex buffers.
        fActiveInstanceBuffer = sk_ref_sp(instanceBuffer);
        fActiveVertexBuffer = sk_ref_sp(vertexBuffer);
    }
}

void GrGLOpsRenderPass::setupGeometry(const GrBuffer* vertexBuffer, int baseVertex,
                                      const GrBuffer* instanceBuffer, int baseInstance) {
    GrGLProgram* program = fGpu->currentProgram();
    if (!program) {
        return;
    }

    if (int vertexStride = program->vertexStride()) {
        SkASSERT(vertexBuffer);
        SkASSERT(vertexBuffer->isCpuBuffer() ||
                 !static_cast<const GrGpuBuffer*>(vertexBuffer)->isMapped());
        size_t bufferOffset = baseVertex * static_cast<size_t>(vertexStride);
        for (int i = 0; i < program->numVertexAttributes(); ++i) {
            const auto& attrib = program->vertexAttribute(i);
            static constexpr int kDivisor = 0;
            fAttribArrayState->set(fGpu, attrib.fLocation, vertexBuffer, attrib.fCPUType,
                                   attrib.fGPUType, vertexStride, bufferOffset + attrib.fOffset,
                                   kDivisor);
        }
    }
    if (int instanceStride = program->instanceStride()) {
        SkASSERT(instanceBuffer);
        SkASSERT(instanceBuffer->isCpuBuffer() ||
                 !static_cast<const GrGpuBuffer*>(instanceBuffer)->isMapped());
        size_t bufferOffset = baseInstance * static_cast<size_t>(instanceStride);
        int attribIdx = program->numVertexAttributes();
        for (int i = 0; i < program->numInstanceAttributes(); ++i, ++attribIdx) {
            const auto& attrib = program->instanceAttribute(i);
            static constexpr int kDivisor = 1;
            fAttribArrayState->set(fGpu, attrib.fLocation, instanceBuffer, attrib.fCPUType,
                                   attrib.fGPUType, instanceStride, bufferOffset + attrib.fOffset,
                                   kDivisor);
        }
    }
}

void GrGLOpsRenderPass::onDraw(int vertexCount, int baseVertex) {
    int baseVertexForDraw = baseVertex;
    if (!fGpu->glCaps().baseInstanceSupport()) {
        if (fGpu->glCaps().drawArraysBaseVertexIsBroken()) {
            this->setupGeometry(fActiveVertexBuffer.get(), baseVertexForDraw, nullptr, 0);
            baseVertexForDraw = 0;
        } else {
            this->setupGeometry(fActiveVertexBuffer.get(), 0, nullptr, 0);
        }
    }
    fGpu->drawArrays(fPrimitiveType, baseVertexForDraw, vertexCount);
}

void GrGLOpsRenderPass::onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                                      uint16_t maxIndexValue, int baseVertex) {
    if (!fGpu->glCaps().baseInstanceSupport()) {
        this->setupGeometry(fActiveVertexBuffer.get(), baseVertex, nullptr, 0);
    } else if (baseVertex != 0) {
        fGpu->drawElementsInstancedBaseVertexBaseInstance(
                fPrimitiveType, indexCount, GR_GL_UNSIGNED_SHORT,
                this->offsetForBaseIndex(baseIndex), 1, baseVertex, 0);
        return;
    }

    if (fGpu->glCaps().drawRangeElementsSupport()) {
        fGpu->drawRangeElements(fPrimitiveType, minIndexValue, maxIndexValue, indexCount,
                                GR_GL_UNSIGNED_SHORT, this->offsetForBaseIndex(baseIndex));
    } else {
        fGpu->drawElements(fPrimitiveType, indexCount, GR_GL_UNSIGNED_SHORT,
                           this->offsetForBaseIndex(baseIndex));
    }
}

void GrGLOpsRenderPass::onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                                        int baseVertex) {
    int maxInstances = fGpu->glCaps().maxInstancesPerDrawWithoutCrashing(instanceCount);
    for (int i = 0; i < instanceCount; i += maxInstances) {
        int instanceCountForDraw = std::min(instanceCount - i, maxInstances);
        int baseInstanceForDraw = baseInstance + i;
        if (fGpu->glCaps().baseInstanceSupport()) {
            fGpu->drawArraysInstancedBaseInstance(fPrimitiveType, baseVertex, vertexCount,
                                                  instanceCountForDraw, baseInstanceForDraw);
        } else {
            this->setupGeometry(fActiveVertexBuffer.get(), 0, fActiveInstanceBuffer.get(),
                                baseInstanceForDraw);
            fGpu->drawArraysInstanced(fPrimitiveType, baseVertex, vertexCount,
                                      instanceCountForDraw);
        }
    }
}

void GrGLOpsRenderPass::onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount,
                                               int baseInstance, int baseVertex) {
    int maxInstances = fGpu->glCaps().maxInstancesPerDrawWithoutCrashing(instanceCount);
    for (int i = 0; i < instanceCount; i += maxInstances) {
        int instanceCountForDraw = std::min(instanceCount - i, maxInstances);
        int baseInstanceForDraw = baseInstance + i;
        if (fGpu->glCaps().baseInstanceSupport()) {
            fGpu->drawElementsInstancedBaseVertexBaseInstance(
                    fPrimitiveType, indexCount, GR_GL_UNSIGNED_SHORT,
                    this->offsetForBaseIndex(baseIndex), instanceCountForDraw, baseVertex,
                    baseInstanceForDraw);
        } else {
            this->setupGeometry(fActiveVertexBuffer.get(), baseVertex, fActiveInstanceBuffer.get(),
                                baseInstanceForDraw);
            fGpu->drawElementsInstanced(fPrimitiveType, indexCount, GR_GL_UNSIGNED_SHORT,
                                        this->offsetForBaseIndex(baseIndex), instanceCountForDraw);
        }
    }
}

void GrGLOpsRenderPass::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    fGpu->clear(clip, color, fRenderTarget, fOrigin);
}

void GrGLOpsRenderPass::onClearStencilClip(const GrFixedClip& clip,
                                           bool insideStencilMask) {
    fGpu->clearStencilClip(clip, insideStencilMask, fRenderTarget, fOrigin);
}

