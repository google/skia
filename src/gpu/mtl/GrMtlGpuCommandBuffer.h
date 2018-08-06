/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlGpuCommandBuffer_DEFINED
#define GrMtlGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"
#include "GrMtlGpu.h"

#import <metal/metal.h>

class GrMtlRenderTarget;

class GrMtlGpuTextureCommandBuffer : public GrGpuTextureCommandBuffer {
public:
    GrMtlGpuTextureCommandBuffer(GrMtlGpu* gpu, GrTexture* texture, GrSurfaceOrigin origin)
            : INHERITED(texture, origin)
            , fGpu(gpu) {
    }

    ~GrMtlGpuTextureCommandBuffer() override {}

    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override {
        fGpu->copySurface(fTexture, fOrigin, src, srcOrigin, srcRect, dstPoint);
    }

    void insertEventMarker(const char* msg) override {}

private:
    void submit() override {}

    GrMtlGpu* fGpu;

    typedef GrGpuTextureCommandBuffer INHERITED;
};

class GrMtlGpuRTCommandBuffer : public GrGpuRTCommandBuffer {
public:
    GrMtlGpuRTCommandBuffer(GrMtlGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                            const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
                            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo)
            : INHERITED(rt, origin)
            , fGpu(gpu)
            , fColorLoadAndStoreInfo(colorInfo)
            , fStencilLoadAndStoreInfo(stencilInfo) {
        // Silence unused var warning
        (void)fColorLoadAndStoreInfo;
        (void)fStencilLoadAndStoreInfo;
    }

    ~GrMtlGpuRTCommandBuffer() override {}

    void begin() override {}
    void end() override {}

    void discard() override {}

    void insertEventMarker(const char* msg) override {}

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override {}

    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override {
        fGpu->copySurface(fRenderTarget, fOrigin, src, srcOrigin, srcRect, dstPoint);
    }

    void submit() override {}

private:
    GrGpu* gpu() override { return fGpu; }

    void onDraw(const GrPrimitiveProcessor& primProc,
                const GrPipeline& pipeline,
                const GrPipeline::FixedDynamicState* fixedDynamicState,
                const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                const GrMesh mesh[],
                int meshCount,
                const SkRect& bounds) override {}

    void onClear(const GrFixedClip& clip, GrColor color) override {}

    void onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) override {}

    GrMtlGpu*                                     fGpu;
    GrGpuRTCommandBuffer::LoadAndStoreInfo        fColorLoadAndStoreInfo;
    GrGpuRTCommandBuffer::StencilLoadAndStoreInfo fStencilLoadAndStoreInfo;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif

