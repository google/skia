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

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)

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
    SkASSERT(program);
    program->bindTextures(primProc, primProcTextures, pipeline);
    return true;
}

void GrGLOpsRenderPass::onBindBuffers(const GrBuffer* indexBuffer, const GrBuffer* instanceBuffer,
                                      const GrBuffer* vertexBuffer,
                                      GrPrimitiveRestart primitiveRestart) {
    SkASSERT((primitiveRestart == GrPrimitiveRestart::kNo) || indexBuffer);
    GrGLProgram* program = fGpu->currentProgram();
    SkASSERT(program);

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

    if (!fGpu->glCaps().baseVertexBaseInstanceSupport()) {
        // This platform does not support baseInstance. Defer binding of the instance buffer.
        fActiveInstanceBuffer = sk_ref_sp(instanceBuffer);
    } else {
        this->bindInstanceBuffer(instanceBuffer, 0);
    }
    if (!indexBuffer && fGpu->glCaps().drawArraysBaseVertexIsBroken()) {
        // There is a driver bug affecting glDrawArrays. Defer binding of the vertex buffer.
        fActiveVertexBuffer = sk_ref_sp(vertexBuffer);
    } else if (indexBuffer && !fGpu->glCaps().baseVertexBaseInstanceSupport()) {
        // This platform does not support baseVertex with indexed draws. Defer binding of the
        // vertex buffer.
        fActiveVertexBuffer = sk_ref_sp(vertexBuffer);
    } else {
        this->bindVertexBuffer(vertexBuffer, 0);
    }
}

void GrGLOpsRenderPass::bindInstanceBuffer(const GrBuffer* instanceBuffer, int baseInstance) {
    GrGLProgram* program = fGpu->currentProgram();
    SkASSERT(program);
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

void GrGLOpsRenderPass::bindVertexBuffer(const GrBuffer* vertexBuffer, int baseVertex) {
    GrGLProgram* program = fGpu->currentProgram();
    SkASSERT(program);
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
}

void GrGLOpsRenderPass::onDraw(int vertexCount, int baseVertex) {
    SkASSERT(!fActiveVertexBuffer || fGpu->glCaps().drawArraysBaseVertexIsBroken());
    GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
    if (fGpu->glCaps().drawArraysBaseVertexIsBroken()) {
        this->bindVertexBuffer(fActiveVertexBuffer.get(), baseVertex);
        baseVertex = 0;
    }
    GL_CALL(DrawArrays(glPrimType, baseVertex, vertexCount));
}

void GrGLOpsRenderPass::onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                                      uint16_t maxIndexValue, int baseVertex) {
    GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
    if (fGpu->glCaps().baseVertexBaseInstanceSupport()) {
        SkASSERT(fGpu->glCaps().drawInstancedSupport());
        SkASSERT(!fActiveVertexBuffer);
        if (baseVertex != 0) {
            GL_CALL(DrawElementsInstancedBaseVertexBaseInstance(
                    glPrimType, indexCount, GR_GL_UNSIGNED_SHORT,
                    this->offsetForBaseIndex(baseIndex), 1, baseVertex, 0));
            return;
        }
    } else {
        this->bindVertexBuffer(fActiveVertexBuffer.get(), baseVertex);
    }

    if (fGpu->glCaps().drawRangeElementsSupport()) {
        GL_CALL(DrawRangeElements(glPrimType, minIndexValue, maxIndexValue, indexCount,
                                  GR_GL_UNSIGNED_SHORT, this->offsetForBaseIndex(baseIndex)));
    } else {
        GL_CALL(DrawElements(glPrimType, indexCount, GR_GL_UNSIGNED_SHORT,
                             this->offsetForBaseIndex(baseIndex)));
    }
}

void GrGLOpsRenderPass::onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                                        int baseVertex) {
    SkASSERT(!fActiveVertexBuffer || fGpu->glCaps().drawArraysBaseVertexIsBroken());
    if (fGpu->glCaps().drawArraysBaseVertexIsBroken()) {
        // We weren't able to bind the vertex buffer during onBindBuffers because of a driver bug
        // affecting glDrawArrays.
        this->bindVertexBuffer(fActiveVertexBuffer.get(), 0);
    }
    int maxInstances = fGpu->glCaps().maxInstancesPerDrawWithoutCrashing(instanceCount);
    for (int i = 0; i < instanceCount; i += maxInstances) {
        GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
        int instanceCountForDraw = std::min(instanceCount - i, maxInstances);
        int baseInstanceForDraw = baseInstance + i;
        if (fGpu->glCaps().baseVertexBaseInstanceSupport()) {
            SkASSERT(!fActiveInstanceBuffer);
            GL_CALL(DrawArraysInstancedBaseInstance(glPrimType, baseVertex, vertexCount,
                                                    instanceCountForDraw, baseInstanceForDraw));
        } else {
            this->bindInstanceBuffer(fActiveInstanceBuffer.get(), baseInstanceForDraw);
            GL_CALL(DrawArraysInstanced(glPrimType, baseVertex, vertexCount, instanceCountForDraw));
        }
    }
}

