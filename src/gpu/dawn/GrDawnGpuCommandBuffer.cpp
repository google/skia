/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnGpuCommandBuffer.h"

#include "include/core/SkRect.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnRenderTarget.h"

void GrDawnGpuTextureCommandBuffer::copy(GrSurface* src, const SkIRect& srcRect,
                                         const SkIPoint& dstPoint) {
}

void GrDawnGpuTextureCommandBuffer::insertEventMarker(const char* msg) {
}

void GrDawnGpuTextureCommandBuffer::submit() {
    for (int i = 0; i < fCopies.count(); ++i) {
        CopyInfo& copyInfo = fCopies[i];
        fGpu->copySurface(fTexture, copyInfo.fSrc, copyInfo.fSrcRect, copyInfo.fDstPoint);
    }
}

GrDawnGpuTextureCommandBuffer::~GrDawnGpuTextureCommandBuffer() {}

////////////////////////////////////////////////////////////////////////////////

dawn::LoadOp to_dawn_load_op(GrLoadOp loadOp) {
    switch (loadOp) {
        case GrLoadOp::kLoad:
            return dawn::LoadOp::Load;
        case GrLoadOp::kClear:
            return dawn::LoadOp::Clear;
        case GrLoadOp::kDiscard:
        default:
            SK_ABORT("Invalid LoadOp");
            return dawn::LoadOp::Load;
    }
}

GrDawnGpuRTCommandBuffer::GrDawnGpuRTCommandBuffer(GrDawnGpu* gpu,
                                                   GrRenderTarget* rt, GrSurfaceOrigin origin,
                                                   const LoadAndStoreInfo& colorInfo,
                                                   const StencilLoadAndStoreInfo& stencilInfo)
        : INHERITED(rt, origin)
        , fGpu(gpu) {
    this->init();
}

void GrDawnGpuRTCommandBuffer::init() {
}


GrDawnGpuRTCommandBuffer::~GrDawnGpuRTCommandBuffer() {
}

GrGpu* GrDawnGpuRTCommandBuffer::gpu() { return fGpu; }

void GrDawnGpuRTCommandBuffer::end() {
}

void GrDawnGpuRTCommandBuffer::submit() {
    if (fCommandBuffer) {
        fGpu->queue().Submit(1, &fCommandBuffer);
    }
}

void GrDawnGpuRTCommandBuffer::insertEventMarker(const char* msg) {
}

void GrDawnGpuRTCommandBuffer::transferFrom(const SkIRect& srcRect, GrColorType bufferColorType,
                                            GrGpuBuffer* transferBuffer, size_t offset) {
    fGpu->transferPixelsFrom(fRenderTarget, srcRect.fLeft, srcRect.fTop, srcRect.width(),
                             srcRect.height(), bufferColorType, transferBuffer, offset);
}

void GrDawnGpuRTCommandBuffer::onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
}

void GrDawnGpuRTCommandBuffer::onClear(const GrFixedClip& clip, const SkPMColor4f& color) {
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnGpuRTCommandBuffer::inlineUpload(GrOpFlushState* state,
                                            GrDeferredTextureUploadFn& upload) {
}

void GrDawnGpuRTCommandBuffer::copy(GrSurface* src, const SkIRect& srcRect,
                                    const SkIPoint& dstPoint) {
}

////////////////////////////////////////////////////////////////////////////////

void GrDawnGpuRTCommandBuffer::bindGeometry(const GrBuffer* indexBuffer,
                                            const GrBuffer* vertexBuffer,
                                            const GrBuffer* instanceBuffer) {
}

void GrDawnGpuRTCommandBuffer::onDraw(const GrPrimitiveProcessor& primProc,
                                      const GrPipeline& pipeline,
                                      const GrPipeline::FixedDynamicState* fixedDynamicState,
                                      const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                      const GrMesh meshes[],
                                      int meshCount,
                                      const SkRect& bounds) {
    if (!meshCount) {
        return;
    }
    GrFragmentProcessor::Iter iter(pipeline);

    for (int i = 0; i < meshCount; ++i) {
        const GrMesh& mesh = meshes[i];
        mesh.sendToGpu(this);
    }
}

void GrDawnGpuRTCommandBuffer::sendInstancedMeshToGpu(GrPrimitiveType,
                                                      const GrBuffer* vertexBuffer,
                                                      int vertexCount,
                                                      int baseVertex,
                                                      const GrBuffer* instanceBuffer,
                                                      int instanceCount,
                                                      int baseInstance) {
    this->bindGeometry(nullptr, vertexBuffer, instanceBuffer);
    fGpu->stats()->incNumDraws();
}

void GrDawnGpuRTCommandBuffer::sendIndexedInstancedMeshToGpu(GrPrimitiveType,
                                                             const GrBuffer* indexBuffer,
                                                             int indexCount,
                                                             int baseIndex,
                                                             const GrBuffer* vertexBuffer,
                                                             int baseVertex,
                                                             const GrBuffer* instanceBuffer,
                                                             int instanceCount,
                                                             int baseInstance,
                                                             GrPrimitiveRestart restart) {
    this->bindGeometry(indexBuffer, vertexBuffer, instanceBuffer);
    fGpu->stats()->incNumDraws();
}

