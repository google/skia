/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLOpsRenderPass.h"

#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTarget.h"

#ifdef SK_DEBUG
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#endif

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
    fGpu->flushScissorRect(scissor, fRenderTarget->height(), fOrigin);
}

bool GrGLOpsRenderPass::onBindTextures(const GrPrimitiveProcessor& primProc,
                                       const GrSurfaceProxy* const primProcTextures[],
                                       const GrPipeline& pipeline) {
    GrGLProgram* program = fGpu->currentProgram();
    SkASSERT(program);
    program->bindTextures(primProc, primProcTextures, pipeline);
    return true;
}

void GrGLOpsRenderPass::onBindBuffers(sk_sp<const GrBuffer> indexBuffer,
                                      sk_sp<const GrBuffer> instanceBuffer,
                                      sk_sp<const GrBuffer> vertexBuffer,
                                      GrPrimitiveRestart primitiveRestart) {
    SkASSERT((primitiveRestart == GrPrimitiveRestart::kNo) || indexBuffer);
    GrGLProgram* program = fGpu->currentProgram();
    SkASSERT(program);

#ifdef SK_DEBUG
    fDidBindInstanceBuffer = false;
    fDidBindVertexBuffer = false;
#endif

    int numAttribs = program->numVertexAttributes() + program->numInstanceAttributes();
    fAttribArrayState = fGpu->bindInternalVertexArray(indexBuffer.get(), numAttribs,
                                                      primitiveRestart);

    if (indexBuffer) {
        if (indexBuffer->isCpuBuffer()) {
            auto* cpuIndexBuffer = static_cast<const GrCpuBuffer*>(indexBuffer.get());
            fIndexPointer = reinterpret_cast<const uint16_t*>(cpuIndexBuffer->data());
        } else {
            fIndexPointer = nullptr;
        }
    }

    // If this platform does not support baseInstance, defer binding of the instance buffer.
    if (fGpu->glCaps().baseVertexBaseInstanceSupport()) {
        this->bindInstanceBuffer(instanceBuffer.get(), 0);
        SkDEBUGCODE(fDidBindInstanceBuffer = true;)
    }
    fActiveInstanceBuffer = std::move(instanceBuffer);

    // We differ binding the vertex buffer for one of two situations:
    // 1) This platform does not support baseVertex with indexed draws.
    // 2) There is a driver bug affecting glDrawArrays.
    if ((indexBuffer && fGpu->glCaps().baseVertexBaseInstanceSupport()) ||
        (!indexBuffer && !fGpu->glCaps().drawArraysBaseVertexIsBroken())) {
            this->bindVertexBuffer(vertexBuffer.get(), 0);
            SkDEBUGCODE(fDidBindVertexBuffer = true;)
    }
    fActiveVertexBuffer = std::move(vertexBuffer);
    fActiveIndexBuffer = std::move(indexBuffer);
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
    SkASSERT(fDidBindVertexBuffer || fGpu->glCaps().drawArraysBaseVertexIsBroken());
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
        SkASSERT(fDidBindVertexBuffer);
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
    SkASSERT(fDidBindVertexBuffer || fGpu->glCaps().drawArraysBaseVertexIsBroken());
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
            SkASSERT(fDidBindInstanceBuffer);
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
            SkASSERT(fDidBindInstanceBuffer);
            SkASSERT(fDidBindVertexBuffer);
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
    using MultiDrawType = GrGLCaps::MultiDrawType;

    SkASSERT(fGpu->caps()->nativeDrawIndirectSupport());
    SkASSERT(fGpu->glCaps().baseVertexBaseInstanceSupport());
    SkASSERT(fDidBindVertexBuffer || fGpu->glCaps().drawArraysBaseVertexIsBroken());

    if (fGpu->glCaps().drawArraysBaseVertexIsBroken()) {
        // We weren't able to bind the vertex buffer during onBindBuffers because of a driver bug
        // affecting glDrawArrays.
        this->bindVertexBuffer(fActiveVertexBuffer.get(), 0);
    }

    if (fGpu->glCaps().multiDrawType() == MultiDrawType::kANGLEOrWebGL) {
        // ANGLE and WebGL don't support glDrawElementsIndirect. We draw everything as a multi draw.
        this->multiDrawArraysANGLEOrWebGL(drawIndirectBuffer, offset, drawCount);
        return;
    }

    fGpu->bindBuffer(GrGpuBufferType::kDrawIndirect, drawIndirectBuffer);

    if (drawCount > 1 && fGpu->glCaps().multiDrawType() == MultiDrawType::kMultiDrawIndirect) {
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

void GrGLOpsRenderPass::multiDrawArraysANGLEOrWebGL(const GrBuffer* drawIndirectBuffer,
                                                    size_t offset, int drawCount) {
    SkASSERT(fGpu->glCaps().multiDrawType() == GrGLCaps::MultiDrawType::kANGLEOrWebGL);
    SkASSERT(drawIndirectBuffer->isCpuBuffer());

    constexpr static int kMaxDrawCountPerBatch = 128;
    GrGLint fFirsts[kMaxDrawCountPerBatch];
    GrGLsizei fCounts[kMaxDrawCountPerBatch];
    GrGLsizei fInstanceCounts[kMaxDrawCountPerBatch];
    GrGLuint fBaseInstances[kMaxDrawCountPerBatch];

    GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
    auto* cpuBuffer = static_cast<const GrCpuBuffer*>(drawIndirectBuffer);
    auto* cmds = reinterpret_cast<const GrDrawIndirectCommand*>(cpuBuffer->data() + offset);

    while (drawCount) {
        int countInBatch = std::min(drawCount, kMaxDrawCountPerBatch);
        for (int i = 0; i < countInBatch; ++i) {
            auto [vertexCount, instanceCount, baseVertex, baseInstance] = cmds[i];
            fFirsts[i] = baseVertex;
            fCounts[i] = vertexCount;
            fInstanceCounts[i] = instanceCount;
            fBaseInstances[i] = baseInstance;
        }
        if (countInBatch == 1) {
            GL_CALL(DrawArraysInstancedBaseInstance(glPrimType, fFirsts[0], fCounts[0],
                                                    fInstanceCounts[0], fBaseInstances[0]));
        } else {
            GL_CALL(MultiDrawArraysInstancedBaseInstance(glPrimType, fFirsts, fCounts,
                                                         fInstanceCounts, fBaseInstances,
                                                         countInBatch));
        }
        drawCount -= countInBatch;
        cmds += countInBatch;
    }
}

void GrGLOpsRenderPass::onDrawIndexedIndirect(const GrBuffer* drawIndirectBuffer, size_t offset,
                                              int drawCount) {
    using MultiDrawType = GrGLCaps::MultiDrawType;

    SkASSERT(fGpu->caps()->nativeDrawIndirectSupport());
    SkASSERT(!fGpu->caps()->nativeDrawIndexedIndirectIsBroken());
    SkASSERT(fGpu->glCaps().baseVertexBaseInstanceSupport());
    // The vertex buffer should have already gotten bound (as opposed us stashing it away during
    // onBindBuffers and not expecting to bind it until this point).
    SkASSERT(fDidBindVertexBuffer);

    if (fGpu->glCaps().multiDrawType() == MultiDrawType::kANGLEOrWebGL) {
        // ANGLE and WebGL don't support glDrawElementsIndirect. We draw everything as a multi draw.
        this->multiDrawElementsANGLEOrWebGL(drawIndirectBuffer, offset, drawCount);
        return;
    }

    fGpu->bindBuffer(GrGpuBufferType::kDrawIndirect, drawIndirectBuffer);

    if (drawCount > 1 && fGpu->glCaps().multiDrawType() == MultiDrawType::kMultiDrawIndirect) {
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

void GrGLOpsRenderPass::multiDrawElementsANGLEOrWebGL(const GrBuffer* drawIndirectBuffer,
                                                      size_t offset, int drawCount) {
    SkASSERT(fGpu->glCaps().multiDrawType() == GrGLCaps::MultiDrawType::kANGLEOrWebGL);
    SkASSERT(drawIndirectBuffer->isCpuBuffer());

    constexpr static int kMaxDrawCountPerBatch = 128;
    GrGLint fCounts[kMaxDrawCountPerBatch];
    const void* fIndices[kMaxDrawCountPerBatch];
    GrGLsizei fInstanceCounts[kMaxDrawCountPerBatch];
    GrGLint fBaseVertices[kMaxDrawCountPerBatch];
    GrGLuint fBaseInstances[kMaxDrawCountPerBatch];

    GrGLenum glPrimType = fGpu->prepareToDraw(fPrimitiveType);
    auto* cpuBuffer = static_cast<const GrCpuBuffer*>(drawIndirectBuffer);
    auto* cmds = reinterpret_cast<const GrDrawIndexedIndirectCommand*>(cpuBuffer->data() + offset);

    while (drawCount) {
        int countInBatch = std::min(drawCount, kMaxDrawCountPerBatch);
        for (int i = 0; i < countInBatch; ++i) {
            auto [indexCount, instanceCount, baseIndex, baseVertex, baseInstance] = cmds[i];
            fCounts[i] = indexCount;
            fIndices[i] = this->offsetForBaseIndex(baseIndex);
            fInstanceCounts[i] = instanceCount;
            fBaseVertices[i] = baseVertex;
            fBaseInstances[i] = baseInstance;
        }
        if (countInBatch == 1) {
            GL_CALL(DrawElementsInstancedBaseVertexBaseInstance(glPrimType, fCounts[0],
                                                                GR_GL_UNSIGNED_SHORT, fIndices[0],
                                                                fInstanceCounts[0],
                                                                fBaseVertices[0],
                                                                fBaseInstances[0]));
        } else {
            GL_CALL(MultiDrawElementsInstancedBaseVertexBaseInstance(glPrimType, fCounts,
                                                                     GR_GL_UNSIGNED_SHORT, fIndices,
                                                                     fInstanceCounts, fBaseVertices,
                                                                     fBaseInstances, countInBatch));
        }
        drawCount -= countInBatch;
        cmds += countInBatch;
    }
}

void GrGLOpsRenderPass::onClear(const GrScissorState& scissor, std::array<float, 4> color) {
    fGpu->clear(scissor, color, fRenderTarget, fOrigin);
}

void GrGLOpsRenderPass::onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) {
    fGpu->clearStencilClip(scissor, insideStencilMask, fRenderTarget, fOrigin);
}