void GrGLOpsRenderPass::onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount,
                                               int baseInstance, int baseVertex) {
    int maxInstances = fGpu->glCaps().maxInstancesPerDrawWithoutCrashing(instanceCount);
    for (int i = 0; i < instanceCount; i += maxInstances) {
        GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
        int instanceCountForDraw = std::min(instanceCount - i, maxInstances);
        int baseInstanceForDraw = baseInstance + i;
        if (fGpu->glCaps().baseVertexBaseInstanceSupport()) {
            SkASSERT(!fActiveInstanceBuffer);
            SkASSERT(!fActiveVertexBuffer);
            GL_CALL(DrawElementsInstancedBaseVertexBaseInstance(
                    glPrimType, indexCount, GR_GL_UNSIGNED_SHORT,
                    this->offsetForBaseIndex(baseIndex), instanceCountForDraw, baseVertex,
                    baseInstanceForDraw));
        } else {
            this->bindInstanceBuffer(fActiveInstanceBuffer.get(), baseInstanceForDraw);
            this->bindVertexBuffer(fActiveVertexBuffer.get(), baseVertex);
            GL_CALL(DrawElementsInstanced(glPrimType, indexCount, GR_GL_UNSIGNED_SHORT,
                                        this->offsetForBaseIndex(baseIndex), instanceCountForDraw));
        }
    }
}

static const void* buffer_offset_to_gl_address(const GrBuffer* drawIndirectBuffer, size_t offset) {
    if (drawIndirectBuffer->isCpuBuffer()) {
        return static_cast<const GrCpuBuffer*>(drawIndirectBuffer)->data() + offset;
    } else {
        return (offset) ? reinterpret_cast<const void*>(offset) : nullptr;
    }
}

void GrGLOpsRenderPass::onDrawIndirect(const GrBuffer* drawIndirectBuffer, size_t offset,
                                       int drawCount) {
    fGpu->bindBuffer(GrGpuBufferType::kDrawIndirect, drawIndirectBuffer);

    if (fGpu->glCaps().multiDrawIndirectSupport() && drawCount > 1) {
        GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
        GL_CALL(MultiDrawArraysIndirect(glPrimType,
                                        buffer_offset_to_gl_address(drawIndirectBuffer, offset),
                                        drawCount, sizeof(GrDrawIndirectCommand)));
        return;
    }

    for (int i = 0; i < drawCount; ++i) {
        GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
        GL_CALL(DrawArraysIndirect(glPrimType,
                                   buffer_offset_to_gl_address(drawIndirectBuffer, offset)));
        offset += sizeof(GrDrawIndirectCommand);
    }
}

void GrGLOpsRenderPass::onDrawIndexedIndirect(const GrBuffer* drawIndirectBuffer, size_t offset,
                                              int drawCount) {
    fGpu->bindBuffer(GrGpuBufferType::kDrawIndirect, drawIndirectBuffer);

    if (fGpu->glCaps().multiDrawIndirectSupport() && drawCount > 1) {
        GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
        GL_CALL(MultiDrawElementsIndirect(glPrimType, GR_GL_UNSIGNED_SHORT,
                                          buffer_offset_to_gl_address(drawIndirectBuffer, offset),
                                          drawCount, sizeof(GrDrawIndexedIndirectCommand)));
        return;
    }

    for (int i = 0; i < drawCount; ++i) {
        GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
        GL_CALL(DrawElementsIndirect(glPrimType, GR_GL_UNSIGNED_SHORT,
                                     buffer_offset_to_gl_address(drawIndirectBuffer, offset)));
        offset += sizeof(GrDrawIndexedIndirectCommand);
    }
}

void GrGLOpsRenderPass::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
    fGpu->clear(clip, color, fRenderTarget, fOrigin);
}

void GrGLOpsRenderPass::onClearStencilClip(const GrFixedClip& clip,
                                           bool insideStencilMask) {
    fGpu->clearStencilClip(clip, insideStencilMask, fRenderTarget, fOrigin);
}

