/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGpuCommandBuffer_DEFINED
#define GrGLGpuCommandBuffer_DEFINED

#include "src/gpu/GrGpuCommandBuffer.h"

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLRenderTarget.h"

class GrGLGpu;
class GrGLRenderTarget;

class GrGLGpuTextureCommandBuffer : public GrGpuTextureCommandBuffer {
public:
    GrGLGpuTextureCommandBuffer(GrGLGpu* gpu) : fGpu(gpu) {}

    void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override {
        fGpu->copySurface(fTexture, src, srcRect, dstPoint);
    }

    void transferFrom(const SkIRect& srcRect, GrColorType bufferColorType,
                      GrGpuBuffer* transferBuffer, size_t offset) override {
        fGpu->transferPixelsFrom(fTexture, srcRect.fLeft, srcRect.fTop, srcRect.width(),
                                 srcRect.height(), bufferColorType, transferBuffer, offset);
    }

    void insertEventMarker(const char* msg) override {
        fGpu->insertEventMarker(msg);
    }

    void reset() {
        fTexture = nullptr;
    }

private:
    GrGLGpu* fGpu;

    typedef GrGpuTextureCommandBuffer INHERITED;
};

class GrGLGpuRTCommandBuffer : public GrGpuRTCommandBuffer {
/**
 * We do not actually buffer up draws or do any work in the this class for GL. Instead commands
 * are immediately sent to the gpu to execute. Thus all the commands in this class are simply
 * pass through functions to corresponding calls in the GrGLGpu class.
 */
public:
    GrGLGpuRTCommandBuffer(GrGLGpu* gpu) : fGpu(gpu) {}

    void begin() override;
    void end() override {}

    void discard() override { }

    void insertEventMarker(const char* msg) override {
        fGpu->insertEventMarker(msg);
    }

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override {
        state->doUpload(upload);
    }

    void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override {
        fGpu->copySurface(fRenderTarget, src,srcRect, dstPoint);
    }

    void transferFrom(const SkIRect& srcRect, GrColorType bufferColorType,
                      GrGpuBuffer* transferBuffer, size_t offset) override {
        fGpu->transferPixelsFrom(fRenderTarget, srcRect.fLeft, srcRect.fTop, srcRect.width(),
                                 srcRect.height(), bufferColorType, transferBuffer, offset);
    }

    void set(GrRenderTarget*, GrSurfaceOrigin,
             const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
             const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&);

    void reset() {
        fRenderTarget = nullptr;
    }

private:
    GrGpu* gpu() override { return fGpu; }

    void onDraw(const GrPrimitiveProcessor& primProc,
                const GrPipeline& pipeline,
                const GrPipeline::FixedDynamicState* fixedDynamicState,
                const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                const GrMesh mesh[],
                int meshCount,
                const SkRect& bounds) override {
        fGpu->draw(fRenderTarget, fOrigin, primProc, pipeline, fixedDynamicState,
                   dynamicStateArrays, mesh, meshCount);
    }

    void onClear(const GrFixedClip& clip, const SkPMColor4f& color) override {
        fGpu->clear(clip, color, fRenderTarget, fOrigin);
    }

    void onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) override {
        fGpu->clearStencilClip(clip, insideStencilMask, fRenderTarget, fOrigin);
    }

    GrGLGpu*                                      fGpu;
    GrGpuRTCommandBuffer::LoadAndStoreInfo        fColorLoadAndStoreInfo;
    GrGpuRTCommandBuffer::StencilLoadAndStoreInfo fStencilLoadAndStoreInfo;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif

